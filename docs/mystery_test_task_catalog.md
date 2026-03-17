# mystery_test Task Catalog

**Target audience:** AI agents and human developers writing YAML scenarios for the mystery_test module.

This document describes every available task action, its payload contract, its runtime behavior,
and its WorldState interaction. Use this as the authoritative reference before writing scenario YAML.

---

## Quick Reference Table

| YAML `action` | Category | Returns | Mutates State | Branches |
|---|---|---|---|---|
| `show_dialogue` | Dialogue | `Waiting` | No | No |
| `add_evidence` | Mutation | `Success` | SCOPE_GLOBAL | No |
| `discover_evidence` | Mutation | `Success` | SCOPE_SESSION | No |
| `check_evidence` | Branch | `Success` | No (read-only) | Yes |
| `check_condition` | Branch | `Success` | No (read-only) | Yes |
| `wait_for_signal` | Timeout | `Waiting`/`Failed` | No | No |
| `end_game` | Terminal | `Success` | No | No |
| `save_load_test` | Diagnostic | `Success`/`Failed` | SCOPE_GLOBAL (temp) | No |
| `parallel` | Composite | `Waiting`/`Success`/`Failed` | Depends on sub-tasks | No |

> **Scope note:** `add_evidence` + `check_evidence` use **SCOPE_GLOBAL** (persists across sessions).
> `discover_evidence` + `check_condition` use **SCOPE_SESSION** (current run only).
> Do NOT mix these pairs — they write and read from different scopes.

---

## Category 1 — Dialogue / Output

### `show_dialogue`

