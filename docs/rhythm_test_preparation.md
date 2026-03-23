# rhythm_test Preparation Memo

Date: 2026-03-23
Status: Pre-implementation preparation

This document is a start memo for the next module.
It does not define implementation details and does not add runtime code.

---

## 1. Why next is `rhythm_test`

`rhythm_test` is the next candidate because current active modules are strong in graph/event/turn coverage, while time-driven strictness remains weak.

The next incremental proof should therefore target clock/timing behavior with minimal gameplay scope.

---

## 2. New coverage to capture with `rhythm_test`

Primary new coverage goals:

- Time/clock-driven progression validation
- Timing window judgment consistency (early/on-time/late boundaries)
- Deterministic scheduling under repeated beat processing

Secondary goal:

- Confirm that time-driven rules can remain data/task driven without expanding core runtime surface.

---

## 3. Weak points not fully covered by existing three modules

Current modules do not fully verify:

- Continuous timing precision under high-frequency schedule checks
- Judgment behavior sensitive to small timing deltas
- Time-driven pass/fail branching consistency

---

## 4. Minimalization policy for `rhythm_test`

Keep scope intentionally narrow.

- Do not build a complete rhythm game.
- Do not build production UI.
- Keep focus on time-driven fitness behavior only.
- Prefer deterministic fake tap/timeline inputs for initial proofs.

---

## 5. Why implementation is not started yet

Implementation is intentionally deferred in this step because:

- This phase is for coverage alignment and start conditions.
- Design constraints must be frozen before introducing new tasks/scenarios.
- Avoid premature expansion of runtime/test complexity.

---

## 6. Design topics to resolve next (before coding)

Resolve these topics first:

1. Logical timeline representation for beat schedule in scenario/task terms.
2. Deterministic fake tap input model and replay reproducibility rule.
3. Timing window contract (units, tolerance, and boundary handling).
4. Pass/fail/continue-like progression rule for rhythm fitness outcomes.
5. Minimal WorldState contract for rhythm (logical state only).

---

## 7. Boundary reminder

Out of scope for initial `rhythm_test` phase:

- Real input integration
- Rich visual/audio presentation
- Advanced scoring UX
- Broad gameplay feature expansion

The target is Kernel coverage proof, not product-level rhythm gameplay.
