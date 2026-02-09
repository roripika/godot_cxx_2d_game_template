# Universal Game Template Walkthrough

## Overview
This project separates core logic from views.

> [!IMPORTANT]
> **Breaking Change**: The `SandboxWorld` node has been replaced by `IsometricView`. You must update your Godot scenes.

## Key Components

### 1. Core Layer (`src/core`)
- **`UniversalWorldData` (Resource)**: Stores voxel data.

### 2. View Layer (`src/views`)
- **`IsometricView` (Node: `TileMapLayer`)**: Visualizes `UniversalWorldData`.

### 3. Entity Layer (`src/entities`) [NEW]
- **`GameEntity` (Node: `CharacterBody2D`)**:
    - Base class for Characters and Enemies.
    - **Properties**: `Max Health`, `Current Health`, `Speed`.
    - **Signals**: `health_changed(new_health)`, `died`.
    - **Movement**: Controlled via `set_movement_input(Vector2)`.

## Usage Guide

### Setup World
1. Create a Scene with `IsometricView`.
2. Assign `UniversalWorldData` Resource.

### Setup Player Entity
1. Create a new Scene inheriting from `GameEntity` (or add a node of type `GameEntity`).
2. Add a `CollisionShape2D` and `Sprite2D` as children.
3. Attach a GDScript to handle input provided by the View:
    ```gdscript
    extends GameEntity

    func _physics_process(delta):
        # Example Isometric Input
        var input = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
        # Convert to Isometric direction if needed, or just pass raw input
        # For Isometric:
        # Up-Right (Screen Up) -> (1, -1) in map? This depends on your controller logic.
        # For now, pass direct vector:
        set_movement_input(input)
    ```
