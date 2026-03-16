# KernelClock Contract v2.0

## 1. PURPOSE
The `KernelClock` is the single source of truth for time in the Karakuri Kernel. It ensures determinism by decoupling game logic from the engine's variable frame rate.

## 2. MONOTONICITY CONTRACT
`KernelClock` is a monotonic clock. It MUST only move forward during scenario execution.

### STRICT RULES
- **STRICT RULE**: `KernelClock` MUST NOT be reset during scenario execution.
- **STRICT RULE**: Resetting the clock (`reset()` or `set_time(0.0)`) is PERMITTED ONLY when:
    - A new scenario is loaded.
    - A new session starts.
    - A saved game is loaded.
- **NEVER** use `_process(delta)` or `_physics_process(delta)` for time-based logic in Tasks or Kernel services.
- **NEVER** reset the clock for "Rewind" or "Replay" features without implementing a proper state-snapshot restoration (See `ARCHITECTURE.md`).

## 3. USAGE RULES
- **ALWAYS** use `KernelClock::get_singleton()->now()` to check for completion or timing.
- **ALWAYS** calculate target times at the start of an action (e.g., `target_time = now + duration`).

### Bad vs Good
❌ **Bad**
```cpp
elapsed_ += delta; 
if (elapsed_ > 5.0) // VIOLATION: Non-deterministic and breaks on frame skips
```

⭕ **Good**
```cpp
if (KernelClock::get_singleton()->now() >= target_time_) // Deterministic comparison
```

## 4. DETERMINISM WARNING
Resetting the clock mid-execution or using raw deltas WILL break the determinism required for replaying scenarios and synchronizing AI observers. Violating these rules is a CRITICAL ARCHITECTURAL BREACH.
