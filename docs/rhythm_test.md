# rhythm_test Design Memo v0.1

Date: 2026-03-23

`rhythm_test` is not a production rhythm game module.
It is a minimal fitness test for time / clock / timing window / deterministic scheduling on Kernel v2.0.

## Purpose

- verify `KernelClock`-driven logical progression
- verify deterministic fake tap judgment
- verify `clear` / `fail` / `continue` branch behavior without real input

## Initial scope

- 1 lane only
- fixed chart only
- 3-stage judge: `perfect`, `good`, `miss`
- fake tap input only
- no long note, no simultaneous input, no production UI

## Initial tasks

- `setup_rhythm_round`
- `load_fake_tap`
- `advance_rhythm_clock`
- `judge_rhythm_note`
- `resolve_rhythm_progress`
- `evaluate_rhythm_round`

## WorldState focus

Primary keys for the initial version:

- `rhythm_test:chart:index`
- `rhythm_test:chart:note_count`
- `rhythm_test:judge:last_result`
- `rhythm_test:judge:perfect_count`
- `rhythm_test:judge:good_count`
- `rhythm_test:judge:miss_count`
- `rhythm_test:tap:last_time_ms`
- `rhythm_test:round:result`
- `rhythm_test:round:status`

The module may use small helper keys under the same namespace for chart/timing config, but rendering state and audio state are intentionally excluded.

## Current status

- minimal skeleton implemented
- validator fixtures added
- runtime verification is not yet recorded in this document

## Out of scope

- real input integration
- audio sync quality validation
- long note / chord support
- presentation polish