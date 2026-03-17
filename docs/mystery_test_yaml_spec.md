# mystery_test YAML Scenario Specification

This document defines the DSL (Domain-Specific Language) used by scenario files in
`src/games/mystery_test/scenario/`.

Run `python3 tools/validate_scenario.py` to check any scenario file against this spec.

---

## Top-Level Structure

```yaml
start_scene: <scene_id>   # required: ID of the first scene to execute

scenes:                    # required: mapping of scene_id -> scene body
  <scene_id>:
    on_enter:              # optional: array of actions to execute when entering this scene
      - action: <name>
        payload: { ... }
```

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `start_scene` | string | YES | Must match an existing scene ID |
| `scenes` | mapping | YES | One entry per scene |
| `scenes.<id>.on_enter` | array | NO | Actions executed in order when scene is entered |

---

## Actions

### `show_dialogue`

Display a line of dialogue.

```yaml
- action: show_dialogue
  payload:
    speaker: detective        # required: speaker identifier
    text: "セリフのテキスト"    # required: dialogue text
```

| Field | Required | Values |
| --- | --- | --- |
| `speaker` | YES | any string (`narrator`, `detective`, `suspect`, `system`, …) |
| `text` | YES | any string |

---

### `discover_evidence`

Find evidence and write it to `WorldState` SESSION scope.
Key written: `mystery_test:evidence:<id>`

```yaml
- action: discover_evidence
  payload:
    evidence_id: knife        # required (canonical); legacy key "id" also accepted
    location: kitchen         # required: where it was found (for log output)
```

| Field | Required | Notes |
| --- | --- | --- |
| `evidence_id` | YES* | Preferred. `id` is accepted as a legacy fallback |
| `location` | YES | Descriptive string only; not written to WorldState |

Duplicate discovery is silently skipped (idempotent).

---

### `add_evidence`

Add evidence to inventory with a type tag.
Key written: `mystery_test:evidence:<id>`

```yaml
- action: add_evidence
  payload:
    id: house_map             # required: evidence identifier
    type: item                # required: category tag
```

| Field | Required | Valid values for `type` |
| --- | --- | --- |
| `id` | YES | any string |
| `type` | NO | `item` `motive` `state` `tool` `trait` `discovery` `evidence` `achievement` `dummy` (validated only when present) |

---

### `wait_for_signal`

Pause execution until an external signal arrives or the timeout expires.

```yaml
- action: wait_for_signal
  payload:
    signal: user_confirm      # required: signal name to wait for
    timeout: 3.0              # required: positive float (seconds)
```

| Field | Required | Notes |
| --- | --- | --- |
| `signal` | YES | Signal name string |
| `timeout` | NO | Positive number (float). Must be > 0 when present. Defaults to `5.0` if omitted. |

---

### `check_condition`

Evaluate a compound boolean condition and jump to a scene.

```yaml
# all_of form — true only if ALL listed evidence flags exist
- action: check_condition
  payload:
    all_of:
      - evidence: knife
      - evidence: fingerprint
    if_true: accuse_chef
    if_false: accuse_butler

# any_of form — true if AT LEAST ONE listed evidence flag exists
- action: check_condition
  payload:
    any_of:
      - evidence: witness_statement
      - evidence: secret_note
    if_true: investigate_office
    if_false: question_butler
```

| Field | Required | Notes |
| --- | --- | --- |
| `all_of` OR `any_of` | YES | Exactly one must be present; both is an error |
| each entry | — | Must be a dict with key `evidence` |
| `if_true` | YES | Must be an existing scene ID |
| `if_false` | YES | Must be an existing scene ID |

Missing evidence is treated as **false**.

---

### `check_evidence`

Branch based on whether a single evidence flag exists in `WorldState` **SCOPE_GLOBAL**.
Pairs with `add_evidence` (same scope).

```yaml
- action: check_evidence
  payload:
    id: motive_inheritance    # required: evidence ID to test
    if_true: confront_scene   # required: scene ID when evidence is present
    if_false: investigate     # required: scene ID when evidence is absent
```

| Field | Required | Notes |
| --- | --- | --- |
| `id` | YES | Evidence ID to test in SCOPE_GLOBAL |
| `if_true` | YES | Must be an existing scene ID |
| `if_false` | YES | Must be an existing scene ID |

> **Scope warning:** `check_evidence` reads from **SCOPE_GLOBAL** (set by `add_evidence`).
> Do NOT use it to test evidence written by `discover_evidence` (SCOPE_SESSION).

---

### `save_load_test`

Diagnostic task that exercises the WorldState `serialize` → `clear` → `deserialize` round-trip.
Use only in diagnostic scenarios — not in story scenarios.

```yaml
- action: save_load_test
  payload:
    key: my_persistence_flag  # optional: flag key used during the test
```

| Field | Required | Notes |
| --- | --- | --- |
| `key` | NO | Defaults to `savetest_flag` when omitted |

Returns `Success` if the round-trip is correct; `Failed` if the persistence check fails.

---

### `parallel`

Run multiple sub-tasks concurrently. Every sub-task is ticked each frame.
The group returns `Success` when all sub-tasks have succeeded;
`Failed` if any sub-task fails; `Waiting` otherwise.

```yaml
- action: parallel
  payload:
    tasks:
      - action: show_dialogue
        payload:
          speaker: narrator
          text: "部屋を調べている..."
      - action: wait_for_signal
        payload:
          signal: search_complete
          timeout: 4.0
```

| Field | Required | Notes |
| --- | --- | --- |
| `tasks` | YES | Non-empty array of `{ action, payload }` objects |

**Forbidden sub-actions** (validator will reject these inside `tasks`):
`check_evidence`, `check_condition`, `end_game`, `parallel`

Branching and terminal actions must not appear inside a parallel group because their
scene-jump / game-end side effects are not coordinated with sibling sub-tasks.

---

### `end_game`

Terminate the scenario with a result code.

```yaml
- action: end_game
  payload:
    result: solved            # required: result code
```

| Field | Required | Valid values |
| --- | --- | --- |
| `result` | YES | `solved` `wrong` `failed` `lost` `timeout` |

---

## Validation Rules Summary

The validator (`tools/validate_scenario.py`) enforces:

| Rule | What is checked |
| --- | --- |
| Unknown action | `action` name must be in the known set |
| Missing required fields | All required payload fields must be present |
| Invalid jump target | `if_true` / `if_false` must reference existing scene IDs |
| Duplicate scene labels | Two scenes with the same ID is an error |
| Malformed condition | `all_of`/`any_of` mutual exclusion; each entry must have `evidence` key |
| `timeout` when present | Must be a positive number (>0) |
| `type` when present | `add_evidence.type` must be from the allowed set |
| `end_game.result` | Must be one of: `solved` `wrong` `failed` `lost` `timeout` |
| `parallel.tasks` | Must be a non-empty array; forbidden actions rejected; sub-task fields validated |
| Nested parallel | Forbidden — `parallel` may not appear inside `tasks` |
| Missing `start_scene` | Top-level key is required |
| `start_scene` target | Must reference an existing scene ID |
