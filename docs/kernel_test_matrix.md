# Kernel Test Matrix

This document defines the **Karakuri Game OS Kernel Test Matrix and Regression Strategy**.

---

## What This Repository Contains

The Karakuri Game OS repository serves two complementary purposes:

**1. A safe foundation for AI-generated games.**
The `src/core/` Kernel provides a stable, tested runtime — `ScenarioRunner`, `TaskSpec`,
`ActionRegistry`, `WorldState`, `KernelClock` — that AI agents can use to generate complete
games purely through YAML scenarios and game-module Task classes, without touching engine
internals. This guarantees that AI-generated content cannot break the runtime by construction.

**2. Reusable benchmark modules that validate Kernel capabilities.**
Each module under `src/games/` is a minimal, genre-representative game chosen specifically
because it exercises a distinct cluster of Kernel features. Together, the five planned modules
form a structured coverage suite that proves the Kernel is correct, complete, and regression-safe
across a wide variety of game genres and interaction patterns.

The two purposes reinforce each other: the benchmark modules are themselves AI-generated games,
so every passing test also validates the AI game-writing workflow end to end.

---

## Each Module Exists to Test Kernel Capabilities

No test module exists because of its genre. Each exists because that genre is the minimal
vehicle to force a specific Kernel subsystem into its hardest operating condition:

- **mystery_test** is not just a mystery game — it is the only module that exercises compound
  condition evaluation, SESSION-scope state accumulation, and signal-timeout interaction
  all together.
- **billiards_test** is not just a physics demo — it is the only module that stresses
  event-driven ScenarioRunner re-entry from a physics callback loop.
- **roguelike_test** is not just an RPG — it is the only module that requires
  turn-order task scheduling and multi-entity WorldState mutation at SESSION scope.
- **rhythm_test** is not just a music game — it is the only module that verifies
  KernelClock frame-independent scheduling and beat-level determinism.
- **sandbox_test** is not just a simulation — it is the only module that stresses
  high-frequency GLOBAL-scope mutations and large-graph ScenarioRunner execution simultaneously.

---

## Test Suite Overview

| Test Game | Primary Genre Concern | Kernel Features Verified | Example Tasks | Current Status |
|---|---|---|---|---|
| `mystery_test` | Narrative / branching / state | ScenarioRunner state machine, WorldState SESSION mutation, compound condition evaluation (`all_of`/`any_of`), KernelClock timeout safety, signal handshake, save/load round-trip | `ShowDialogueTask`, `DiscoverEvidenceTask`, `CheckConditionTask`, `WaitForSignalTask`, `EndGameTask`, `SaveLoadTestTask` | **Active** |
| `billiards_test` | Physics / event reaction | Event-driven task dispatch, collision-triggered WorldState updates, transient SCENE-scope state, ScenarioRunner re-entry from physics layer | `CollisionReactTask`, `ScoreUpdateTask`, `ResetBallTask` | Planned |
| `roguelike_test` | Entity / turn / progression | Turn progression scheduling, entity WorldState mutation (position/HP), fixed-grid movement, occupancy/collision-safe resolution, SESSION-scope progression persistence, clear/fail/continue branch evaluation | `SetupRoguelikeRoundTask`, `ApplyPlayerMoveTask`, `ApplyPlayerAttackTask`, `ApplyEnemyTurnTask`, `ResolveRoguelikeTurnTask`, `EvaluateRoguelikeRoundTask` | **Active** |
| `rhythm_test` | Time / clock / deterministic scheduling | KernelClock frame-independent scheduling, deterministic beat timing, high-frequency task dispatch, clock drift detection | `BeatScheduleTask`, `InputJudgeTask`, `ScoreComboTask` | Planned |
| `sandbox_test` | Scale / dynamic state / large world mutation | High-frequency WorldState writes, large-graph ScenarioRunner execution, dynamic task registration via ActionRegistry, GLOBAL-scope persistence at scale | `TileUpdateTask`, `ResourceHarvestTask`, `WorldChunkLoadTask` | Planned |

---

## Coverage by Kernel Subsystem

