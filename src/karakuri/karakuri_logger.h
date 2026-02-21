#ifndef KARAKURI_LOGGER_H
#define KARAKURI_LOGGER_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

/**
 * @file karakuri_logger.h
 * @brief Global Debug Logger for C++ and GDScript.
 * This class provides static methods to log messages at different levels (Debug, Info, Warn, Error).
 * Debug logs are filtered out unless the game is running in a debug build.
 */

namespace karakuri {

class KarakuriLogger : public godot::Object {
  GDCLASS(KarakuriLogger, godot::Object)

protected:
  static void _bind_methods();

public:
  // Core logging methods exposed to GDScript as static functions
  static void debug(const godot::String &msg);
  static void info(const godot::String &msg);
  static void warn(const godot::String &msg);
  static void error(const godot::String &msg);
};

} // namespace karakuri

// Convenient C++ Macros for internal use
#define KARAKURI_DEBUG(msg) karakuri::KarakuriLogger::debug(godot::String(msg))
#define KARAKURI_INFO(msg) karakuri::KarakuriLogger::info(godot::String(msg))
#define KARAKURI_WARN(msg) karakuri::KarakuriLogger::warn(godot::String(msg))
#define KARAKURI_ERR(msg) karakuri::KarakuriLogger::error(godot::String(msg))

#endif // KARAKURI_LOGGER_H
