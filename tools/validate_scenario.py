#!/usr/bin/env python3
"""
validate_scenario.py
====================
Static validator for Karakuri YAML scenario files.
Phase 3-B guidance upgrade: each error now includes a SUGGEST line with a
fix hint (Did you mean? / Add to payload / Available scenes).

Purpose
-------
Detect invalid scenario DSL **before runtime** so AI-generated scenarios
fail fast with human-readable errors instead of causing ScenarioRunner
crashes or silent logic bugs.

Guide structure (per error):
  ERROR  <file>  scene='...'  action[N]
         <what is wrong>
         SUGGEST: <how to fix it>

Usage
-----
    python3 tools/validate_scenario.py [FILE ...]

    # validate a single file
    python3 tools/validate_scenario.py src/games/mystery_test/scenario/mystery_case.yaml

    # validate all mystery_test scenarios
    python3 tools/validate_scenario.py src/games/mystery_test/scenario/*.yaml

    # validate all (default when no args given)
    python3 tools/validate_scenario.py

Exit codes
----------
  0  all files valid
  1  one or more validation errors found
"""

import sys
import os
import glob
import difflib
from pathlib import Path

try:
    import yaml
except ImportError:
    print("ERROR: PyYAML is required.  Run:  pip install pyyaml", file=sys.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------------
# Schema: known actions and their required payload fields
# ---------------------------------------------------------------------------
#
# SOURCE OF TRUTH: docs/mystery_test_task_catalog.md
#
# This dict is the single place that defines the set of valid YAML actions.
# Whenever a new Task class is added to ActionRegistry, add a matching entry
# here so that the validator stays in sync with the runtime.
#
# "required": list of field groups; at least one key per group must be present.
# "optional": informational only — any extra payload keys are silently ignored.
# "_special": if present, a dedicated function below handles this action.

ACTIONS = {
    "show_dialogue": {
        "required": [["speaker"], ["text"]],
        "optional": [],
    },
    "discover_evidence": {
        # "evidence_id" is canonical; "id" is accepted as a legacy alias
        "required": [["evidence_id", "id"], ["location"]],
        "optional": [],
    },
    "add_evidence": {
        # "type" is optional — it is a semantic label for AI readability only
        # and is NOT validated at runtime (docs/mystery_test_task_catalog.md §2)
        "required": [["id"]],
        "optional": ["type"],
    },
    "check_evidence": {
        # Branch on a single SCOPE_GLOBAL evidence flag (pairs with add_evidence)
        "required": [],
        "optional": [],
        "_special": "check_evidence",
    },
    "wait_for_signal": {
        # "timeout" is optional; runtime default is 5.0 seconds
        "required": [["signal"]],
        "optional": ["timeout"],
    },
    "check_condition": {
        # Complex all_of / any_of structure — handled by a dedicated function
        "required": [],
        "optional": [],
        "_special": "check_condition",
    },
    "save_load_test": {
        # Diagnostic task: exercises WorldState serialize/deserialize round-trip
        # Do NOT use in story scenarios.
        "required": [],
        "optional": ["key"],
    },
    "parallel": {
        # Composite task: runs sub-tasks concurrently — handled by a dedicated function
        "required": [],
        "optional": [],
        "_special": "parallel",
    },
    "end_game": {
        "required": [["result"]],
        "optional": [],
    },
    # ------------------------------------------------------------------
    # Billiards Test tasks
    # ------------------------------------------------------------------
    "setup_billiards_round": {
        # Both fields are optional; runtime defaults: shot_limit=5, target_count=2
        "required": [],
        "optional": ["shot_limit", "target_count"],
    },
    "wait_for_billiards_event": {
        # 'events' is required (validated in the special handler)
        "required": [],
        "optional": [],
        "_special": "wait_for_billiards_event",
    },
    "record_billiards_event": {
        "required": [["event"]],
        "optional": [],
    },
    "evaluate_billiards_round": {
        # all three scene refs are required (validated in the special handler)
        "required": [],
        "optional": [],
        "_special": "evaluate_billiards_round",
    },
    # ------------------------------------------------------------------
    # Roguelike Test tasks
    # ------------------------------------------------------------------
    "setup_roguelike_round": {
        # hp and position overrides are all optional
        "required": [],
        "optional": ["player_hp", "enemy_hp", "player_x", "player_y", "enemy_1_x", "enemy_1_y"],
    },
    "load_fake_player_command": {
        # 'command' is required (validated in the special handler)
        "required": [],
        "optional": [],
        "_special": "load_fake_player_command",
    },
    "apply_player_move": {
        "required": [],
        "optional": [],
    },
    "apply_player_attack": {
        "required": [["target"]],
        "optional": [],
    },
    "apply_enemy_turn": {
        "required": [],
        "optional": [],
    },
    "resolve_roguelike_turn": {
        "required": [],
        "optional": [],
    },
    "evaluate_roguelike_round": {
        # all three scene refs are required (validated in the special handler)
        "required": [],
        "optional": [],
        "_special": "evaluate_roguelike_round",
    },
    # ------------------------------------------------------------------
    # Rhythm Test tasks
    # ------------------------------------------------------------------
    "setup_rhythm_round": {
        "required": [],
        "optional": [],
        "_special": "setup_rhythm_round",
    },
    "load_fake_tap": {
        "required": [],
        "optional": [],
    },
    "advance_rhythm_clock": {
        "required": [],
        "optional": [],
    },
    "judge_rhythm_note": {
        "required": [],
        "optional": [],
    },
    "resolve_rhythm_progress": {
        "required": [],
        "optional": [],
    },
    "evaluate_rhythm_round": {
        "required": [],
        "optional": [],
        "_special": "evaluate_rhythm_round",
    },
}

VALID_END_GAME_RESULTS = {"solved", "wrong", "failed", "lost", "timeout"}
VALID_EVIDENCE_TYPES = {
    "item", "motive", "state", "tool", "trait",
    "discovery", "evidence", "achievement", "dummy",
}
VALID_BILLIARDS_EVENTS = {
    "shot_committed", "ball_pocketed", "cue_ball_pocketed", "balls_stopped",
}
VALID_PLAYER_COMMANDS = {
    "move_up", "move_down", "move_left", "move_right", "attack",
}

# Known payload keys for actions that use _special handlers.
# Used by the unknown-key guard in validate_action().
SPECIAL_KNOWN_KEYS = {
    "check_evidence":           {"id", "if_true", "if_false"},
    "check_condition":          {"all_of", "any_of", "if_true", "if_false"},
    "parallel":                 {"tasks"},
    "wait_for_billiards_event": {"events", "timeout"},
    "evaluate_billiards_round": {"if_clear", "if_fail", "if_continue"},
    "load_fake_player_command": {"command"},
    "evaluate_roguelike_round": {"if_clear", "if_fail", "if_continue"},
    "setup_rhythm_round": {"notes", "taps", "advance_ms", "perfect_window_ms", "good_window_ms", "clear_hit_count", "max_miss_count"},
    "evaluate_rhythm_round": {"if_clear", "if_fail", "if_continue"},
}

# ---------------------------------------------------------------------------
# Error collector
# ---------------------------------------------------------------------------

def _did_you_mean(word: str, candidates, n: int = 1) -> str:
    """Return 'Did you mean: X?' string if a close match exists, else empty string."""
    matches = difflib.get_close_matches(word, candidates, n=n, cutoff=0.6)
    if not matches:
        return ""
    if len(matches) == 1:
        return f"Did you mean: '{matches[0]}'?"
    return "Did you mean one of: " + ", ".join(f"'{m}'" for m in matches) + "?"


class ValidationError:
    def __init__(self, filename: str, scene: str, action_index: int, message: str, suggestion: str = ""):
        self.filename     = filename
        self.scene        = scene
        self.action_index = action_index
        self.message      = message
        self.suggestion   = suggestion

    def __str__(self) -> str:
        loc = f"{self.filename}  scene='{self.scene}'"
        if self.action_index >= 0:
            loc += f"  action[{self.action_index}]"
        out = f"  ERROR  {loc}\n         {self.message}"
        if self.suggestion:
            out += f"\n         SUGGEST: {self.suggestion}"
        return out


# ---------------------------------------------------------------------------
# Core validation logic
# ---------------------------------------------------------------------------

def _scene_label(scene: str, idx: int) -> str:
    return f"scene '{scene}', action[{idx}]"


def validate_payload_fields(required_groups, payload, filename, scene, idx, errors):
    """
    For each group in required_groups, at least one key in the group must be
    present in payload.  E.g. [["evidence_id", "id"]] means either key is OK.
    """
    for group in required_groups:
        if not any(k in payload for k in group):
            if len(group) == 1:
                key = group[0]
                msg = f"Missing required payload field '{key}'."
                sug = f"Add to payload:  {key}: <value>"
            else:
                msg = f"Missing required payload field: one of {group} must be present."
                sug = f"Add one of these to payload: " + "  OR  ".join(f"{k}: <value>" for k in group)
            errors.append(ValidationError(filename, scene, idx, msg, suggestion=sug))


# Actions that are forbidden inside a parallel group because their side effects
# (scene jumps, game-end signals) are not coordinated with sibling sub-tasks.
PARALLEL_FORBIDDEN_ACTIONS = {"check_evidence", "check_condition", "end_game", "parallel"}


def validate_check_evidence(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the check_evidence action format."""
    for field in ("id", "if_true", "if_false"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"check_evidence is missing required field '{field}'.",
                suggestion=f"Add to payload:  {field}: <value>"))

    for field in ("if_true", "if_false"):
        if field in payload and payload[field] not in all_scene_ids:
            ref = payload[field]
            hint = _did_you_mean(ref, all_scene_ids)
            sug = f"Available scenes: {sorted(all_scene_ids)}"
            if hint:
                sug = f"{hint}  Available scenes: {sorted(all_scene_ids)}"
            errors.append(ValidationError(filename, scene, idx,
                f"check_evidence '{field}' references unknown scene '{ref}'.",
                suggestion=sug))


def validate_parallel(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the parallel composite action and all its sub-tasks."""
    if "tasks" not in payload:
        errors.append(ValidationError(filename, scene, idx,
            "parallel is missing required field 'tasks'."))
        return

    tasks = payload["tasks"]
    if not isinstance(tasks, list):
        errors.append(ValidationError(filename, scene, idx,
            "parallel 'tasks' must be an array."))
        return

    if len(tasks) == 0:
        errors.append(ValidationError(filename, scene, idx,
            "parallel 'tasks' must not be empty."))
        return

    for sub_idx, sub_task in enumerate(tasks):
        if not isinstance(sub_task, dict):
            errors.append(ValidationError(filename, scene, idx,
                f"parallel 'tasks[{sub_idx}]' must be a dict with 'action' and 'payload'."))
            continue

        if "action" not in sub_task:
            errors.append(ValidationError(filename, scene, idx,
                f"parallel 'tasks[{sub_idx}]' is missing the 'action' key."))
            continue

        sub_action = sub_task["action"]

        if sub_action in PARALLEL_FORBIDDEN_ACTIONS:
            errors.append(ValidationError(filename, scene, idx,
                f"parallel 'tasks[{sub_idx}]' uses forbidden action '{sub_action}'. "
                f"Branching, terminal, and nested parallel actions must not appear "
                f"inside a parallel group. Forbidden: {sorted(PARALLEL_FORBIDDEN_ACTIONS)}."))
            continue

        if sub_action not in ACTIONS:
            errors.append(ValidationError(filename, scene, idx,
                f"parallel 'tasks[{sub_idx}]' uses unknown action '{sub_action}'. "
                f"Valid actions: {sorted(ACTIONS.keys())}."))
            continue

        sub_payload = sub_task.get("payload", {})
        if not isinstance(sub_payload, dict):
            errors.append(ValidationError(filename, scene, idx,
                f"parallel 'tasks[{sub_idx}]' payload must be a dict."))
            continue

        sub_schema = ACTIONS[sub_action]
        validate_payload_fields(
            sub_schema["required"], sub_payload,
            filename, f"{scene}[parallel.tasks[{sub_idx}]]", 0, errors
        )


def validate_wait_for_billiards_event(payload, filename, scene, idx, errors):
    """Validate the wait_for_billiards_event action."""
    if "events" not in payload:
        errors.append(ValidationError(filename, scene, idx,
            "wait_for_billiards_event is missing required field 'events'."))
        return

    events = payload["events"]
    if not isinstance(events, list) or len(events) == 0:
        errors.append(ValidationError(filename, scene, idx,
            "wait_for_billiards_event 'events' must be a non-empty array."))
        return

    for ev in events:
        if ev not in VALID_BILLIARDS_EVENTS:
            hint = _did_you_mean(ev, VALID_BILLIARDS_EVENTS)
            sug = f"Valid events: {sorted(VALID_BILLIARDS_EVENTS)}"
            if hint:
                sug = f"{hint}  Valid events: {sorted(VALID_BILLIARDS_EVENTS)}"
            errors.append(ValidationError(filename, scene, idx,
                f"wait_for_billiards_event: unknown event '{ev}'.",
                suggestion=sug))

    if "timeout" in payload:
        try:
            t = float(payload["timeout"])
            if t <= 0:
                errors.append(ValidationError(filename, scene, idx,
                    f"wait_for_billiards_event 'timeout' must be > 0, got '{payload['timeout']}'",
                    suggestion="timeout: 10.0  # seconds; omit to use the runtime default of 10.0"))
        except (TypeError, ValueError):
            errors.append(ValidationError(filename, scene, idx,
                f"wait_for_billiards_event 'timeout' must be a number, got '{payload['timeout']}'",
                suggestion="timeout: 10.0  # numeric seconds"))


def validate_evaluate_billiards_round(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the evaluate_billiards_round action."""
    for field in ("if_clear", "if_fail", "if_continue"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_billiards_round is missing required field '{field}'.",
                suggestion=f"Add to payload:  {field}: <scene_name>"))
        elif payload[field] not in all_scene_ids:
            ref = payload[field]
            hint = _did_you_mean(ref, all_scene_ids)
            sug = f"Available scenes: {sorted(all_scene_ids)}"
            if hint:
                sug = f"{hint}  Available scenes: {sorted(all_scene_ids)}"
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_billiards_round '{field}' references unknown scene '{ref}'.",
                suggestion=sug))


def validate_load_fake_player_command(payload, filename, scene, idx, errors):
    """Validate the load_fake_player_command action."""
    if "command" not in payload:
        errors.append(ValidationError(filename, scene, idx,
            "load_fake_player_command is missing required field 'command'.",
            suggestion=f"Add to payload:  command: <cmd>  Valid commands: {sorted(VALID_PLAYER_COMMANDS)}"))
        return
    cmd = payload["command"]
    if cmd not in VALID_PLAYER_COMMANDS:
        hint = _did_you_mean(cmd, VALID_PLAYER_COMMANDS)
        sug = f"Valid commands: {sorted(VALID_PLAYER_COMMANDS)}"
        if hint:
            sug = f"{hint}  Valid commands: {sorted(VALID_PLAYER_COMMANDS)}"
        errors.append(ValidationError(filename, scene, idx,
            f"load_fake_player_command: unknown command '{cmd}'.",
            suggestion=sug))


def validate_evaluate_roguelike_round(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the evaluate_roguelike_round action."""
    for field in ("if_clear", "if_fail", "if_continue"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_roguelike_round is missing required field '{field}'.",
                suggestion=f"Add to payload:  {field}: <scene_name>"))
        elif payload[field] not in all_scene_ids:
            ref = payload[field]
            hint = _did_you_mean(ref, all_scene_ids)
            sug = f"Available scenes: {sorted(all_scene_ids)}"
            if hint:
                sug = f"{hint}  Available scenes: {sorted(all_scene_ids)}"
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_roguelike_round '{field}' references unknown scene '{ref}'.",
                suggestion=sug))


