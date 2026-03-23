# roguelike_test Completion Memo

Date: 2026-03-21

This document fixes the current completion state of `roguelike_test` as an operational memo.
It is not a new design proposal and does not expand scope.

---

## 1. Purpose

`roguelike_test` is not a module for shipping a complete roguelike game.

Its purpose is to verify that a turn/entity/grid style module can safely run on Kernel v2.0 as a minimal fitness test, with no `src/core` modifications required by game-side implementation.

---

## 2. What roguelike_test validates

`roguelike_test` currently validates the following points.

- Fixed 5x5 grid state progression through ScenarioRunner
- Single-player and single-enemy turn processing
- WorldState SESSION-scope mutation for entity position/HP/result
- Branching correctness for `clear` / `fail` / `continue`
- Action registration and task compilation path under Kernel v2.0
- Deterministic smoke execution via fake command inputs

---

## 3. Current verified scope

Verified implementation scope at this time:

- Fixed map only (5x5)
- 1 player
- 1 enemy (`enemy_1`)
- 1 goal
- Fake command based player action selection
- Smoke scenarios for `clear` / `fail` / `continue`
- Runtime confirmation for all three paths
- Debug observer scene/overlay for live WorldState inspection
- `src/core` is not used as a feature-expansion target for roguelike design work
- Validator and regression path are passing for this scope

---

## 4. Runtime verification status

Runtime verification is completed for all three smoke paths.

- `clear` path: runtime confirmed
- `fail` path: runtime confirmed
- `continue` path: runtime confirmed

Observed in debug overlay:

- Position transitions (`player`, `enemy_1`)
- HP transitions (`player:hp`, `enemy_1:hp`)
- Result transitions (`round:result` = `clear`, `fail`, or empty for continue)
- Turn advance (`turn:index`)

Important note:

- This runtime confirmation does not mean production UI is complete.
- It confirms kernel-fitness behavior for the current minimal test scope.

---

## 5. WorldState contract snapshot

Namespace and scope:

- Namespace: `roguelike_test`
- Scope: `SCOPE_SESSION`

Current keys:

- `roguelike_test:turn:index`
- `roguelike_test:turn:phase`
- `roguelike_test:player:x`
- `roguelike_test:player:y`
- `roguelike_test:player:hp`
- `roguelike_test:enemy_1:x`
- `roguelike_test:enemy_1:y`
- `roguelike_test:enemy_1:hp`
- `roguelike_test:goal:x`
- `roguelike_test:goal:y`
- `roguelike_test:round:result`
- `roguelike_test:last_action:actor`
- `roguelike_test:last_action:type`

Rules:

- Store logical state only in WorldState.
- Do not store rendering state.
- Do not store per-frame transient presentation data.
- Debug overlay reads these keys as an observer; it does not own game logic.

---

## 6. Debug view positioning

The debug view is intentionally a diagnostic observer, not a finished game UI.

- Not production UI
- Observer/debug overlay only
- Used for replacement verification from mystery-shell-dependent flow
- Used to visualize WorldState and scenario progression
- Not a substitute for final game presentation

Related files:

- `samples/roguelike/roguelike_debug_scene.tscn`
- `samples/roguelike/roguelike_debug_overlay.gd`

---

## 7. What is intentionally out of scope

The following items are intentionally not addressed in this completion state.

- Real input integration
- Multi-enemy generalization
- Inventory and item systems
- FOV / visibility systems
- Procedural generation
- Richer enemy AI and advanced tactical behavior
- Presentation polish / production UI
- Generalized full-roguelike ruleset expansion

---

## 8. Related implementation notes

Relevant implementation flow (summary):

- `de1e3ba`: `roguelike_test` minimal skeleton
- `5c02458`: compile/include/namespace fixes in roguelike task headers
- `f952fad`: debug overlay + debug scene + roguelike docs additions
- `80d85ed`: YAML/ScenarioRunner related foundation fixes used by smoke/runtime stability
- `eaf761b`: runtime verification note reflected in kernel docs

Documentation linkage:

- Design spec: `docs/roguelike_test.md`
- Kernel matrix note: `docs/kernel_test_matrix.md`

---

## 9. Suggested next step

Keep next work minimal and explicit.

- If needed, define one small ticket for `last_action` lifecycle visibility policy (debug expectation vs turn-resolve reset timing) without expanding gameplay scope.
- Otherwise, move to a separate phase document for input integration planning, not implementation.

### Real Input Entry Conditions (deferred)

The current module is complete for fake-command based fitness goals.
If real input is considered later, entry conditions should be explicit:

- Input buffering/queue semantics are defined outside direct frame noise.
- A wait-style command acquisition step exists in scenario progression.
- Only finalized logical command results are written to WorldState.
- Interaction policy between input wait and turn progression is documented.

Real input implementation is intentionally deferred in this phase.

---

## 10. Final status statement

`roguelike_test` has reached a valid completion checkpoint as a Kernel v2.0 fitness test module for turn/entity/grid behavior.

Within the current intentional scope, validator and runtime evidence support that `clear` / `fail` / `continue` progression works, WorldState contract is observable, and no large-scope roguelike feature expansion has been introduced.
