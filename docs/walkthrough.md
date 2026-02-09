# Universal Game Template Walkthrough

## Overview
This project separates core logic from views, allowing for both Isometric RPGs and Side-Scrolling Platformers using the same data.

## Key Components

### 1. View Layer (`src/views`)
- **`IsometricView`**: Renders Top-Down/Iso view.
- **`SideScrollingView`**: Renders Side-View (Platformer).

### 2. Entity Layer (`src/entities`)
- **`PlayerControllerIso`**: Isometric controls.
- **`PlayerControllerSide`**: Platformer controls.

### 3. Loop & Cycle (`src/core/cycles`)
- **`RoguelikeGenerator`**: Algorithms for generating map data.
- **`RoguelikeManager`**: A Node that orchestrates the level generation and player spawning.

## Sample System

The project now includes a Demo System to easily switch between different game templates.

### Layout
- **`samples/roguelike/`**: Roguelike Demos.
- **`samples/platformer/`**: Side-scroller Demos.
- **`samples/mystery/`**: Adventure Game Demos.

### Switching Demos
Use the provided script to switch the project's startup scene:

```bash
# Switch to Roguelike Demo
./scripts/setup_demo.sh roguelike

# Switch to Platformer Demo
./scripts/setup_demo.sh platformer

# Switch to Mystery Demo (coming soon)
./scripts/setup_demo.sh mystery
```

## Usage Guide: creating a Roguelike Demo

To create a playable demo scene manually:

1. Create a new Scene with Root Node: **`PlayerControllerIso`**.
2. Add a `Sprite2D` and `CollisionShape2D` to it.
3. Save as `res://player.tscn`.
4. Create a new Level Scene with `RoguelikeManager` and `IsometricView`.
5. Link them in the Inspector and run.
