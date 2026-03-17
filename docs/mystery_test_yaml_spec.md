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
| `type` | YES | `item` `motive` `state` `tool` `trait` `discovery` `evidence` `achievement` `dummy` |

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
| `timeout` | YES | Positive number (float). Must be > 0. Prevents deadlock. |

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
| Missing fields | All required payload fields must be present |
| Invalid jump target | `if_true` / `if_false` must reference existing scene IDs |
| Duplicate scene labels | Two scenes with the same ID is an error |
| Malformed condition | `all_of`/`any_of` mutual exclusion; each entry must have `evidence` key |
| Malformed wait block | `timeout` must be a positive number |
| Invalid enum values | `end_game.result` and `add_evidence.type` must be from allowed sets |
| Missing `start_scene` | Top-level key is required |
| `start_scene` target | Must reference an existing scene ID |
