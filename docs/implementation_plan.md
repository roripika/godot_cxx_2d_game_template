# Universal Game Template Implementation Plan

## Goal
Transform the specific "Isometric Sandbox" project into a **Modular Universal Game Template** capable of handling:
- **Views**: 2D Iso, 2D Side, 3D, FPS
- **Styles**: RPG, Action, Adventure
- **Cycles**: Roguelike, Sandbox, Story

## Architecture

### 1. Data Layer (Core)
The "Truth" of the game world. Pure logic/data, no rendering dependencies.

- **`UniversalWorldData` (extends `Resource`)**
    - **Header**: `src/core/universal_world_data.h`
    - **Data**: `std::vector<int> m_voxels` (stored as flattened 3D array or chunk map).
    - **API**:
        - `set_voxel(Vector3i p_pos, int p_id)`
        - `get_voxel(Vector3i p_pos)`
        - `save_to_disk(String path)`
        - `load_from_disk(String path)`
    - **Signals**: `changed(Vector3i pos)`

- **`GameCycleManager` (extends `Node`)**
    - **Header**: `src/core/game_cycle_manager.h`
    - **Responsibility**: Manages the game flow state (Menu -> Generating -> Playing -> Paused -> Result).
    - **Modes**: Enum `CycleMode { SANDBOX, ROGUELIKE, STORY }`.

### 2. View Layer (Adapters)
Visualizes the Data Layer. Inherits from Godot Nodes (`TileMapLayer`, `Node3D`, etc).

- **`IsometricView` (extends `TileMapLayer`)**
    - **Header**: `src/views/isometric_view.h`
    - **Responsibility**: Listens to `UniversalWorldData`. Renders a specific Z-slice or "top-down" projection of the 3D data as isometric tiles.
    - **Input**: Converts mouse clicks on ISO grid to `Vector3i` world commands.

- **`SideScrollingView` (extends `TileMapLayer`)**
    - **Header**: `src/views/side_scrolling_view.h`
    - **Responsibility**: Renders a vertical slice (X-Y plane) of the 3D data.
    - **Physics**: May generate collision shapes for platforming.

### 3. Entity Layer [NEW]
- **`GameEntity` (extends `CharacterBody2D/3D` via wrapper or composition)**
    - **Header**: `src/entities/game_entity.h`
    - **Common Stats**: HP, MaxHP, Stamina, MoveSpeed.
    - **Controllers**:
        - `PlayerController` (abstract base)
        - `PlayerControllerIso` (inherits `PlayerController`)
        - `PlayerControllerSide` (inherits `PlayerController`)

## Migration Steps

1.  **Refactor Directory**: Move existing `sandbox_world` logic. (Done)
2.  **Create `UniversalWorldData`**: Port storage logic. (Done)
3.  **Create `IsometricView`**: Port rendering logic. (Done)
4.  **Implement `GameEntity`**: Create base class for player/enemies. (Next)
5.  **Implement Stats**: Add HP/Stamina logic to `GameEntity`. (Next)

## Verification
- **Build**: `scons target=template_debug`
- **Run**:
    - Create a Scene with `IsometricView` + `UniversalWorldData`.
    - View should function exactly as the previous `SandboxWorld`.
