#include "register_types.h"

// Layer 1: Basic Game Karakuri (core)
#include "core/adventure_game_state.h"
#include "core/components/hitbox_component.h"
#include "core/components/hurtbox_component.h"
#include "core/components/raycast_component.h"
#include "core/cycles/roguelike_generator.h"
#include "core/cycles/roguelike_manager.h"
#include "core/dialogue_ui.h"
#include "core/entities/base_entity.h"
#include "core/interaction_manager.h"
#include "core/items/game_item.h"
#include "core/items/inventory.h"
#include "core/logger/karakuri_logger.h"
#include "core/scenario/scenario_runner.h"
#include "core/services/localization_service.h"
#include "core/services/save_service.h"
#include "core/ui/inventory_slot_ui.h"
#include "core/ui/inventory_ui.h"
#include "core/universal_world_data.h"
#include "plugins/features/fighting/fighting_game_manager.h"
#include "plugins/features/fighting/hit_stop_manager.h"
#include "plugins/features/rhythm/conductor.h"
#include "plugins/features/rhythm/rhythm_game_manager.h"

// Layer 2: Mystery template
#include "mystery/evidence_manager.h"
#include "mystery/mystery_manager.h"

// Plugins (genre-specific, isolated)
#include "plugins/features/sandbox/building_component.h"
#include "plugins/features/sandbox/mining_component.h"

// Views & Scenes
#include "core/views/side_scrolling_view.h"
#include "mystery/scenes/haunted_spot_scene_logic.h"
#include "mystery/scenes/office_scene_logic.h"
#include "plugins/views/isometric_view.h"
#include "plugins/views/rhythm/note_lane.h"
#include "plugins/views/rhythm/rhythm_note.h"
#include "world_generator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_sandbox_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  godot::UtilityFunctions::print("Sandbox GDExtension Initialized!");

  // Layer 1: Basic Game Karakuri
  ClassDB::register_class<karakuri::AdventureGameStateBase>();
  ClassDB::register_class<karakuri::UniversalWorldData>();
  ClassDB::register_class<karakuri::RoguelikeGenerator>();
  ClassDB::register_class<karakuri::RoguelikeManager>();
  ClassDB::register_class<karakuri::InteractionManager>();
  ClassDB::register_class<karakuri::DialogueUI>();
  ClassDB::register_class<karakuri::BaseEntity>();
  ClassDB::register_class<karakuri::GameItem>();
  ClassDB::register_class<karakuri::Inventory>();
  ClassDB::register_class<karakuri::HitboxComponent>();
  ClassDB::register_class<karakuri::HurtboxComponent>();
  ClassDB::register_class<karakuri::RayCastComponent>();
  ClassDB::register_class<karakuri::InventoryUI>();
  ClassDB::register_class<karakuri::InventorySlotUI>();
  ClassDB::register_class<karakuri::LocalizationService>();
  ClassDB::register_class<karakuri::KarakuriLogger>();
  ClassDB::register_class<karakuri::SaveService>();
  ClassDB::register_class<karakuri::ScenarioRunner>();

  // Layer 2: Mystery template
  ClassDB::register_class<mystery::MysteryManager>();
  ClassDB::register_class<mystery::EvidenceManager>();

  // Plugins / Features
  ClassDB::register_class<FightingGameManager>();
  ClassDB::register_class<HitStopManager>();
  ClassDB::register_class<karakuri::Conductor>();
  ClassDB::register_class<karakuri::RhythmGameManager>();
  ClassDB::register_class<karakuri::MiningComponent>();
  ClassDB::register_class<karakuri::BuildingComponent>();
  ClassDB::register_class<karakuri::RhythmNote>();
  ClassDB::register_class<karakuri::NoteLane>();

  // Views & Scenes
  ClassDB::register_class<karakuri::IsometricView>();
  ClassDB::register_class<karakuri::SideScrollingView>();
  ClassDB::register_class<karakuri::WorldGenerator>();
  ClassDB::register_class<karakuri::OfficeSceneLogic>();
  ClassDB::register_class<karakuri::HauntedSpotSceneLogic>();
}

void uninitialize_sandbox_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
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
