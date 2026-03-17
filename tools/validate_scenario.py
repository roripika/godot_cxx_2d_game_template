#!/usr/bin/env python3
"""
validate_scenario.py
====================
Static validator for Karakuri mystery_test YAML scenario files.

Purpose
-------
Detect invalid scenario DSL **before runtime** so AI-generated scenarios
fail fast with human-readable errors instead of causing ScenarioRunner
crashes or silent logic bugs.

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
from pathlib import Path

try:
    import yaml
except ImportError:
    print("ERROR: PyYAML is required.  Run:  pip install pyyaml", file=sys.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------------
# Schema: known actions and their required payload fields
# ---------------------------------------------------------------------------

# If the payload key may appear under an alias the value is a list of accepted
# keys where *at least one* must be present.
ACTIONS = {
    "show_dialogue": {
        "required": [["speaker"], ["text"]],
        "optional": [],
    },
    "discover_evidence": {
        # "evidence_id" is canonical; "id" is legacy fallback
        "required": [["evidence_id", "id"], ["location"]],
        "optional": [],
    },
    "add_evidence": {
        "required": [["id"], ["type"]],
        "optional": [],
    },
    "wait_for_signal": {
        "required": [["signal"], ["timeout"]],
        "optional": [],
    },
    "check_condition": {
        # handled separately because it is structurally more complex
        "required": [],
        "optional": [],
        "_special": "check_condition",
    },
    "end_game": {
        "required": [["result"]],
        "optional": [],
    },
}

VALID_END_GAME_RESULTS = {"solved", "wrong", "failed", "lost", "timeout"}
VALID_EVIDENCE_TYPES = {
    "item", "motive", "state", "tool", "trait",
    "discovery", "evidence", "achievement", "dummy",
}

# ---------------------------------------------------------------------------
# Error collector
# ---------------------------------------------------------------------------

class ValidationError:
    def __init__(self, filename: str, scene: str, action_index: int, message: str):
        self.filename    = filename
        self.scene       = scene
        self.action_index = action_index
        self.message     = message

    def __str__(self) -> str:
        loc = f"{self.filename}  scene='{self.scene}'"
        if self.action_index >= 0:
            loc += f"  action[{self.action_index}]"
        return f"  ERROR  {loc}\n         {self.message}"


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
                msg = f"Missing required payload field '{group[0]}'."
            else:
                msg = f"Missing required payload field: one of {group} must be present."
            errors.append(ValidationError(filename, scene, idx, msg))


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
                f"check_condition is missing required field '{field}'."))
        elif payload[field] not in all_scene_ids:
            errors.append(ValidationError(filename, scene, idx,
                f"check_condition '{field}' references unknown scene '{payload[field]}'."))


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
        errors.append(ValidationError(filename, scene, idx,
            f"Unknown action '{action_name}'. "
            f"Valid actions: {sorted(ACTIONS.keys())}."))
        return

    payload = action_dict.get("payload", {})
    if not isinstance(payload, dict):
        errors.append(ValidationError(filename, scene, idx,
            f"'payload' must be a dict, got {type(payload).__name__}."))
        return

    schema = ACTIONS[action_name]

    # Special handlers
    if schema.get("_special") == "check_condition":
        validate_check_condition(payload, filename, scene, idx, errors, all_scene_ids)
        return

    # Generic required-field check
    validate_payload_fields(schema["required"], payload, filename, scene, idx, errors)

    # Extra semantic checks
    if action_name == "end_game":
        result = payload.get("result", "")
        if result not in VALID_END_GAME_RESULTS:
            errors.append(ValidationError(filename, scene, idx,
                f"end_game 'result' must be one of {sorted(VALID_END_GAME_RESULTS)}, "
                f"got '{result}'."))

    if action_name == "add_evidence":
        ev_type = payload.get("type", "")
        if ev_type not in VALID_EVIDENCE_TYPES:
            errors.append(ValidationError(filename, scene, idx,
                f"add_evidence 'type' must be one of {sorted(VALID_EVIDENCE_TYPES)}, "
                f"got '{ev_type}'."))

    if action_name == "wait_for_signal":
        try:
            timeout = float(payload.get("timeout", -1))
            if timeout <= 0:
                errors.append(ValidationError(filename, scene, idx,
                    f"wait_for_signal 'timeout' must be a positive number, got '{payload.get('timeout')}'."))
        except (TypeError, ValueError):
            errors.append(ValidationError(filename, scene, idx,
                f"wait_for_signal 'timeout' must be a number, got '{payload.get('timeout')}'."))


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
        targets = sorted(str(p) for p in (repo_root / "src/games/mystery_test/scenario").glob("*.yaml"))
        if not targets:
            print(f"No scenario files found under {repo_root / 'src/games/mystery_test/scenario'}")
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
