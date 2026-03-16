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

## 7. FINAL WARNING
Violating the Architecture Contract WILL break the Game OS. If you are unsure about a design decision, HALT and request clarification from the human lead architect.

**Protect the Kernel. Preserve Determinism. Stay Decoupled.**
