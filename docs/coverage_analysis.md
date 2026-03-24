# Kernel v2.0 Coverage Analysis

Date: 2026-03-23
Status: Accepted as planning reference

This document captures the current coverage situation and the next-module decision basis.
It is a preparation memo for the next phase, not an implementation spec.

---

## 1. Purpose

- Keep a stable snapshot of current Kernel v2.0 coverage.
- Clarify which area is still weak.
- Explain why the next module candidate is `rhythm_test`.
- Record why `roguelike_test` should not be expanded now.

---

## 2. Role split of active fitness modules

### `mystery_test`

- Focus: static scenario graph, condition evaluation, timeout handling.
- Main proof points:
  - Scenario graph progression via ScenarioRunner.
  - Condition checks with WorldState-based logic (`all_of` / `any_of`).
  - Wait/timeout control for async-like flow.

### `billiards_test`

- Focus: event-driven bridge between external runtime events and Kernel tasks.
- Main proof points:
  - Event wait/receive path and re-entry to scenario progression.
  - Event-history based post-evaluation path.

### `roguelike_test`

- Focus: turn/entity/grid progression under deterministic fake command input.
- Main proof points:
  - Turn loop (`player -> enemy -> resolve`).
  - Entity state mutation (position/HP) in WorldState SESSION scope.
  - Grid movement and occupancy/collision-safe behavior.
  - 3-way branch (`clear` / `fail` / `continue`) runtime confirmation.

### `rhythm_test`

- Focus: time-driven strictness, KernelClock precision, and timing window judgment.
- Main proof points:
  - `KernelClock`-based frame-independent scheduling.
  - Deterministic hit judgment (Perfect/Good/Miss) against fixed chart.
  - Clear/Fail/Continue branching under time-sensitive conditions.

---

## 3. Coverage already secured

Current modules now cover:

- Scenario graph execution stability
- Conditional branching correctness
- Event-driven progression bridge
- Turn-driven progression with entity mutation
- Time-driven strictly deterministic scheduling (`KernelClock`)
- Deterministic timing window judgment (rhythm engine)
- Deterministic smoke execution and regression paths

Runtime evidence exists for:

- `roguelike_test` `clear` / `fail` / `continue`
- `rhythm_test` `clear` / `fail` / `continue`
- Debug observer visibility of position/HP/clock/result transitions

---

## 4. Coverage still weak

The main weak areas remain:

- Real input bridge (currently mocked/fake-driven)
- Multi-threaded task execution (Core is currently single-threaded)
- Massive parallel stress (kept out intentionally for now)

---

`rhythm_test` was selected and implemented because:

1. It directly proved time-driven behavior not covered by previous modules.
2. It exercised deterministic scheduling pressure via beat/timing windows.
3. It completed the "quadrant" of fitness tests:
   - `mystery_test`: condition/graph-driven
   - `billiards_test`: event-driven
   - `roguelike_test`: state/turn-driven
   - `rhythm_test`: time/clock-driven

---

## 6. Why `roguelike_test` is not expanded now

`roguelike_test` currently meets its fitness-test objective.

It should not be expanded now because:

- The current checkpoint already proves turn/entity/grid viability.
- Real input, multi-enemy, inventory, FOV, and procgen are scope expansion, not missing proof for current objective.
- Vertical expansion in one module would delay horizontal proof of untested Kernel coverage.

---

## 7. Real input note (not now)

Real input is recognized as an important future phase, but implementation is intentionally deferred.

If entered later, minimum entry conditions should include:

- Input buffering service or equivalent queue semantics.
- A wait-style task that blocks progression until a valid command is available.
- Strict rule that only finalized logical command results are persisted in WorldState.
- Explicit sequence policy for input wait vs other entity progression.

---

## 8. Next minimal step

- Freeze Phase 2 implementation.
- Focus on design memos for next phases (Real Input, Stress Strategy).
- Define entry conditions for Phase 3 (Template / Generator).
