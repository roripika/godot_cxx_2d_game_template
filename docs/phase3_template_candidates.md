# Phase 3 Template Candidates

Phase 3 では、既存の fitness test modules をそのまま再利用し、
AI が安全に選択・拡張できる **テンプレート候補** として整理する。

この段階の目的は、テンプレート数を増やすことではなく、
**少数の強いテンプレートを明確な検証軸ごとに整備すること**である。

---

## Template Candidate Comparison

| Template Candidate | Source Module | Primary Validation Axis | Suitable Use Cases | What AI Primarily Writes | Complexity | Phase 3 Priority |
|---|---|---|---|---|---|---|
| **Branching Template** | `mystery_test` | condition branching, flag management, timeout, static scenario progression | novel scenes, conversations, investigations, quest flow, event-driven scene transitions | conditions, branches, flags, scene transitions | Low | High |
| **Event-Driven Template** | `billiards_test` | external event waiting, re-entry, event-driven branching | signal integration, physics callbacks, external trigger response | event names, waiting steps, event recording, result evaluation | Medium | Medium |
| **Turn / Grid Template** | `roguelike_test` | turn progression, entity state, grid movement, occupancy/collision | SRPG, dungeon progression, board movement, turn-based battle | actor state, coordinates, move/attack rules, turn evaluation | Medium | High |
| **Time / Clock Template** | `rhythm_test` | KernelClock, time progression, timing window, deterministic scheduling | rhythm-like judgement, timed events, schedule-based progression, time-window evaluation | clock progression, tap/input timing, judge logic, progress evaluation | Medium–High | High |

---

## Summary of Each Template

### 1. Branching Template (`mystery_test`)
The most basic template.  
Best suited for scenario-heavy flows where progression is controlled by conditions and branching.

**Good for:**
- conversation flow
- quest progression
- investigation sequences
- simple event-based games

**Notes:**
- easy to reuse
- should remain focused on condition-heavy / scene-heavy use cases
- avoid turning it into an overly generic “do anything” template

---

### 2. Event-Driven Template (`billiards_test`)
Template for scenarios that must wait for and react to external events.

**Good for:**
- physics or collision callbacks
- signal-based progression
- systems driven by external triggers

**Notes:**
- valuable as the canonical external-event template
- should be used only when real event re-entry is necessary
- easier to misuse than other templates, so usage guidance must be explicit

---

### 3. Turn / Grid Template (`roguelike_test`)
Template for stateful turn-based systems with actor positions and simple occupancy rules.

**Good for:**
- turn-based battles
- tile/grid movement
- tactics / board systems
- lightweight dungeon progression

**Notes:**
- likely one of the main Phase 3 templates
- should stay anchored to the minimal contract:
  - 1 player
  - 1 enemy
  - 1 goal
- avoid expanding into inventory, multi-enemy generalization, or full roguelike systems at this stage

---

### 4. Time / Clock Template (`rhythm_test`)
Template for systems where progression depends on absolute time, scheduling, and timing windows.

**Good for:**
- timing-window evaluation
- scheduled progression
- time-driven minigames
- rhythm-like judge systems

**Notes:**
- should be described as a **time-driven template**, not merely a rhythm-game template
- valuable because it validates `KernelClock` usage directly
- should remain focused on deterministic timing rather than audiovisual polish

---

## Recommended Phase 3 Ordering

### Primary templates for early Phase 3
1. **Branching Template** (`mystery_test`)
2. **Turn / Grid Template** (`roguelike_test`)
3. **Time / Clock Template** (`rhythm_test`)

These three cover the most reusable axes:

- branching
- stateful progression
- time-driven progression

### Template to formalize slightly later
4. **Event-Driven Template** (`billiards_test`)

This template is still important, but it is more specialized and may be less frequently selected as an initial entry point.

---

## Why Start with Fewer Templates

At this stage, the goal is not to maximize template count.  
The goal is to establish a **small number of reliable, easy-to-select, hard-to-misuse templates**.

Too many similar templates create risks:

- overlapping responsibilities
- unclear selection criteria
- validator rule fragmentation
- increased prompt/context noise for AI agents

Therefore, Phase 3 should prefer:

- **fewer templates**
- **clear validation axes**
- **strong contracts**
- **repeatable runtime verification patterns**

---

## Suggested Direction

Treat the current four modules as the natural template family covering these four axes:

- **Condition / Branching**
- **External Event / Re-entry**
- **Turn / Entity / Grid**
- **Time / Clock**

However, prioritize formalization in this order:

1. Branching
2. Turn / Grid
3. Time / Clock
4. Event-Driven

This keeps the template system compact while maximizing practical reuse and minimizing AI confusion.
