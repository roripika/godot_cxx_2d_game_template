#ifndef KARAKURI_SAVE_SERVICE_H
#define KARAKURI_SAVE_SERVICE_H

/**
 * @file karakuri_save_service.h
 * @brief Basic Game Karakuri: AdventureGameStateBase の永続化サービス。
 *
 * 保存パス規約: user://karakuri/<demo_id>/save.json
 * (demo_id が空の場合は user://karakuri/save.json)
 *
 * 保存フォーマット (JSON):
 * {
 *   "flags": { "flag_name": true, ... },
 *   "inventory": ["item1", "item2", ...],
 *   "health": 3
 * }
 *
 * ScenarioRunner との連携:
 *   KarakuriScenarioRunner::init_builtin_actions() が
 *   "save" / "load" アクションをこのサービス経由で登録する。
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief ゲームステート永続化サービス。
 *
 * このノードをシーンに置くか、ScenarioRunner が内部的に使用する。
 * GDScript から直接呼ぶことも可能:
 *   $SaveService.save_game("mystery")
 *   $SaveService.load_game("mystery")
 */
class KarakuriSaveService : public godot::Node {
  GDCLASS(KarakuriSaveService, godot::Node)

public:
  KarakuriSaveService() = default;
  ~KarakuriSaveService() override = default;

  /**
   * @brief demo_id ごとのセーブファイルパスを返す。
   * @param demo_id デモ識別子 (空文字可)。
   * @return user:// パス文字列。
   */
  static godot::String build_save_path(const godot::String &demo_id);

  /**
   * @brief AdventureGameStateBase の状態をファイルに書き出す。
   * @param demo_id 保存先サブフォルダ。
   * @return 成功なら true。
   */
  static bool save_game(const godot::String &demo_id);

  /**
   * @brief ファイルから読み込んで AdventureGameStateBase に反映する。
   * @param demo_id 読み込むサブフォルダ。
   * @return 成功なら true。
   */
  static bool load_game(const godot::String &demo_id);

  /**
   * @brief セーブファイルが存在するか確認する。
   * @param demo_id セーブのサブフォルダ。
   * @return 存在なら true。
   */
  static bool has_save(const godot::String &demo_id);

  /**
   * @brief セーブファイルを削除する。
   * @param demo_id サブフォルダ。
   * @return 成功なら true。
   */
  static bool delete_save(const godot::String &demo_id);

private:
  static void _bind_methods();
};

} // namespace karakuri

#endif // KARAKURI_SAVE_SERVICE_H
