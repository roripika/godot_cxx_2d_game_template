# Universal Game Template Implementation Plan

## Goal
Transform the specific "Isometric Sandbox" project into a **Modular Universal Game Template** capable of handling:
- **Views**: 2D Iso, 2D Side, 3D, FPS
- **Styles**: RPG, Action, Adventure
- **Cycles**: Roguelike, Sandbox, Story

## Architecture

### 1-5. Core, Views, Entities, Items, Cycles (Done/In-Progress)
- `UniversalWorldData`, `IsometricView`, `SideScrollingView`
- `GameEntity`, `PlayerControllerIso`, `PlayerControllerSide`
- `GameItem`, `Inventory`
- `RoguelikeGenerator`, `RoguelikeManager`

### 6. Sample & Demo System (Phase 4) [NEW]
- **Directory Structure**:
    - `godot_project/samples/`: Contains isolated demo folders.
        - `roguelike/`: Scene + Assets specific to Roguelike.
        - `platformer/`: Scene + Assets specific to SideScroller.
        - `mystery/`: Scene + Assets for Adventure.
- **Switching Mechanism**:
    - **Script**: `scripts/setup_demo.sh` (Bash)
    - **Function**: CLI tool to list available demos and update `project.godot`'s `application/run/main_scene` path to the selected demo scene.

### 7. Adventure & Mystery Layer (Phase 5)
- **`InteractionManager`**: Click/Inspect logic.
- **`DialogueSystem`**: UI/Text logic.

## Migration Steps

1-10. **Refactoring & Core Implementation** (Done)
11. **Implement Sample Demos**: 
    - Create Scene files (`.tscn`) using existing C++ nodes.
    - Organize into `samples/` folder.
12. **Implement Setup Script**: Write `setup_demo.sh`.
13. **Implement Adventure Features**: Add interaction logic for the Mystery demo.

## Verification
- **Build**: `scons target=template_debug`
- **Run**:
    - `./scripts/setup_demo.sh roguelike` -> Launches Godot with Roguelike Demo.
    - `./scripts/setup_demo.sh platformer` -> Launches Godot with Platformer Demo.
