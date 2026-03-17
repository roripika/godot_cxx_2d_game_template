# Kernel Fitness Test Matrix

This document describes the **Karakuri Game OS Kernel Fitness Test Suite**.

Each test game in this suite exists for one reason: to measure whether Kernel v2.0 can support
a complete, genre-appropriate game using only:

- `ScenarioRunner`
- `TaskSpec` typed IR
- `ActionRegistry`
- `WorldState`
- `KernelClock`
- Game-module `Task` classes
- YAML scenario data

**without modifying `src/core/`.**

This is not a sample game collection. It is a structured coverage framework for the Kernel.

---

## Test Suite Overview

| Test Game | Primary Genre Concern | Kernel Features Verified | Example Tasks | Current Status |
| --- | --- | --- | --- | --- |
| `mystery_test` | Narrative / branching / state | ScenarioRunner state machine, WorldState SESSION mutation, compound condition evaluation, KernelClock timeout safety, signal handshake | `ShowDialogueTask`, `DiscoverEvidenceTask`, `CheckConditionTask`, `WaitForSignalTask`, `EndGameTask` | **Active** |
| `billiards_test` | Physics / event reaction | Event-driven task dispatch, collision-triggered WorldState updates, transient SCENE-scope state, ScenarioRunner reentry | `CollisionReactTask`, `ScoreUpdateTask`, `ResetBallTask` | Planned |
| `roguelike_test` | Entity / turn / progression | Turn-order scheduling via ScenarioRunner, entity-level WorldState mutation, SESSION-scope progression persistence, ActionRegistry multi-entity registration | `TurnAdvanceTask`, `EnemyMoveTask`, `LevelUpTask`, `PersistProgressTask` | Planned |
| `rhythm_test` | Time / clock / deterministic scheduling | KernelClock frame-independent scheduling, deterministic beat timing, high-frequency task dispatch, clock drift detection | `BeatScheduleTask`, `InputJudgeTask`, `ScoreComboTask` | Planned |
| `sandbox_test` | Scale / dynamic state / large world mutation | High-frequency WorldState writes, large-graph ScenarioRunner execution, dynamic task registration via ActionRegistry, GLOBAL-scope persistence at scale | `TileUpdateTask`, `ResourceHarvestTask`, `WorldChunkLoadTask` | Planned |

---

## Coverage by Kernel Subsystem

| Kernel Subsystem | mystery | billiards | roguelike | rhythm | sandbox |
| --- | --- | --- | --- | --- | --- |
| ScenarioRunner | full | partial | full | partial | full |
| TaskSpec | full | partial | full | partial | partial |
| ActionRegistry | full | partial | full | partial | full |
| WorldState | full | partial | full | partial | full |
| KernelClock | partial | partial | partial | full | partial |
| Signal Handshake | full | partial | planned | planned | planned |
| Save/Load Safety | partial | planned | full | planned | full |
| Condition Logic | full | planned | partial | planned | partial |
| High-Scale Mutation | partial | planned | partial | planned | full |

**Key**: `full` = primary coverage target · `partial` = incidental coverage · `planned` = intended but not yet implemented

---

## Why the Matrix Exists

Karakuri is a **Game OS**, not a collection of sample games. Its value is in the Kernel:
the ability to run any genre of game from YAML + Tasks without touching `src/core/`.

This matrix serves three purposes:

1. **Regression guard.** If a test game breaks, a Kernel API contract has changed. Fix the
   Kernel, not the test.
2. **Coverage guide.** Before writing a new test module, consult this matrix to identify which
   Kernel subsystems are under-tested. New modules should fill gaps, not duplicate existing coverage.
3. **AI agent orientation.** Future AI agents contributing to this repository must understand
   that each test module has a defined scope. Expanding a test beyond its intended coverage
   area belongs in a separate module with its own matrix row.

---

## Guidance for Future Test Modules

When adding a new test module to `src/games/`, follow these rules:

- **Target uncovered capability.** Check the subsystem coverage table above. A new module must
  have at least one `full` cell in a column where all existing modules show `partial` or `planned`.
- **Never modify `src/core/`.** The entire point of a fitness test is to prove the Kernel works
  as-is. A test that requires a Kernel change is a Kernel bug report, not a new test.
- **Stay minimal but focused.** A test module should be the smallest possible game that
  exercises its target capability. Avoid UI complexity, asset dependencies, or cross-module state.
- **Document what you verify.** Add a comment block at the top of the module's primary `.cpp`
  file and its YAML scenario listing the Kernel features under test. Update this matrix.
- **Add a row to this table** before writing any code, so the intended coverage is visible
  to reviewers and future agents before implementation begins.
