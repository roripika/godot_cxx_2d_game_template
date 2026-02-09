# Tasks: Universal Game Template

## Project Maintenance
- [x] Create/Move Agent Rules to `.agent/AGENTS.md`

## Phase 1: Core Architecture (Logic/View Separation)
- [x] **Refactor Project Structure**
    - [x] Create `src/core`, `src/views`, `src/entities` directories
    - [x] Update `SConstruct` to compile sources from new directories
- [x] **Core Data Layer**
    - [x] Implement `UniversalWorldData` (Resource)
        - [x] 3D Voxel storage (flat vector or map)
        - [x] Signal emission on data change (`voxel_changed`)
    - [ ] Implement `GameCycleManager` (Node)
        - [ ] State Machine for Cycle (Roguelike/Story/Sandbox)
- [x] **View Layer Adapters**
    - [x] Implement `IsometricView` (inherits `TileMapLayer`)
        - [x] Connect to `UniversalWorldData`
        - [x] Render 2D Isometric tiles based on 3D data (z-slicing or projection)
    - [ ] **[NEW]** Implement `SideScrollingView` (inherits `TileMapLayer`)
        - [ ] Render Side view (x, y) from 3D data (z=depth)

## Phase 2: Game Style & Entities
- [x] **Entity System**
    - [x] Implement `GameEntity` (CharacterBody2D/3D wrapper)
        - [x] Stats logic (HP, Speed)
        - [x] Movement input Interface
    - [ ] Stats System (HP, Stamina - RPG/Action common) (Refined: Included in GameEntity basic implementation)
- [ ] **Player Controllers**
    - [ ] `PlayerControllerIso` (Isometric movement)
    - [ ] `PlayerControllerSide` (Platformer movement)
    - [ ] `PlayerControllerFPS` (First-person movement)

## Phase 3: Game Cycles
- [ ] **Cycle Implementation**
    - [ ] `RoguelikeSession`: Dungeon Generation -> Play -> Result
    - [ ] `SandboxSession`: Infinite generating & Saving

## Legacy Tasks (Completed)
- [x] Initial Isometric Sandbox Setup (Godot 4.3 + GDExtension)
- [x] Environment Setup (macOS M2)
