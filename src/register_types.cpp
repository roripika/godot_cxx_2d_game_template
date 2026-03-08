#include "register_types.h"

// Layer 1: Basic Game Karakuri (core)
#include "core/karakuri_game_state.h"
#include "core/action_registry.h"
#include "core/condition_evaluator.h"
#include "core/components/hitbox_component.h"
#include "core/components/hurtbox_component.h"
#include "core/components/interaction_component.h"
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
#include "core/services/action_runner.h"
#include "core/services/flag_service.h"
#include "core/services/input_service.h"
#include "core/services/item_service.h"
#include "core/services/localization_service.h"
#include "core/services/save_service.h"
#include "core/services/scene_flow.h"
#include "core/services/sound_service.h"
#include "core/ui/inventory_slot_ui.h"
#include "core/ui/inventory_ui.h"
#include "core/universal_world_data.h"
#include "plugins/features/fighting/fighting_game_manager.h"
#include "plugins/features/fighting/hit_stop_manager.h"
#include "plugins/features/rhythm/conductor.h"
#include "plugins/features/rhythm/rhythm_game_manager.h"

// Task sequence system (core)
#include "core/tasks/task_base.h"
#include "core/tasks/wait_task.h"
#include "core/tasks/task_group.h"
#include "core/tasks/state_snapshot.h"
#include "core/tasks/sequence_player.h"

// Task sequence system (mystery)
#include "mystery/tasks/zoom_camera_task.h"
#include "mystery/tasks/show_evidence_ui_task.h"
#include "mystery/tasks/play_mystery_sound_task.h"
#include "mystery/tasks/task_add_evidence.h"

// Layer 2: Mystery template
#include "mystery/mystery_game_state.h"
#include "mystery/evidence.h"
#include "mystery/evidence_manager.h"
#include "mystery/evidence_presenter.h"
#include "mystery/mystery_trigger.h"
#include "mystery/mystery_effect_map.h"
#include "mystery/mystery_manager.h"
#include "mystery/mystery_object.h"
#include "mystery/mystery_player.h"

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
  ClassDB::register_class<karakuri::KarakuriGameState>();
  ClassDB::register_class<karakuri::ActionRegistry>();
  ClassDB::register_class<karakuri::ConditionEvaluator>();
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
  ClassDB::register_class<karakuri::InteractionComponent>();
  ClassDB::register_class<karakuri::RayCastComponent>();
  ClassDB::register_class<karakuri::InventoryUI>();
  ClassDB::register_class<karakuri::InventorySlotUI>();
  ClassDB::register_class<karakuri::LocalizationService>();
  ClassDB::register_class<karakuri::Logger>();
  ClassDB::register_class<karakuri::ActionRunner>();
  ClassDB::register_class<karakuri::FlagService>();
  ClassDB::register_class<karakuri::InputService>();
  ClassDB::register_class<karakuri::ItemService>();
  ClassDB::register_class<karakuri::SaveService>();
  ClassDB::register_class<karakuri::SceneFlow>();
  ClassDB::register_class<karakuri::SoundService>();
  ClassDB::register_class<karakuri::ScenarioRunner>();

  // Task sequence system (core)
  ClassDB::register_class<karakuri::TaskBase>();
  ClassDB::register_class<karakuri::WaitTask>();
  ClassDB::register_class<karakuri::TaskGroup>();
  ClassDB::register_class<karakuri::StateSnapshot>();
  ClassDB::register_class<karakuri::SequencePlayer>();

  // Task sequence system (mystery)
  ClassDB::register_class<mystery::ZoomCameraTask>();
  ClassDB::register_class<mystery::ShowEvidenceUITask>();
  ClassDB::register_class<mystery::PlayMysterySoundTask>();
  ClassDB::register_class<mystery::TaskAddEvidence>();

  // Layer 2: Mystery template
  ClassDB::register_class<mystery::MysteryGameState>();
  ClassDB::register_class<mystery::Evidence>();
  ClassDB::register_class<mystery::EvidencePresenter>();
  ClassDB::register_class<mystery::MysteryEffectMap>();
  ClassDB::register_class<mystery::MysteryManager>();
  ClassDB::register_class<mystery::EvidenceManager>();
  ClassDB::register_class<mystery::MysteryObject>();
  ClassDB::register_class<mystery::MysteryPlayer>();
  ClassDB::register_class<mystery::MysteryTrigger>();

  // Plugins / Features
  ClassDB::register_class<FightingGameManager>();
  ClassDB::register_class<HitStopManager>();
  ClassDB::register_class<Conductor>();
  ClassDB::register_class<RhythmGameManager>();
  ClassDB::register_class<MiningComponent>();
  ClassDB::register_class<BuildingComponent>();
  ClassDB::register_class<RhythmNote>();
  ClassDB::register_class<NoteLane>();

  // Views & Scenes
  ClassDB::register_class<IsometricView>();
  ClassDB::register_class<karakuri::SideScrollingView>();
  ClassDB::register_class<WorldGenerator>();
  ClassDB::register_class<mystery::OfficeSceneLogic>();
  ClassDB::register_class<mystery::HauntedSpotSceneLogic>();
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
