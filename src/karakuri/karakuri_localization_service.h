#ifndef KARAKURI_LOCALIZATION_SERVICE_H
#define KARAKURI_LOCALIZATION_SERVICE_H

/**
 * @file karakuri_localization_service.h
 * @brief Basic Game Karakuri: ロケールの保存とランタイム切り替えサービス。
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief 全デモで共通利用されるロケールサービス。
 *
 * 責務:
 * - ロケールプレフィックス（`en`, `ja` など）を `user://` に保存。
 * - 起動時に保存されたロケールを復元。
 * - ロケール切り替え用の単一のランタイムAPIを提供。
 * - UIが即座に再描画できるよう `locale_changed(locale)` シグナルを発火。
 */
class KarakuriLocalizationService : public godot::Node {
  GDCLASS(KarakuriLocalizationService, godot::Node)

public:
  KarakuriLocalizationService() = default;
  ~KarakuriLocalizationService() override = default;

  /**
   * @brief Godotのライフサイクルフック。保存されたロケールを復元する。
   */
  void _ready() override;

  /**
   * @brief 永続化するロケールの保存先パスを設定する。
   * @param path ロケールプレフィックスを保存する `user://` パス。
   */
  void set_locale_store_path(const godot::String &path);

  /**
   * @brief 永続化されたロケールの保存先パスを取得する。
   * @return `user://` プレフィックスのファイルパス。
   */
  godot::String get_locale_store_path() const;

  /**
   * @brief ロケールプレフィックスからロケールを設定し、保存する。
   * @param prefix `en` や `ja` などのロケールプレフィックス。
   * @return 適用されたロケール文字列（例: `en_US`）。
   */
  godot::String set_locale_prefix(const godot::String &prefix);

  /**
   * @brief 現在アクティブなロケールプレフィックスを保存する。
   * @return 書き込みに成功した場合は `true`。
   */
  bool save_current_locale_prefix();

  /**
   * @brief 保存されたロケールプレフィックスを読み込み、適用する。
   * @return 適用されたロケール文字列。利用できない場合は空文字。
   */
  godot::String load_saved_locale_prefix();

  /**
   * @brief 現在のロケールプレフィックス（`en`, `ja` など）を解決する。
   * @return 現在の TranslationServer のロケールから抽出されたプレフィックス。
   */
  godot::String get_current_locale_prefix() const;

private:
  static void _bind_methods();

  godot::String resolve_loaded_locale(const godot::String &prefix) const;
  godot::String extract_prefix(const godot::String &locale) const;
  void emit_locale_changed();

  godot::String locale_store_path_ = "user://karakuri/locale.txt";
};

} // namespace karakuri

#endif // KARAKURI_LOCALIZATION_SERVICE_H
