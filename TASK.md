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
    - [x] **[NEW]** Implement `SideScrollingView` (inherits `TileMapLayer`)
        - [x] Connect to `UniversalWorldData`
        - [x] Render Side view (x, y) from 3D data (z=depth)

## Phase 2: Game Style & Entities
- [x] **Entity System**
    - [x] Implement `GameEntity` (CharacterBody2D/3D wrapper)
        - [x] Stats logic (HP, Speed)
        - [x] Movement input Interface
- [x] **Player Controllers**
    - [x] `PlayerControllerIso` (Isometric movement)
    - [x] `PlayerControllerSide` (Platformer movement)
    - [ ] `PlayerControllerFPS` (First-person movement)

## Phase 2.5: Items & Inventory (Hack & Slash Foundation)
- [x] **Item Data Structure**
    - [x] `GameItem` (Resource): Base class (Name, Icon, Type)
    - [ ] `EquipmentItem` (Inherits `GameItem`): Stats (Attack, Defense)
- [x] **Inventory System**
    - [x] `Inventory` (Node/Resource): List of items, Add/Remove logic
- [ ] **Loot System**
    - [ ] `LootTable`: Weighted random item generation

## Phase 3: Game Cycles (Roguelike Loop)
- [ ] **Cycle Implementation**
    - [x] `RoguelikeGenerator` (Procedural Map):
        - [x] Room & Corridor Generation Algorithm
        - [x] Apply to `UniversalWorldData` (Floor=0, Wall=1)
    - [x] `RoguelikeManager` (Node):
        - [x] Orchestrate Dungeon Generation
        - [x] Spawn Player at Start Point
    - [ ] `SandboxSession`: Infinite generating & Saving

## Phase 4: Sample & Demo System (Templates)
- [x] **Sample Organization**
    - [x] Create `samples/` directory in Godot project.
    - [ ] **Demo 1: Roguelike**: `samples/roguelike/demo_rogue.tscn`
    - [ ] **Demo 2: Platformer**: `samples/platformer/demo_side.tscn`
    - [ ] **Demo 3: Mystery (Adventure)**: `samples/mystery/demo_adv.tscn`
- [x] **Setup Tool**
    - [x] `scripts/setup_demo.sh`: Script to switch `project.godot` main scene to selected demo.

## Phase 5: Adventure Features (Evaluation Phase)
- [ ] **Interaction System**
    - [ ] `InteractionManager`: Raycast/Area detection.
    - [ ] `DialogueUI`: Text display.

## Legacy Tasks (Completed)
- [x] Initial Isometric Sandbox Setup (Godot 4.3 + GDExtension)
- [x] Environment Setup (macOS M2)
