# WorldState Specification v2.0

`WorldState` is the central, deterministic memory arena of the Karakuri Kernel. It is the SINGLE SOURCE OF TRUTH for all game-related state.

## 1. MEMORY SCOPES & LIFECYCLE
AI MUST select the appropriate scope for every variable.

- **GLOBAL**: Persists until the application closes. Use for settings, trophies, and cross-save meta-progression.
- **SESSION**: Persists within a single "New Game". Saved and Loaded from disk. Use for inventory, quest progress, and character stats.
- **SCENE**: Volatile memory. Cleared AUTOMATICALLY during every scene transition. Use for temporary sequence flags (e.g., `door_just_opened`).

### STRICT RULES
- **NEVER** use `static` variables in C++ or `Dictionary` variables in Nodes to store persistable game state.
- **NEVER** store UI-only state (e.g., window position) in `WorldState`.
- **STRICT RULE**: Scene scope data MUST be accessed with the assumption it will be wiped after a `transition_requested` signal completes.

## 2. NAMESPACE CONVENTION
To prevent collisions between Kernel, Mystery Modules, and AI additions, use namespacing.

- **Pattern**: `<module>:<entity>:<property>`
- **STRICT RULE**: AI MUST NOT use keys without a namespace prefix.

❌ **Bad**
```cpp
WorldState::get_singleton()->set_bool(SCOPE_SESSION, "is_open", true);
```

⭕ **Good**
```cpp
WorldState::get_singleton()->set_bool(SCOPE_SESSION, "mystery:gate_01:is_open", true);
```

## 3. THE EVENT BUS
`WorldState` acts as the primary event bus for the Game OS.

- **`state_changed(scope, key, old_value, new_value)`**: Emitted every time a value is modified. Use this for reactive UI.
- **`scope_cleared(scope)`**: Emitted when a scope is wiped. Use this for cleanup operations.

## 4. PERSISTENCE BOUNDARIES
- **NEVER** attempt to manually write to file system from a Task. `WorldState` handles the `save_game` / `load_game` serialization automatically.
- **ALWAYS** use the type-specific methods (e.g., `set_int`, `get_bool`) to ensure data integrity.

## 5. ANTI-PATTERNS
- **NEVER** use `WorldState` as a messaging system for frame-by-frame data (e.g., player position). Use signals or direct node communication for high-frequency data.
- **MUST NOT** exceed memory limits by storing large binary blobs in `WorldState`. It is for state, not assets.

Failure to follow the scope rules WILL lead to "Stale State Bugs" where flags from previous scenes interfere with new logic.
Respect the Lifecycle. Use Namespaces.