| Kernel Subsystem | mystery | billiards | roguelike | rhythm | sandbox |
|---|---|---|---|---|---|
| ScenarioRunner | full | partial | full | partial | full |
| TaskSpec | full | partial | full | partial | partial |
| ActionRegistry | full | partial | full | partial | full |
| WorldState | full | partial | full | partial | full |
| KernelClock | partial | partial | partial | full | partial |
| Signal Handshake | full | partial | planned | planned | planned |
| Condition Logic | full | planned | partial | planned | partial |
| Save/Load Safety | partial | planned | full | planned | full |
| High-Scale Mutation | partial | planned | partial | planned | full |

### Roguelike Runtime Note (2026-03-21)

- Runtime verification completed for all three smoke paths: `clear`, `fail`, `continue`.
- Observed state transitions are consistent with scenario expectations:
  - turn progression confirmed (`turn:index` advance).
  - entity state updates confirmed (player/enemy position and HP).
  - grid movement and occupancy/collision-safe behavior confirmed.
  - `clear`: goal reached, `round:result = clear`.
  - `fail`: player HP reached 0, `round:result = fail`.
  - `continue`: turn advanced with coordinate updates while `round:result` remained empty.

**Key:** `full` = primary coverage target for this module · `partial` = incidental coverage · `planned` = intended, not yet implemented

Reading this table column-by-column shows what each module is designed to prove.
Reading row-by-row shows which subsystems still have coverage gaps.

---

## Regression Strategy

### Purpose

The five test modules together form a **regression suite for the Karakuri Kernel**. A test
module that previously passed and now fails means a Kernel API contract has changed in a
breaking way. The correct response is to fix the Kernel, not the test module.

### Rules for Running Regression

1. **Run validator first.** Before executing any test scenario, run the static validator:
   ```
   python3 tools/validate_scenario.py
   ```
   A YAML validation failure is a scenario authoring bug, not a Kernel regression.

2. **Treat task compilation failures as ActionRegistry regressions.** If
   `ActionRegistry::compile_task()` returns null for a known action name, the registration
   contract in `src/core/` has changed.

3. **Treat WorldState read/write mismatches as scope contract regressions.** If evidence
   written by `discover_evidence` (SCOPE_SESSION) cannot be read by `check_condition`
   (SCOPE_SESSION), the scope isolation guarantee has been broken.

4. **Treat `WaitForSignalTask` timeout failures as KernelClock regressions.** The timeout
   logic depends on `KernelClock::now()` advancing monotonically. A stuck or reversed clock
   is a Kernel bug.

### Guidance for Adding New Test Modules

- **Cover missing capability, not missing genre.** Before proposing a new module, inspect the
  subsystem coverage table above. A valid new module must turn at least one `planned` or
  `partial` cell to `full` without duplicating an existing `full` cell.

- **Stay minimal and capability-focused.** A test module should be the smallest possible game
  that creates sufficient load on its target subsystem. Avoid large asset dependencies, complex
  UI, or cross-module shared state.

- **Never modify `src/core/`.** The entire purpose of a fitness test is to prove the Kernel
  works as delivered. Any test that requires a `src/core/` change to pass is a bug report for
  the Kernel team, not a new benchmark.

- **AI agents must consult this matrix before proposing tests.** Before generating a new
  scenario or task, an AI agent should read this file to confirm: (a) the target subsystem is
  not already fully covered, and (b) the proposed approach fills a documented gap.

- **Update this file when adding a module.** Add a row to the Test Suite Overview table, update
  the subsystem coverage cells, and increment the status from `Planned` to `Active`.

---

## Why This Exists

Karakuri is a **Game OS Kernel**, not a sample game collection.

Its thesis is: *if the Kernel is correct, an AI agent can generate any genre of game as pure
data (YAML + Tasks) without writing engine code or modifying `src/core/`.*

Proving that thesis requires more than one working game. It requires a structured set of games,
each chosen to stress a different subsystem, together forming a test suite that gives confidence
the Kernel is production-ready across a broad range of use cases.

This matrix is the map of that proof. It records what has been tested, what is planned, and
what gaps remain. Every time a new module passes, one more row of the Kernel's correctness
guarantee is confirmed by evidence rather than assumption.

The benchmark is also self-referential: the test modules are themselves AI-generated games.
A test suite that an AI cannot write is not a useful benchmark for an AI game-writing platform.
The fact that `mystery_test` was generated entirely through AI-assisted development and now
serves as a passing regression test is the first proof-of-concept of the platform's own goal.
- **Add a row to this table** before writing any code, so the intended coverage is visible
  to reviewers and future agents before implementation begins.
