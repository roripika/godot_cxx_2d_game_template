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
    - [x] **Demo 1: Roguelike**: `samples/roguelike/` (Dungeon Crawler)
    - [x] **Demo 2: Platformer**: `samples/platformer/` (Side Action)
    - [x] **Demo 3: Mystery**: `samples/mystery/` (Urban Myth Adventure)
    - [x] **Demo 4: Sandbox**: `samples/sandbox/` (CoreKeeper-like Mining/Crafting)
    - [x] **Demo 5: Fighting**: `samples/fighting/` (Street Fighter-like VS Battle)
- [x] **Setup Tool**
    - [x] `scripts/setup_demo.sh`: Script to switch `project.godot` main scene to selected demo.

## Phase 5: Adventure Features (逆転裁判/都市伝説スタイル)
- [x] **Interaction System**
    - [x] `InteractionManager`: Mouse Input & Signal emission.
    - [x] `DialogueUI`: Text display components.
- [ ] **Core Adventure Systems**
    - [ ] `GlobalState` (Autoload): Manage flags, variables, and story progress.
    - [ ] `SceneTransitionManager`: Seamless scene switching with fade effects.
    - [ ] `ChoiceManager`: Display choices and handle branching logic.
    - [ ] `FlagCondition`: Conditional scene/dialogue display based on flags.
- [ ] **Evidence & Inventory**
    - [ ] `EvidenceItem` (Resource): Evidence data (Name, Description, Icon, Category).
    - [ ] `EvidenceManager`: Add/Remove/Check evidence.
    - [ ] `InventoryUI`: Display evidence list with details view.
    - [ ] `EvidencePresentSystem`: Click evidence to present in dialogue.
- [ ] **Character & Presentation**
    - [ ] `CharacterPortrait`: Sprite with multiple expressions/poses.
    - [ ] `PortraitManager`: Switch portraits based on dialogue.
    - [ ] `TextAnimator`: Type-writer effect with speed control.
- [ ] **Investigation Mode**
    - [ ] `InvestigationScene`: Click hotspots to examine/talk/move.
    - [ ] `HotspotManager`: Define clickable areas with descriptions.
    - [ ] `LocationMap`: Visual map for scene navigation.
- [ ] **Court/Confrontation Mode**
    - [ ] `TestimonySystem`: Display witness testimony line-by-line.
    - [ ] `ContradictionDetector`: Press/Present mechanics.
    - [ ] `HealthBar`: Penalty system for wrong answers.
- [ ] **Demo Scenes**
    - [ ] **Scene 1: Investigation**: Office with clues and NPCs.
    - [ ] **Scene 2: Dialogue**: Conversation with choices.
    - [ ] **Scene 3: Presentation**: Court-style evidence presentation.
    - [ ] **Scene 4: Multiple Endings**: Flag-based story branches.

## Phase 6: Fighting Game Features [NEW]
- [ ] **Combat System**
    - [ ] `FightingEntity` (Inherits GameEntity): State Machine (Idle, Walk, Attack, Stun).
    - [ ] `HitboxManager`: Frame-based collision detection.
    - [ ] `InputBuffer`: Command interpretation (Quarter-circle, etc.).

## Phase 7: Sandbox Features [NEW]
- [ ] **World Interaction**
    - [ ] `MiningSystem`: Break voxels and drop items.
    - [ ] `BuildingSystem`: Place voxels from inventory.

## Phase 8: Shooting Game Features
- [ ] **Bullet System**
    - [ ] `BulletManager`: Object pooling for projectiles.
    - [ ] `BulletPattern`: Danmaku pattern generator.
- [ ] **Enemy Formation**
    - [ ] `EnemyWaveManager`: Spawn enemies in waves.
    - [ ] `PathFollower`: Predefined enemy movement paths.

## Phase 9: Puzzle Game Features
- [ ] **Board System**
    - [ ] `PuzzleBoard`: Grid-based tile matching.
    - [ ] `MatchDetector`: Pattern recognition (match-3, etc.).
- [ ] **Physics Puzzle**
    - [ ] `FallingBlockSystem`: Gravity-based tile fall.

## Phase 10: Strategy Game Features
- [ ] **Unit Management**
    - [ ] `RTSUnitController`: Multi-unit selection & command.
    - [ ] `PathfindingManager`: A* / Flow field navigation.
- [ ] **Turn-Based System**
    - [ ] `TurnManager`: Initiative-based turn order.
    - [ ] `GridTacticsController`: SRPG-style movement.

## Phase 11: Card Game Features
- [ ] **Card System**
    - [ ] `Card` (Resource): Base card data (Cost, Effect).
    - [ ] `Deck`: Card collection with shuffle/draw.
    - [ ] `Hand`: Active cards display & management.
- [ ] **Battle System**
    - [ ] `CardBattleManager`: Turn flow for card battles.

## Phase 12: Tower Defense Features
- [ ] **Tower System**
    - [ ] `Tower`: Turret with range & targeting.
    - [ ] `TowerPlacement`: Grid-based tower building.
- [ ] **Enemy Waves**
    - [ ] `WaveSpawner`: Timed enemy generation.
    - [ ] `PathManager`: Enemy movement along predefined routes.

## Phase 13: Racing Game Features
- [ ] **Vehicle System**
    - [ ] `VehicleController`: Acceleration, steering, drift.
    - [ ] `TrackManager`: Checkpoint & lap system.
- [ ] **Physics**
    - [ ] Custom vehicle physics or Godot VehicleBody3D integration.

## Phase 14: Simulation Features
- [ ] **Resource Management**
    - [ ] `ResourcePool`: Complex economy system (money, materials, etc.).
    - [ ] `ProductionChain`: Resource conversion & crafting trees.
- [ ] **AI Characters**
    - [ ] `NPCScheduler`: Daily routine & behavior trees.
    - [ ] `RelationshipSystem`: NPC affinity & reaction.

## Phase 15: Metroidvania Features
- [ ] **Exploration System**
    - [ ] `MapRevealSystem`: Fog of war & area discovery.
    - [ ] `AbilityGate`: Block areas until player gains abilities.
- [ ] **Progression**
    - [ ] `PowerupManager`: Collectible abilities (double jump, etc.).

## Legacy Tasks (Completed)
- [x] Initial Isometric Sandbox Setup (Godot 4.3 + GDExtension)
- [x] Environment Setup (macOS M2)
