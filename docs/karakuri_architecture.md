# Karakuri Architecture Contract v2.0

This document is a STRICT RULESET for AI development assistants working on the Karakuri Kernel. 
Violating these rules WILL break the kernel's determinism, safety, and architectural integrity.

---

## 1. SYSTEM OVERVIEW

Karakuri Game OS is structured as a STRICT multi-layered system.

- **Kernel Layer (`src/core/`)**: Pure C++ logic, state management, and orchestration.
- **Game Modules (`src/mystery/`, `src/invaders/`)**: Game-specific task implementations and data.
- **View Layer (GDScript / `.tscn`)**: UI presentation and input handling.

### DEPENDECY RULES
- **STRICT RULE**: Game Modules MUST depend on the Kernel.
- **STRICT RULE**: The View Layer MUST depend on the Kernel.
- **NEVER** allow the Kernel to depend on Game Modules or the View Layer.
- **NEVER** include `mystery/` headers inside `core/`.

---

## 2. MEMORY MODEL (WorldState)

`WorldState` is the SINGLE SOURCE OF TRUTH for all game data.

### STORAGE SCOPES
- **GLOBAL**: Persists across the entire application lifecycle.
- **SESSION**: Persists across a single playthrough/save file.
- **SCENE**: Persists only within the current scenario scene scope.

### STRICT RULES
- **NEVER** use local `godot::Dictionary` or static variables to store game state.
- **NEVER** store game logic state in Godot Nodes or Singleton Nodes.
- **ALWAYS** use `WorldState` for state persistence and retrieval.
- **ALWAYS** use the `state_changed` signal for observers to react to changes.

### Bad vs Good
❌ **Bad**
```cpp
Dictionary flags;
flags["door_open"] = true; // State is lost/unmanaged
```

⭕ **Good**
```cpp
WorldState::get_singleton()->set_bool(
    WorldState::SCOPE_SCENE,
    "door_open",
    true
); // Managed, deterministic, and observable
```

---

## 3. TIME MODEL (KernelClock)

Karakuri uses a **Deterministic Runtime Clock**. Engine frame deltas are unreliable.

### STRICT RULES
- **NEVER** use Godot's `_process(double delta)` or `_physics_process` for game logic.
- **NEVER** perform time-based calculations using raw `delta` values.
- **ALWAYS** use `KernelClock::get_singleton()->now()` to get the current deterministic time.

### Bad vs Good
❌ **Bad**
```cpp
elapsed_ += delta;
if (elapsed_ > 3.0) {
    complete();
} // Not deterministic
```

⭕ **Good**
```cpp
if (KernelClock::get_singleton()->now() >= target_time_) {
    return TaskResult::Success;
} // Deterministic and repeatable
```

---

## 4. SCENARIO EXECUTION MODEL

`ScenarioRunner` is a PURE STATE MACHINE. It orchestrates tasks but does not own them.

### STRICT RULES
- **NEVER** hold a `NodePath` or raw pointer to UI nodes in the Kernel.
- **NEVER** call functions on Godot's `SceneTree` directly for scenario flow.
- **ALWAYS** use **Signal Handshakes** for external interactions:
    - `dialogue_requested`
    - `choice_requested`
    - `transition_requested`
- **STRICT RULE**: All transitions MUST have a safety timeout (default 5 seconds). Transition tasks MUST yield until `complete_transition()` is called by the View Layer.

---

## 5. DATA MODEL (Typed Scenario IR)

Karakuri DOES NOT execute raw YAML/JSON at runtime. Data MUST be compiled into `TaskSpec`.

### STRICT RULES
- **NEVER** use dynamic property injection like `task->set("key", value)`.
- **NEVER** use `ADD_PROPERTY` for task-internal execution state.
- **ALWAYS** use `TaskSpec` for task construction and `validate_and_setup(const TaskSpec&)` for initialization.
- **STRICT RULE**: Tasks MUST treat `TaskSpec` as read-only IR.

### Bad vs Good
❌ **Bad**
```cpp
task->set("duration", 5.0); // Legacy dynamic injection
```

⭕ **Good**
```cpp
TaskSpec spec;
spec.action = "wait";
spec.payload["duration"] = 5.0;
registry->compile_task(spec); // Validated and compiled
```

---

## 6. ACTION FACTORY

Action registration MUST be type-safe.

### STRICT RULES
- **NEVER** use `ClassDB::instantiate` or string-to-class reflection for Task creation.
- **ALWAYS** use `ActionRegistry::register_action_class<T>(name)`.

### Bad vs Good
❌ **Bad**
```cpp
Object *obj = ClassDB::instantiate("WaitTask"); // Unsafe reflection
```

⭕ **Good**
```cpp
registry->register_action_class<WaitTask>("wait"); // Type-safe factory
```

---

## 7. VIEW / CORE SEPARATION

**Core is agnostic of the View.**

### STRICT RULES
- **NEVER** use `get_node()` to find UI elements from `src/core/`.
- **ALWAYS** communicate from Core to View via **signals**.
- **ALWAYS** communicate from View to Core via **method calls** on singletons.

### Bad vs Good
❌ **Bad**
```cpp
get_node("UI/Dialogue").call("show_text", text); // Hard coupling
```

⭕ **Good**
```cpp
emit_signal("dialogue_requested", speaker, text); // Decoupled
```

---

## 8. EXTENDING KARAKURI

To add new functionality to the Game OS:

### THE EXTENSION PROTOCOL
1. **CREATE** a new `Task` class inheriting from `TaskBase`.
2. **DEFINE** its `TaskSpec` structure (if applicable).
3. **IMPLEMENT** `validate_and_setup(const TaskSpec& spec)`.
4. **REGISTER** the class in `register_types.cpp` via `ActionRegistry`.

**STRICT RULE**: DO NOT modify `ScenarioRunner.cpp` to add new game-specific logic. Extend by adding Tasks.

---

## 9. FINAL WARNING

These rules are NOT suggestions. 

Violating these rules WILL break the Karakuri Kernel, corrupt the `WorldState`, and destroy determinism. 

**IF YOU ARE UNSURE: DO NOT MODIFY `src/core/`.**
Extend the system by adding new modules in `src/mystery/` or other game-specific directories. 
Respect the abstraction. Keep the Kernel pure.