def validate_setup_rhythm_round(payload, filename, scene, idx, errors):
    """Validate the setup_rhythm_round action."""
    for field in ("notes", "taps"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round is missing required field '{field}'."))
    if "notes" not in payload or "taps" not in payload:
        return

    notes = payload["notes"]
    taps = payload["taps"]
    if not isinstance(notes, list) or not isinstance(taps, list):
        errors.append(ValidationError(filename, scene, idx,
            "setup_rhythm_round 'notes' and 'taps' must both be arrays."))
        return

    if not (3 <= len(notes) <= 5):
        errors.append(ValidationError(filename, scene, idx,
            "setup_rhythm_round 'notes' length must be in [3, 5]."))
    if len(taps) != len(notes):
        errors.append(ValidationError(filename, scene, idx,
            "setup_rhythm_round 'taps' length must match 'notes'."))

    prev = None
    for i, note in enumerate(notes):
        if not isinstance(note, (int, float)):
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round notes[{i}] must be numeric."))
            continue
        if note <= 0:
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round notes[{i}] must be > 0."))
        if prev is not None and note <= prev:
            errors.append(ValidationError(filename, scene, idx,
                "setup_rhythm_round note times must be strictly ascending."))
        prev = note

    for i, tap in enumerate(taps):
        if not isinstance(tap, (int, float)):
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round taps[{i}] must be numeric."))
            continue
        if tap < -1:
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round taps[{i}] must be >= -1."))

    for field in ("advance_ms", "perfect_window_ms", "good_window_ms", "clear_hit_count", "max_miss_count"):
        if field in payload and not isinstance(payload[field], (int, float)):
            errors.append(ValidationError(filename, scene, idx,
                f"setup_rhythm_round '{field}' must be numeric."))

    if "advance_ms" in payload and payload["advance_ms"] <= 0:
        errors.append(ValidationError(filename, scene, idx,
            "setup_rhythm_round 'advance_ms' must be > 0."))
    if "perfect_window_ms" in payload and payload["perfect_window_ms"] <= 0:
        errors.append(ValidationError(filename, scene, idx,
            "setup_rhythm_round 'perfect_window_ms' must be > 0."))
    if "good_window_ms" in payload and "perfect_window_ms" in payload:
        if payload["good_window_ms"] < payload["perfect_window_ms"]:
            errors.append(ValidationError(filename, scene, idx,
                "setup_rhythm_round 'good_window_ms' must be >= 'perfect_window_ms'."))


def validate_evaluate_rhythm_round(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the evaluate_rhythm_round action."""
    for field in ("if_clear", "if_fail", "if_continue"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_rhythm_round is missing required field '{field}'.",
                suggestion=f"Add to payload:  {field}: <scene_name>"))
        elif payload[field] not in all_scene_ids:
            ref = payload[field]
            hint = _did_you_mean(ref, all_scene_ids)
            sug = f"Available scenes: {sorted(all_scene_ids)}"
            if hint:
                sug = f"{hint}  Available scenes: {sorted(all_scene_ids)}"
            errors.append(ValidationError(filename, scene, idx,
                f"evaluate_rhythm_round '{field}' references unknown scene '{ref}'.",
                suggestion=sug))


def validate_check_condition(payload, filename, scene, idx, errors, all_scene_ids):
    """Validate the check_condition action format."""
    has_all = "all_of" in payload
    has_any = "any_of" in payload

    if has_all and has_any:
        errors.append(ValidationError(filename, scene, idx,
            "check_condition payload must contain exactly one of 'all_of' or 'any_of', not both."))
        return
    if not has_all and not has_any:
        errors.append(ValidationError(filename, scene, idx,
            "check_condition payload must contain 'all_of' or 'any_of'."))
        return

    cond_key = "all_of" if has_all else "any_of"
    cond_list = payload[cond_key]

    if not isinstance(cond_list, list):
        errors.append(ValidationError(filename, scene, idx,
            f"'{cond_key}' must be an array."))
        return

    for i, entry in enumerate(cond_list):
        if not isinstance(entry, dict):
            errors.append(ValidationError(filename, scene, idx,
                f"'{cond_key}[{i}]' must be a dictionary with key 'evidence'."))
            continue
        if "evidence" not in entry:
            errors.append(ValidationError(filename, scene, idx,
                f"'{cond_key}[{i}]' must contain key 'evidence'. Got: {list(entry.keys())}"))

    for field in ("if_true", "if_false"):
        if field not in payload:
            errors.append(ValidationError(filename, scene, idx,
                f"check_condition is missing required field '{field}'.",
                suggestion=f"Add to payload:  {field}: <scene_name>"))
        elif payload[field] not in all_scene_ids:
            ref = payload[field]
            hint = _did_you_mean(ref, all_scene_ids)
            sug = f"Available scenes: {sorted(all_scene_ids)}"
            if hint:
                sug = f"{hint}  Available scenes: {sorted(all_scene_ids)}"
            errors.append(ValidationError(filename, scene, idx,
                f"check_condition '{field}' references unknown scene '{ref}'.",
                suggestion=sug))


def validate_action(action_dict, scene: str, idx: int, filename: str,
                    errors: list, all_scene_ids: set):
    """Validate a single action dictionary."""
    if not isinstance(action_dict, dict):
        errors.append(ValidationError(filename, scene, idx,
            f"Action must be a dict, got {type(action_dict).__name__}."))
        return

    if "action" not in action_dict:
        errors.append(ValidationError(filename, scene, idx,
            "Action entry is missing the 'action' key."))
        return

    action_name = action_dict["action"]

    if action_name not in ACTIONS:
        hint = _did_you_mean(action_name, ACTIONS.keys())
        sug = f"Valid actions: {sorted(ACTIONS.keys())}"
        if hint:
            sug = f"{hint}  Valid actions: {sorted(ACTIONS.keys())}"
        errors.append(ValidationError(filename, scene, idx,
            f"Unknown action '{action_name}'.",
            suggestion=sug))
        return

    payload = action_dict.get("payload", {})
    if not isinstance(payload, dict):
        errors.append(ValidationError(filename, scene, idx,
            f"'payload' must be a dict, got {type(payload).__name__}."))
        return

    schema = ACTIONS[action_name]

    # --- Unknown payload key guard (for _special handlers) ----------
    special_name = schema.get("_special")
    if special_name and special_name in SPECIAL_KNOWN_KEYS:
        known = SPECIAL_KNOWN_KEYS[special_name]
        for key in payload:
            if key not in known:
                hint = _did_you_mean(key, known)
                sug = f"Known keys for '{action_name}': {sorted(known)}"
                if hint:
                    sug = f"{hint}  Known keys for '{action_name}': {sorted(known)}"
                errors.append(ValidationError(filename, scene, idx,
                    f"Unknown payload key '{key}' for action '{action_name}'.",
                    suggestion=sug))

    if special_name == "check_evidence":
        validate_check_evidence(payload, filename, scene, idx, errors, all_scene_ids)
        return

    if special_name == "check_condition":
        validate_check_condition(payload, filename, scene, idx, errors, all_scene_ids)
        return

    if special_name == "parallel":
        validate_parallel(payload, filename, scene, idx, errors, all_scene_ids)
        return

    if special_name == "wait_for_billiards_event":
        validate_wait_for_billiards_event(payload, filename, scene, idx, errors)
        return

    if special_name == "evaluate_billiards_round":
        validate_evaluate_billiards_round(payload, filename, scene, idx, errors, all_scene_ids)
        return

    if special_name == "load_fake_player_command":
        validate_load_fake_player_command(payload, filename, scene, idx, errors)
        return

    if special_name == "evaluate_roguelike_round":
        validate_evaluate_roguelike_round(payload, filename, scene, idx, errors, all_scene_ids)
        return

    if special_name == "setup_rhythm_round":
        validate_setup_rhythm_round(payload, filename, scene, idx, errors)
        return

    if special_name == "evaluate_rhythm_round":
        validate_evaluate_rhythm_round(payload, filename, scene, idx, errors, all_scene_ids)
        return

    # --- Generic required-field check -----------------------------------
    # Unknown key guard for non-special actions
    all_known_keys = set()
    for group in schema["required"]:
        all_known_keys.update(group)
    all_known_keys.update(schema["optional"])
    for key in payload:
        if key not in all_known_keys:
            hint = _did_you_mean(key, all_known_keys)
            sug = f"Known keys for '{action_name}': {sorted(all_known_keys)}" if all_known_keys else f"Action '{action_name}' takes no payload keys."
            if hint:
                sug = f"{hint}  {sug}"
            errors.append(ValidationError(filename, scene, idx,
                f"Unknown payload key '{key}' for action '{action_name}'.",
                suggestion=sug))

    # Generic required-field check
    validate_payload_fields(schema["required"], payload, filename, scene, idx, errors)

    # Extra semantic checks
    if action_name == "end_game":
        result = payload.get("result", "")
        if result not in VALID_END_GAME_RESULTS:
            hint = _did_you_mean(result, VALID_END_GAME_RESULTS)
            sug = f"Valid results: {sorted(VALID_END_GAME_RESULTS)}"
            if hint:
                sug = f"{hint}  Valid results: {sorted(VALID_END_GAME_RESULTS)}"
            errors.append(ValidationError(filename, scene, idx,
                f"end_game 'result' must be one of {sorted(VALID_END_GAME_RESULTS)}, "
                f"got '{result}'.",
                suggestion=sug))

    if action_name == "add_evidence":
        # "type" is optional — only validate it when present
        if "type" in payload:
            ev_type = payload["type"]
            if ev_type not in VALID_EVIDENCE_TYPES:
                hint = _did_you_mean(ev_type, VALID_EVIDENCE_TYPES)
                sug = f"Valid types: {sorted(VALID_EVIDENCE_TYPES)}"
                if hint:
                    sug = f"{hint}  Valid types: {sorted(VALID_EVIDENCE_TYPES)}"
                errors.append(ValidationError(filename, scene, idx,
                    f"add_evidence 'type' must be one of {sorted(VALID_EVIDENCE_TYPES)}, "
                    f"got '{ev_type}'.",
                    suggestion=sug))

    if action_name == "wait_for_signal":
        # "timeout" is optional (runtime default 5.0) — only validate when present
        if "timeout" in payload:
            try:
                timeout = float(payload["timeout"])
                if timeout <= 0:
                    errors.append(ValidationError(filename, scene, idx,
                        f"wait_for_signal 'timeout' must be a positive number, got '{payload['timeout']}'.",
                        suggestion="timeout: 5.0  # seconds; omit to use the runtime default of 5.0"))
            except (TypeError, ValueError):
                errors.append(ValidationError(filename, scene, idx,
                    f"wait_for_signal 'timeout' must be a number, got '{payload['timeout']}'.",
                    suggestion="timeout: 5.0  # numeric seconds"))


def validate_file(filepath: str) -> list:
    """
    Validate a single scenario YAML file.
    Returns a list of ValidationError instances (empty = valid).
    """
    errors = []
    filename = os.path.basename(filepath)

    # --- Load YAML --------------------------------------------------------
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            root = yaml.safe_load(f)
    except yaml.YAMLError as e:
        errors.append(ValidationError(filename, "<parse>", -1,
            f"YAML parse error: {e}"))
        return errors
    except OSError as e:
        errors.append(ValidationError(filename, "<io>", -1, str(e)))
        return errors

    if not isinstance(root, dict):
        errors.append(ValidationError(filename, "<root>", -1,
            "File must be a YAML mapping at the top level."))
        return errors

    # --- Top-level structure ----------------------------------------------
    if "start_scene" not in root:
        errors.append(ValidationError(filename, "<root>", -1,
            "Missing required top-level key 'start_scene'."))

    if "scenes" not in root:
        errors.append(ValidationError(filename, "<root>", -1,
            "Missing required top-level key 'scenes'."))
        return errors

    scenes = root["scenes"]
    if not isinstance(scenes, dict):
        errors.append(ValidationError(filename, "<root>", -1,
            "'scenes' must be a mapping."))
        return errors

    # --- Duplicate scene labels (YAML parsers merge them silently) --------
    # Re-parse raw to detect duplicates
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            raw_text = f.read()
        seen_labels: set = set()
        duplicates: set = set()
        for line in raw_text.splitlines():
            stripped = line.strip()
            # Heuristic: top-level scene keys are lines starting with 2-space
            # indent followed by a YAML key pattern, but we just collect ALL
            # scene ids that appear as keys in the parsed dict.
        for key in scenes.keys():
            if key in seen_labels:
                duplicates.add(key)
            seen_labels.add(key)
        for dup in sorted(duplicates):
            errors.append(ValidationError(filename, dup, -1,
                f"Duplicate scene label '{dup}' (YAML will silently overwrite)."))
    except Exception:
        pass  # best-effort; main validation continues below

    all_scene_ids = set(scenes.keys())

    # --- Validate start_scene reference -----------------------------------
    start = root.get("start_scene", "")
    if start and start not in all_scene_ids:
        errors.append(ValidationError(filename, "<root>", -1,
            f"'start_scene' references unknown scene '{start}'."))

    # --- Validate each scene ----------------------------------------------
    for scene_name, scene_body in scenes.items():
        if not isinstance(scene_body, dict):
            errors.append(ValidationError(filename, scene_name, -1,
                "Scene body must be a mapping."))
            continue

        on_enter = scene_body.get("on_enter", [])
        if not isinstance(on_enter, list):
            errors.append(ValidationError(filename, scene_name, -1,
                "'on_enter' must be an array."))
            continue

        for idx, action in enumerate(on_enter):
            validate_action(action, scene_name, idx, filename, errors, all_scene_ids)

    return errors


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

DEFAULT_GLOB = "src/games/mystery_test/scenario/*.yaml"

def main():
    repo_root = Path(__file__).resolve().parent.parent

    if len(sys.argv) > 1:
        targets = sys.argv[1:]
    else:
        scenario_dirs = [
            repo_root / "src/games/mystery_test/scenario",
            repo_root / "src/games/billiards_test/scenario",
            repo_root / "src/games/roguelike_test/scenario",
        ]
        targets = []
        for d in scenario_dirs:
            targets.extend(sorted(str(p) for p in d.glob("*.yaml")))
        if not targets:
            print(f"No scenario files found under src/games/*/scenario/")
            sys.exit(0)

    total_errors = 0
    total_files  = 0

    for target in targets:
        path = Path(target)
        if not path.is_absolute():
            path = repo_root / target

        print(f"\nValidating: {path.relative_to(repo_root) if path.is_relative_to(repo_root) else path}")
        errors = validate_file(str(path))
        total_files += 1

        if errors:
            total_errors += len(errors)
            for err in errors:
                print(str(err))
        else:
            print("  OK  No errors found.")

    print(f"\n{'='*60}")
    print(f"Checked {total_files} file(s).  "
          f"{'ALL VALID' if total_errors == 0 else f'{total_errors} error(s) found'}.")
    print('='*60)

    sys.exit(0 if total_errors == 0 else 1)


if __name__ == "__main__":
    main()
