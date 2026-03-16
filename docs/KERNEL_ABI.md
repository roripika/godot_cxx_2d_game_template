# Karakuri Kernel ABI Contract v2.0

This document defines the strict binary and logic interface for the Karakuri Kernel. All AI-generated tasks MUST comply with this ABI.

## 1. TASK EXECUTION MODEL
The Kernel uses a parameterless `execute()` model driven by the `KernelClock`.

### STRICT RULES
- **NEVER** change the `execute()` signature to accept parameters (e.g., `execute(double delta)` is DEPRECATED).
- **NEVER** perform time-based state updates inside `execute()` using raw deltas.
- **ALWAYS** return a `TaskResult` (`Success`, `Failed`, or `Yielded`).

❌ **Bad (Legacy)**
```cpp
TaskResult execute(double delta) {
    elapsed += delta; // VIOLATION: Direct delta usage
    return Success;
}
```

⭕ **Good (v2.0)**
```cpp
TaskResult execute() {
    if (KernelClock::get_singleton()->now() >= target_time_) {
        return TaskResult::Success;
    }
    return TaskResult::Yielded;
}
```

## 2. TYPED SCENARIO IR (TaskSpec)
Karakuri DOES NOT use dynamic property injection. AI MUST use intermediate Representation (IR).

### STRICT RULES
- **ALWAYS** use `TaskSpec` to pass data to Tasks.
- **ALWAYS** use `validate_and_setup(const TaskSpec&)` for initialization.
- **NEVER** implement dynamic setters (e.g., `set_duration()`) for Task properties.
- **NEVER** use `ADD_PROPERTY` for runtime execution state.

❌ **Bad**
```cpp
task->set("duration", 5.0); // VIOLATION: Dynamic property injection
```

⭕ **Good**
```cpp
TaskSpec spec;
spec.action = "wait";
spec.payload["duration"] = 5.0;
registry->compile_task(spec); // Logic is validated during compilation
```

## 3. FAIL-FAST VALIDATION
The Kernel enforces strict load-time validation.

- **STRICT RULE**: `validate_and_setup` MUST return `godot::OK` only if the `TaskSpec` is 100% valid.
- **ALWAYS** return an error code (e.g., `ERR_INVALID_DATA`) immediately if keys are missing or types are incorrect.

## 4. ACTION FACTORY CONTRACT
All actions MUST be registered via the type-safe factory.

- **NEVER** use `ClassDB::instantiate` or string-based reflection to create Tasks.
- **ALWAYS** use `ActionRegistry::register_action_class<T>("action_name")`.
- **STRICT RULE**: Once an action name is established (e.g., "dialogue"), it MUST NOT be changed.

❌ **Bad**
```cpp
Object *obj = ClassDB::instantiate("WaitTask"); // VIOLATION: Weak typing
```

⭕ **Good**
```cpp
registry->register_action_class<WaitTask>("wait"); // Strong typing
```

## 5. ABI STABILITY
- **MUST NOT** rename keys in `TaskSpec.payload` once used in production scenarios.
- **MUST NOT** change the semantic meaning of `TaskResult` codes.

If these contracts are breached, the Kernel compiled scenario and existing save data WILL become unreadable.
Protect the ABI at all costs.
