# AI Contributing Guidelines for Karakuri Kernel

This is the operative manual for AI assistants (Cursor, Copilot, Antigravity) contributing to this repository. You MUST verify your actions against this list before every commit.

## 1. PREREQUISITE KNOWLEDGE
Before making ANY change, you MUST have read and understood:
1. `ARCHITECTURE.md` (Layering and Dependencies)
2. `KERNEL_ABI.md` (Interface and Signing)
3. `WORLD_STATE.md` (State and Memory)

## 2. THE "TASK-FIRST" EXTENSION RULE
Karakuri Kernel is designed to be immutable in its core but infinitely extendable via Tasks.

- **STRICT RULE**: If a feature can be implemented as a `Task` inside a module, you MUST NOT modify `src/core/`.
- **STRICT RULE**: ScenarioRunner MUST NOT provide extension hooks for gameplay logic. NEVER register ad-hoc actions via callbacks, lambdas, or ActionHandler-like mechanisms. ALL executable scenario behavior MUST be implemented as Task classes and registered only through ActionRegistry.
- **NEVER** modify `ScenarioRunner.cpp` to add branching logic or game-specific behavior.

## 3. KERNEL MODIFICATION PROTOCOL
If you MUST modify `src/core/`:
- **ALWAYS** explain the architectural necessity.
- **NEVER** introduce dependencies on `src/mystery/` or any other game module.
- **MUST** ensure the `KernelClock` determinism is maintained.
- **STRICT RULE**: Every new core feature MUST be documented in `KERNEL_ABI.md`.

## 4. UI SEPARATION ENFORCEMENT
Karakuri uses a "Core -> Signal -> UI" model.
- **NEVER** add `NodePath` properties to a Task to reference a specific UI node.
- **ALWAYS** emit a custom signal if a Task needs a visual representation.
- **NEVER** perform logic inside GDScript that affects the outcome of a scenario. View is for display only.

## 5. PRE-COMMIT CHECKLIST (GREP AUDIT)
AI assistants MUST run these checks before finalizing changes:

| Search Pattern | Prohibited Result | Rationale |
| :--- | :--- | :--- |
| `task->set(` | ANY | Violates ABI v2.0 Immutable Spec |
| `ClassDB::instantiate(` | Inside `src/core/` | Violates Strong-Typed Factory principle |
| `elapsed_ += delta` | ANY | Violates Deterministic Clock principle |
| `#include "mystery/` | Inside `src/core/` | Violates Layer Isolation |
| `get_node(` | Inside `src/core/` | Violates View/Core separation |
| `register_action(` | ANY | Violates Unified Task Extension (ABI v2.0) |
| `ActionHandler` | ANY | Violates Unified Task Extension (ABI v2.0) |
| `InlineHandlerTask` | ANY | Violates Unified Task Extension (ABI v2.0) |
| `std::function` | Inside `src/core` | Prohibited Pseudo-escape hatch |
| `Variant payload` | ANY | Prohibited generic payload (Use `TaskSpec`) |

## 6. CODING STYLE
- **STRICT RULE**: Use the `karakuri::` namespace for all core classes.
- **STRICT RULE**: Use `godot::OK` and standard error codes for validation.
- **ALWAYS** use `ERR_FAIL_NULL` macros for singleton access safety.

## 7. AI-SAFE EXTENSION PROTOCOL
AI assistants MUST follow these steps precisely when adding new functionality:
1. **Inherit**: Create a new class inheriting from `karakuri::TaskBase`.
2. **Modularize**: Place the class in the appropriate `karakuri::games::<module_name>` namespace.
3. **Validate**: Implement `validate_and_setup(const TaskSpec& spec)` to verify all payload parameters at load-time.
4. **Isolate**: Ensure all execution state is contained within the Task instance. NEVER use global/static variables.
5. **Decouple**: Use `ScenarioRunner` API (e.g., `emit_signal`, `set_waiting_for_dialogue`) for interactions. NEVER use `get_node()` or `get_tree()`.
6. **Register**: Add the task to `ActionRegistry` via `register_action_class<T>()` in the module's bootstrap/`register_types`.
7. **Integrate**: Reference the task by its registered name in the YAML scenario.

## 8. AI IMPLEMENTATION CHECKLIST
Before finalizing any PR or commit, the AI MUST verify:
- [ ] **No Direct Access**: Zero instances of `get_node()`, `get_tree()`, or `NodePath` in core/module logic.
- [ ] **WorldState Only**: All persistent game state is stored in `WorldState` with correct scope.
- [ ] **Typed IR**: Task initialization is handled exclusively via `TaskSpec`. No `task->set()`.
- [ ] **Namespace Purity**: Game-specific classes are inside `karakuri::games::<name>`.
- [ ] **Signal Ownership**: Visual/UI signals are owned by `ScenarioRunner`, not individual Tasks.
- [ ] **No Reflection**: `ActionRegistry` is used for task creation; no custom factory or `ClassDB` hacks.
- [ ] **Core Untouched**: No modifications made to `src/core/` unless strictly required and architecturally approved.
- [ ] **Clock Determinism**: No frame-rate dependent logic (no `delta` usage).

## 9. MYSTERY_TEST: KERNEL FITNESS TEST

`src/games/mystery_test/` is **not a sample game**. It is the primary Kernel Fitness Test for v2.0.

Before submitting any change to the repository, AI assistants MUST understand what `mystery_test` is verifying:

- **Narrative control flow**: `ScenarioRunner` state-machine advances through YAML scenes correctly.
- **WorldState mutation discipline**: Evidence flags are written to SESSION scope and never leak to GLOBAL.
- **Compound condition evaluation**: `CheckConditionTask` evaluates `all_of` / `any_of` against live `WorldState`.
- **Deterministic wait / timeout safety**: `WaitForSignalTask` must not deadlock; `KernelClock` drives the timeout.
- **Action registration contract**: Every task is registered via `ActionRegistry`; no reflection or inline handlers.
- **Task-level game extension model**: The entire module runs without modifying `src/core/`.

| mystery_test capability | Kernel feature verified |
| --- | --- |
| `show_dialogue` | Signal handshake / waiting state |
| `discover_evidence` | `WorldState` mutation (SESSION scope) |
| `check_condition` | Condition engine (all_of / any_of) |
| `wait_for_signal` | `KernelClock` timeout safety |
| `end_game` | External signal dispatch |

If `mystery_test` regresses, a **Kernel API contract has been broken**. Fix the Kernel, do not patch the test.

## 10. FINAL WARNING
Violating the Architecture Contract WILL break the Game OS. If you are unsure about a design decision, HALT and request clarification from the human lead architect.

**Protect the Kernel. Preserve Determinism. Stay Decoupled.**
