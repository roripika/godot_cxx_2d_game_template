#include "karakuri_logger.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void KarakuriLogger::_bind_methods() {
  ClassDB::bind_static_method("KarakuriLogger", D_METHOD("debug", "msg"),
                              &KarakuriLogger::debug);
  ClassDB::bind_static_method("KarakuriLogger", D_METHOD("info", "msg"),
                              &KarakuriLogger::info);
  ClassDB::bind_static_method("KarakuriLogger", D_METHOD("warn", "msg"),
                              &KarakuriLogger::warn);
  ClassDB::bind_static_method("KarakuriLogger", D_METHOD("error", "msg"),
                              &KarakuriLogger::error);
}

void KarakuriLogger::debug(const String &msg) {
  // Only log if we are in a debug build (typically the editor or an exported
  // debug build)
  if (OS::get_singleton()->is_debug_build()) {
    UtilityFunctions::print("[DEBUG] ", msg);
  }
}

void KarakuriLogger::info(const String &msg) {
  UtilityFunctions::print("[INFO] ", msg);
}

void KarakuriLogger::warn(const String &msg) {
  // Push an actual editor warning in addition to printing
  UtilityFunctions::push_warning(msg);
  UtilityFunctions::print("[WARN] ", msg);
}

void KarakuriLogger::error(const String &msg) {
  // Push an actual editor error in addition to printing
  UtilityFunctions::push_error(msg);
  UtilityFunctions::printerr("[ERROR] ", msg);
}

} // namespace karakuri
