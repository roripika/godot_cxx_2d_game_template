# Karakuri Architecture Contract v2.0

## 1. SYSTEM OVERVIEW
Karakuri is a **Game OS Kernel** designed for AI-driven game generation. It is NOT a standard Godot template. It enforces strict separation between logic and presentation.

### Layer Definitions
- **Kernel Layer (`src/core/`)**: The "Brain". Manages state, time, and execution flow. Pure C++.
- **Game Modules (`src/mystery/`, `src/invaders/`)**: The "Content". Implements specific Tasks and data structures using Kernel APIs.
- **View Layer (GDScript / `.tscn`)**: The "Body". Strictly restricted to UI rendering and input. NO game logic.

## 2. STRICT DEPENDENCY RULES
- **STRICT RULE**: Game Modules MUST depend on `src/core/`.
- **STRICT RULE**: View Layer MUST depend on `src/core/` via signals and singleton calls.
- **NEVER** include module-specific headers (e.g., `#include "mystery/..."`) inside `src/core/`.
- **NEVER** reference UI nodes or GDScript classes directly from `src/core/`.

❌ **Bad**
```cpp
// inside src/core/scenario/scenario_runner.cpp
#include "../../mystery/tasks/zoom_camera_task.h" // VIOLATION: Core depends on Module
```

⭕ **Good**
```cpp
// src/core only interacts with TaskBase
Ref<TaskBase> task = registry->compile_task(spec);
```

## 3. CORE RESPONSIBILITIES
- **ScenarioRunner**: A pure state machine. MUST NOT manage UI state. MUST NOT handle `SceneTree` operations directly.
- **WorldState**: The ONLY place to store game data.
- **KernelClock**: The ONLY source of time.
- **ActionRegistry**: The ONLY factory for Tasks.

## 4. EXTENDING THE SYSTEM
AI MUST follow the "Extension by Task" model.

- **NEVER** modify `ScenarioRunner.cpp` to add game-specific logic.
- **ALWAYS** implement new logic as a new `Task` class in a Game Module.
- **ALWAYS** register new actions via `ActionRegistry::register_action_class<T>()`.

❌ **Bad**
```cpp
// inside scenario_runner.cpp
if (action == "show_mystery_ui") {
    // VIOLATION: Hardcoding module logic into the kernel
}
```

⭕ **Good**
```cpp
// inside register_types.cpp
registry->register_action_class<ShowMysteryUITask>("show_mystery_ui");
```

## 5. KERNEL MODIFICATION POLICY
- **STRICT RULE**: Modifications to `src/core/` are PERMITTED ONLY for improving the Kernel itself.
- **NEVER** add domain-specific features to the Kernel.
- **ALWAYS** verify that Kernel changes do not break existing Game Modules.

Violating these architectural boundaries WILL result in circular dependencies and fatal kernel instability.
Do not attempt to bypass these layers. Respect the abstraction.
