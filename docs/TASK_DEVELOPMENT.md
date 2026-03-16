# Task Development Guide v2.0

This guide defines how AI assistants MUST implement new `Task` classes in Karakuri Kernel v2.0.

## 1. THE TASK INTERFACE
All tasks MUST inherit from `TaskBase` and implement the following contract.

### `validate_and_setup(const TaskSpec& spec)`
- **PURPOSE**: Load-time validation and IR processing.
- **STRICT RULE**: MUST perform Fail-Fast validation. Return error codes immediately if data is invalid.
- **NEVER** modify `WorldState` during this phase.

### `execute()`
- **PURPOSE**: Runtime execution logic called every step.
- **STRICT RULE**: MUST be parameterless.
- **STRICT RULE**: MUST rely on `KernelClock` for time.
- **RETURN VALUES**:
    - `TaskResult::Success`: Task completed successfully.
    - `TaskResult::Yielded`: Task is still running (wait for next frame).
    - `TaskResult::Failed`: Task encountered a fatal error.

## 2. STATE MANAGEMENT
- **STRICT RULE**: Tasks MUST NOT own persistent game state. Use `WorldState`.
- **STRICT RULE**: Transient execution state (e.g., `target_time_`) MUST be private members.

## 3. SIGNAL HANDSHAKES
If a Task interacts with the View (UI), it MUST use signals.

- **ALWAYS** emit logic-centric signals (e.g., `choice_requested`).
- **NEVER** call UI nodes directly via `get_node()`.
- **STRICT RULE**: If the View needs to confirm completion, the Task MUST `Yield` until a callback (via `ScenarioRunner`) occurs.

## 4. EXAMPLE RE-IMPLEMENTATION (WaitTask)
```cpp
TaskResult WaitTask::execute() {
    auto *clock = KernelClock::get_singleton();
    if (clock->now() >= target_time_) {
        return TaskResult::Success;
    }
    return TaskResult::Yielded;
}
```

## 5. ANTI-PATTERNS
- **NEVER** use `TaskResult::Success` for an action that hasn't visually finished if it's meant to be blocking.
- **NEVER** perform heavy I/O or blocking operations in `execute()`.
- **STRICT RULE**: DO NOT assume `_process` order. Only rely on the `execute()` call from `ScenarioRunner`.

Verify your task implementation against `KERNEL_ABI.md` before finalizing.
