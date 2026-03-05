#!/usr/bin/env python3
"""Write clean save_service.h (generic API)."""
import os

WORKSPACE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(WORKSPACE, "src/core/services/save_service.h")

content = r"""#ifndef KARAKURI_SAVE_SERVICE_H
#define KARAKURI_SAVE_SERVICE_H

/**
 * @file save_service.h
 * @brief Basic Game Karakuri: JSON ファイル I/O の汎用セーブサービス。
 *
 * このクラス自身は「何を保存するか」を知りません。
 * 呼び出し元（mystery 層等）が Dictionary を組み立てて渡します。
 *
 * 保存パス規約: user://karakuri/<filename>.json
 *
 * 使い方:
 *   // 保存
 *   Dictionary data = ...;  // 呼び出し元が組み立てる
 *   karakuri::SaveService::save_game("mystery", data);
 *
 *   // 読み込み
 *   Dictionary data = karakuri::SaveService::load_game("mystery");
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief 汎用 JSON セーブサービス。
 *
 * - save_game(filename, data) : Dictionary を JSON ファイルに書き出す。
 * - load_game(filename)       : JSON ファイルを読み込み Dictionary を返す。
 * - has_save(filename)        : セーブファイルの存在確認。
 * - delete_save(filename)     : セーブファイルの削除。
 *
 * filename は拡張子なし（"mystery" → user://karakuri/mystery.json）。
 */
class SaveService : public godot::Node {
  GDCLASS(SaveService, godot::Node)

public:
  SaveService() = default;
  ~SaveService() override = default;

  /**
   * @brief セーブファイルの絶対パスを返す。
   * @param filename 拡張子なしのファイル名。
   */
  static godot::String build_save_path(const godot::String &filename);

  /**
   * @brief Dictionary を JSON ファイルに保存する。
   * @param filename 拡張子なしのファイル名。
   * @param data     保存するデータ。
   * @return 成功なら true。
   */
  static bool save_game(const godot::String &filename,
                        const godot::Dictionary &data);

  /**
   * @brief JSON ファイルを読み込み Dictionary を返す。
   * @param filename 拡張子なしのファイル名。
   * @return 読み込んだデータ。失敗時は空 Dictionary。
   */
  static godot::Dictionary load_game(const godot::String &filename);

  /**
   * @brief セーブファイルが存在するか確認する。
   */
  static bool has_save(const godot::String &filename);

  /**
   * @brief セーブファイルを削除する。
   */
  static bool delete_save(const godot::String &filename);

private:
  static void _bind_methods();
};

} // namespace karakuri

#endif // KARAKURI_SAVE_SERVICE_H
"""

with open(path, 'w') as f:
    f.write(content)
print(f"Written: {path}")
print(f"Lines: {len(content.splitlines())}")