**Purpose:** Display a single line of dialogue to the player. Pauses scenario execution until the
dialogue is acknowledged (the runner's "waiting for dialogue" flag is cleared by the UI layer).

**Behavior:** Returns `Waiting`. Execution does NOT proceed to the next task until
`set_waiting_for_dialogue(false)` is called externally (typically by the UI component).

**WorldState interaction:** None.

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `speaker` | String | Speaker identifier shown to the UI (e.g. `detective`, `narrator`, `suspect`, `system`) |
| `text` | String | Dialogue text content |

**Optional payload fields:** None.

**Signal emitted:** `dialogue_requested(speaker: String, text: String)` on the ScenarioRunner.

**Example:**
```yaml
- action: show_dialogue
  payload:
    speaker: detective
    text: "書斎に入ると、金庫が半開きになっていた。"
```

---

## Category 2 — Mutation Tasks

Mutation tasks write evidence flags into WorldState. They complete instantly (`Success`) unless
WorldState is unavailable.

---

### `add_evidence`

**Purpose:** Unconditionally add a named evidence flag to **SCOPE_GLOBAL** WorldState. Used for
abstract game facts: motives, traits, achievements, and synthetic evidence IDs that do not
correspond to a physical discovery.

**Behavior:** Writes `evidence:<id> = true` to `SCOPE_GLOBAL` and returns `Success`.
No duplicate guard — calling this multiple times for the same `id` is safe (idempotent write).

**WorldState interaction:**
- Scope: `SCOPE_GLOBAL`
- Namespace: `mystery_test`
- Key written: `evidence:<id>` → `true`

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `id` | String | Evidence identifier to set (snake_case recommended) |

**Optional payload fields:**

| Field | Type | Description |
|---|---|---|
| `type` | String | Semantic label for human/AI readability (`evidence`, `motive`, `trait`, `achievement`). **Ignored at runtime.** |

**Example:**
```yaml
- action: add_evidence
  payload:
    id: motive_inheritance
    type: motive
```

> **Important:** Evidence written here is readable by `check_evidence` (SCOPE_GLOBAL),
> but NOT by `check_condition` (SCOPE_SESSION). Do not expect `check_condition`
> to see flags written by `add_evidence`.

---

### `discover_evidence`

**Purpose:** Record that the player has physically discovered a piece of evidence at a location.
Writes to **SCOPE_SESSION** and includes a duplicate guard — re-discovering the same evidence
is a silent no-op.

**Behavior:** Checks `has_flag` before writing. If already discovered, logs a skip message and
returns `Success` without overwriting. Otherwise writes `evidence:<evidence_id> = true` to
`SCOPE_SESSION`.

**WorldState interaction:**
- Scope: `SCOPE_SESSION`
- Namespace: `mystery_test`
- Key written: `evidence:<evidence_id>` → `true`

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `evidence_id` | String | Canonical evidence identifier (preferred field name) |
| `location` | String | Physical location identifier where the evidence was found |

**Legacy field:** `id` is accepted as an alias for `evidence_id` if `evidence_id` is absent.
Prefer `evidence_id` in new scenarios.

**Optional payload fields:** None.

**Example:**
```yaml
- action: discover_evidence
  payload:
    evidence_id: torn_will
    location: study_desk
```

> **Important:** Evidence written here is readable by `check_condition` (SCOPE_SESSION),
> but NOT by `check_evidence` (SCOPE_GLOBAL). Do not use `check_evidence` to test
> flags written by `discover_evidence`.

---

## Category 3 — Branching Tasks

Branching tasks read WorldState and call `load_scene_by_id` to jump to another scene.
They always return `Success` (the branch jump itself is the outcome, not a wait state).

---

### `check_evidence`

**Purpose:** Branch based on whether a single named evidence flag exists in **SCOPE_GLOBAL**.
Pairs with `add_evidence`.

**Behavior:** Reads `evidence:<id>` from `SCOPE_GLOBAL`. If true, jumps to `if_true`;
otherwise jumps to `if_false`. Calls `runner->load_scene_by_id(target)` and returns `Success`.

**WorldState interaction:**
- Scope: `SCOPE_GLOBAL` (read-only)
- Namespace: `mystery_test`
- Key read: `evidence:<id>`

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `id` | String | Evidence ID to test |
| `if_true` | String | Scene ID to jump to when evidence is present |
| `if_false` | String | Scene ID to jump to when evidence is absent |

**Optional payload fields:** None.

**Example:**
```yaml
- action: check_evidence
  payload:
    id: motive_inheritance
    if_true: confrontation_scene
    if_false: investigation_continues
```

---

### `check_condition`

**Purpose:** Branch based on a compound condition over one or more evidence flags in
**SCOPE_SESSION**. Supports `all_of` (logical AND) and `any_of` (logical OR). Pairs with
`discover_evidence`.

**Behavior:** Evaluates the condition list against `SCOPE_SESSION`. Calls
`runner->load_scene_by_id(target)` and returns `Success`.

**WorldState interaction:**
- Scope: `SCOPE_SESSION` (read-only)
- Namespace: `mystery_test`
- Keys read: `evidence:<id>` for each entry in the condition list

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `all_of` **or** `any_of` | Array of `{evidence: String}` | Condition list. Exactly one must be present. `all_of` = every entry must be true. `any_of` = at least one must be true. |
| `if_true` | String | Scene ID when condition is satisfied |
| `if_false` | String | Scene ID when condition is not satisfied |

> **Constraint:** `all_of` and `any_of` are mutually exclusive. Providing both, or neither,
> is a validation error that returns `ERR_INVALID_DATA` and stops the scenario.

**Optional payload fields:** None.

**Single-evidence example (`any_of`):**
```yaml
- action: check_condition
  payload:
    any_of:
      - evidence: torn_will
    if_true: bedroom
    if_false: study_room
```

**Multi-evidence AND example (`all_of`):**
```yaml
- action: check_condition
  payload:
    all_of:
      - evidence: motive_inheritance
      - evidence: chemical_solvent
    if_true: confrontation_scene
    if_false: hall_east
```

**Multi-evidence OR example (`any_of`):**
```yaml
- action: check_condition
  payload:
    any_of:
      - evidence: servant_note
      - evidence: conspiracy_evidence
    if_true: suspect_chain_1
    if_false: lobby
```

> **Common mistake:** Do NOT write `evidence: bloodstain` directly under `payload:`.
> This shorthand is NOT supported. Always wrap inside `any_of:` or `all_of:`.

---

## Category 4 — Waiting / Timeout Tasks

### `wait_for_signal`

**Purpose:** Pause scenario execution for a named signal or until a timeout elapses.
Used to test KernelClock determinism and to introduce deliberate pacing.

**Behavior:** On each `execute()` call, compares the current `KernelClock::now()` against
`start_time_ + timeout_`. Returns `Waiting` while time remains. Returns `Failed`
(with a push_warning) once the timeout elapses. The named signal is recorded but the current
implementation uses the timeout as the primary resolution mechanism.

**WorldState interaction:** None.

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `signal` | String | Logical signal name (recorded in logs; used as intent label) |

**Optional payload fields:**

| Field | Type | Default | Description |
|---|---|---|---|
| `timeout` | Float | `5.0` | Maximum seconds to wait before returning `Failed` |

**Returns:** `Waiting` each frame until timeout; then `Failed`.

**Example:**
```yaml
- action: wait_for_signal
  payload:
    signal: examine_safe
    timeout: 3.0
```

> **Design note:** `WaitForSignalTask` returns `Failed` on timeout, not `Success`.
> Subsequent tasks in the same scene's `on_enter` list will NOT execute after a timeout.
> If you need to branch on timeout vs. completion, use `check_condition` in a separate scene
> reached after the wait node.

---

## Category 5 — Terminal Tasks

### `end_game`

**Purpose:** Terminate the scenario with a named outcome. This is always the last action
in a scene's `on_enter` list.

**Behavior:** Emits `game_ended(result: String)` on the ScenarioRunner and returns `Success`.
No further tasks execute after this.

**WorldState interaction:** None.

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `result` | String | Outcome string sent to the UI layer |

**Known valid `result` values used in existing scenarios:**

| Value | Meaning |
|---|---|
| `solved` | Case solved successfully |
| `wrong` | Wrong conclusion reached |
| `timeout` | Time expired before resolution |

**Example:**
```yaml
- action: end_game
  payload:
    result: solved
```

---

## Category 6 — Diagnostic Tasks

Diagnostic tasks exist to exercise engine subsystems. They should not appear in story-driven
scenario files.

---

### `save_load_test`

**Purpose:** Self-contained smoke test for the WorldState persistence subsystem. Writes a flag
to `SCOPE_GLOBAL`, serializes the entire global scope, clears it, deserializes, and verifies
the flag was restored correctly. Returns `Success` if the round-trip is correct; `Failed` if
the persistence check fails.

**Behavior:** Mutates and then restores `SCOPE_GLOBAL` as a side-effect. After completion,
the tested flag remains in `SCOPE_GLOBAL`.

**WorldState interaction:**
- Scope: `SCOPE_GLOBAL` (write, serialize, clear, deserialize, read)
- Namespace: `mystery_test`
- Key used: value of `key` field

**Required payload fields:** None.

**Optional payload fields:**

| Field | Type | Default | Description |
|---|---|---|---|
| `key` | String | `savetest_flag` | Flag key used during the persistence test |

**Example:**
```yaml
- action: save_load_test
  payload:
    key: my_persistence_check
```

---

## Category 7 — Composite Tasks

### `parallel`

**Purpose:** Execute multiple sub-tasks concurrently within the same scene. All sub-tasks are
ticked each frame. The group returns `Success` only when every sub-task has returned `Success`
or `Failed`. If any sub-task returns `Failed`, the group returns `Failed`.

**Behavior:** On each `execute()` call, iterates all non-finished sub-tasks and calls their
`execute()`. Tracks completion state per sub-task. If any returns `Failed`, the entire group
fails immediately. `complete_instantly()` is forwarded to all sub-tasks.

**WorldState interaction:** Entirely dependent on the sub-tasks contained in the `tasks` array.

**Required payload fields:**

| Field | Type | Description |
|---|---|---|
| `tasks` | Array of `{action: String, payload: Dictionary}` | Sub-task definitions, same shape as a normal YAML task node |

**Optional payload fields:** None.

**Example:**
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

> **Constraint:** Sub-tasks that branch (`check_evidence`, `check_condition`) or terminate
> (`end_game`) should NOT be placed inside a `parallel` group — their scene-jump or
> game-end side effects are not coordinated with sibling sub-task state.

---

## Scope Reference

| Scope | Lifetime | Primary tasks |
|---|---|---|
| `SCOPE_GLOBAL` | Persists across saves/loads | `add_evidence`, `check_evidence`, `save_load_test` |
| `SCOPE_SESSION` | Current play session only | `discover_evidence`, `check_condition` |

**Rule:** Always pair tasks within the same scope family:

```
add_evidence      →  check_evidence      (both SCOPE_GLOBAL)
discover_evidence →  check_condition     (both SCOPE_SESSION)
```

Crossing scopes (e.g. `discover_evidence` then `check_evidence`) will silently produce
the wrong result because the flag is written to SESSION but read from GLOBAL.

---

## Task Selection Guide

| Intent | Use this task |
|---|---|
| Show story text / pause for player | `show_dialogue` |
| Set a motive, trait, or abstract game flag | `add_evidence` |
| Record a physical evidence discovery with dedup guard | `discover_evidence` |
| Branch on a single abstract flag (GLOBAL) | `check_evidence` |
| Branch on one or more discovered evidences (SESSION) | `check_condition` |
| Pause execution for a timed beat | `wait_for_signal` |
| End the scenario with an outcome | `end_game` |
| Test save/load subsystem | `save_load_test` |
| Run multiple task streams simultaneously | `parallel` |

---

## Validation Checklist for AI Agents

Before generating a scenario YAML, verify:

- [ ] Every `check_condition` payload has exactly one of `all_of` or `any_of` (not both, not neither)
- [ ] Every entry in `all_of`/`any_of` is `{ evidence: <id> }` — no other keys
- [ ] Every `if_true` and `if_false` value refers to a scene ID defined elsewhere in the same scenario
- [ ] Evidence IDs used in `check_condition` were previously set by `discover_evidence` (same scope)
- [ ] Evidence IDs used in `check_evidence` were previously set by `add_evidence` (same scope)
- [ ] `end_game` `result` is one of: `solved`, `wrong`, `timeout`
- [ ] `wait_for_signal` `timeout` is a positive float (seconds)
- [ ] `parallel` sub-tasks do not include `check_condition`, `check_evidence`, or `end_game`
- [ ] `show_dialogue` has both `speaker` and `text`
- [ ] `discover_evidence` has both `evidence_id` (or `id`) and `location`

Run `python3 tools/validate_scenario.py <file>` to catch structural errors automatically.
