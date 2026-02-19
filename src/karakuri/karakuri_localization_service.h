#ifndef KARAKURI_LOCALIZATION_SERVICE_H
#define KARAKURI_LOCALIZATION_SERVICE_H

/**
 * @file karakuri_localization_service.h
 * @brief Basic Game Karakuri: locale persistence and runtime switch service.
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief Shared locale service used by all demos.
 *
 * Responsibilities:
 * - Persist locale prefix (`en`, `ja`, ...) to `user://`.
 * - Restore saved locale on startup.
 * - Provide a single runtime API for locale switching.
 * - Emit `locale_changed(locale)` so UIs can redraw immediately.
 */
class KarakuriLocalizationService : public godot::Node {
  GDCLASS(KarakuriLocalizationService, godot::Node)

public:
  KarakuriLocalizationService() = default;
  ~KarakuriLocalizationService() override = default;

  /**
   * @brief Godot lifecycle hook. Restores saved locale.
   */
  void _ready() override;

  /**
   * @brief Set storage path for persisted locale.
   * @param path `user://` path where locale prefix is stored.
   */
  void set_locale_store_path(const godot::String &path);

  /**
   * @brief Get storage path for persisted locale.
   * @return `user://` file path.
   */
  godot::String get_locale_store_path() const;

  /**
   * @brief Set locale by prefix and persist it.
   * @param prefix Locale prefix such as `en` or `ja`.
   * @return Applied locale string (can be `en_US` etc).
   */
  godot::String set_locale_prefix(const godot::String &prefix);

  /**
   * @brief Save the currently active locale prefix.
   * @return `true` on successful write.
   */
  bool save_current_locale_prefix();

  /**
   * @brief Load saved locale prefix and apply it.
   * @return Applied locale string. Empty when not available.
   */
  godot::String load_saved_locale_prefix();

  /**
   * @brief Resolve current locale prefix (`en`, `ja`, ...).
   * @return Prefix extracted from current TranslationServer locale.
   */
  godot::String get_current_locale_prefix() const;

private:
  static void _bind_methods();

  godot::String resolve_loaded_locale(const godot::String &prefix) const;
  godot::String extract_prefix(const godot::String &locale) const;
  void emit_locale_changed();

  godot::String locale_store_path_ = "user://karakuri_locale.txt";
};

} // namespace karakuri

#endif // KARAKURI_LOCALIZATION_SERVICE_H
