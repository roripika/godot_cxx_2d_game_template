#include "register_types.h"

#include "core/cycles/roguelike_generator.h"
#include "core/cycles/roguelike_manager.h"
#include "core/universal_world_data.h"
#include "entities/game_entity.h"
#include "entities/player_controller_iso.h"
#include "entities/player_controller_side.h"
#include "items/game_item.h"
#include "items/inventory.h"
#include "views/isometric_view.h"
#include "views/side_scrolling_view.h"
#include "world_generator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_sandbox_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }

  ClassDB::register_class<UniversalWorldData>();
  ClassDB::register_class<RoguelikeGenerator>();
  ClassDB::register_class<RoguelikeManager>();
  ClassDB::register_class<IsometricView>();
  ClassDB::register_class<SideScrollingView>();
  ClassDB::register_class<GameEntity>();
  ClassDB::register_class<PlayerControllerIso>();
  ClassDB::register_class<PlayerControllerSide>();
  ClassDB::register_class<GameItem>();
  ClassDB::register_class<Inventory>();
  ClassDB::register_class<WorldGenerator>();
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
