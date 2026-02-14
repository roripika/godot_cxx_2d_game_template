#include "register_types.h"

#include "core/adventure_game_state.h"
#include "core/cycles/roguelike_generator.h"
#include "core/cycles/roguelike_manager.h"
#include "core/dialogue_ui.h"
#include "core/fighting_game_manager.h"
#include "core/interaction_manager.h"
#include "core/universal_world_data.h"
#include "entities/fighter_controller.h"
#include "entities/game_entity.h"
#include "entities/player_controller_iso.h"
#include "entities/player_controller_side.h"
#include "items/game_item.h"
#include "items/inventory.h"
#include "views/isometric_view.h"
#include "views/side_scrolling_view.h"
#include "world_generator.h"

#include "core/hit_stop_manager.h"
#include "entities/components/hitbox_component.h"
#include "entities/components/hurtbox_component.h"
#include "entities/components/raycast_component.h"

#include "core/ui/inventory_slot_ui.h"
#include "core/ui/inventory_ui.h"

#include "features/sandbox/building_component.h"
#include "features/sandbox/mining_component.h"

#include "features/sandbox/building_component.h"
#include "features/sandbox/mining_component.h"

#include "features/sandbox/building_component.h"
#include "features/sandbox/mining_component.h"

#include "core/rhythm/conductor.h"
#include "core/rhythm/rhythm_game_manager.h"
#include "features/rhythm/rhythm_command_listener.h"
#include "views/rhythm/note_lane.h"
#include "views/rhythm/rhythm_note.h"

#include "scenes/haunted_spot_scene_logic.h"
#include "scenes/office_scene_logic.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_sandbox_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  godot::UtilityFunctions::print("Sandbox GDExtension Initialized!");

  ClassDB::register_class<AdventureGameStateBase>();
  ClassDB::register_class<OfficeSceneLogic>();
  ClassDB::register_class<HauntedSpotSceneLogic>();
  ClassDB::register_class<FightingGameManager>();
  ClassDB::register_class<FighterController>();
  ClassDB::register_class<UniversalWorldData>();
  ClassDB::register_class<RoguelikeGenerator>();
  ClassDB::register_class<RoguelikeManager>();
  ClassDB::register_class<InteractionManager>();
  ClassDB::register_class<DialogueUI>();
  ClassDB::register_class<IsometricView>();
  ClassDB::register_class<SideScrollingView>();
  ClassDB::register_class<GameEntity>();
  ClassDB::register_class<PlayerControllerIso>();
  ClassDB::register_class<PlayerControllerSide>();
  ClassDB::register_class<GameItem>();
  ClassDB::register_class<Inventory>();
  ClassDB::register_class<WorldGenerator>();
  ClassDB::register_class<HitboxComponent>();
  ClassDB::register_class<HurtboxComponent>();
  ClassDB::register_class<RayCastComponent>();
  ClassDB::register_class<HitStopManager>();
  ClassDB::register_class<InventoryUI>();
  ClassDB::register_class<InventorySlotUI>();
  ClassDB::register_class<MiningComponent>();
  ClassDB::register_class<BuildingComponent>();
  ClassDB::register_class<Conductor>();
  ClassDB::register_class<RhythmGameManager>();
  ClassDB::register_class<RhythmNote>();
  ClassDB::register_class<NoteLane>();
  ClassDB::register_class<RhythmCommandListener>();
}

void uninitialize_sandbox_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT
sandbox_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                     const GDExtensionClassLibraryPtr p_library,
                     GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_sandbox_module);
  init_obj.register_terminator(uninitialize_sandbox_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}
