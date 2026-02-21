#ifndef KARAKURI_YAML_LITE_H
#define KARAKURI_YAML_LITE_H

/**
 * @file karakuri_yaml_lite.h
 * @brief Basic Game Karakuri: プランナー向けシナリオファイル用のYAML
 * Liteパーサー。
 *
 * ゲームシナリオの設定に適した、敢えて小規模なYAMLサブセットのパーサーです:
 * - マップ: `key: value` および `key:` (ネストされたブロック)
 * - シーケンス: `- value`, `- key: value`, `- key:` (ネストされたブロック)
 * - スカラー: 文字列、整数、浮動小数点数、真偽値
 *
 * サポートしていない機能:
 * -
 * アンカーやエイリアス、複数行のスカラー、複雑なクォーテーション規則、インデント用のタブなど。
 *
 * 目的は、完全なYAMLライブラリの依存関係をGDExtensionビルドに取り込むことなく、
 * プランナーが書いたYAMLを読み書き可能な状態に保つことです。
 */

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

/**
 * @brief シナリオ用に使用される制限されたサブセット向けの最小のYAMLパーサー。
 */
class KarakuriYamlLite final {
public:
  /**
   * @brief
   * YAMLテキストをGodotのVariantツリー（Dictionary/Array/スカラー等）としてパースする。
   *
   * @param yaml_text YAMLのソーステキスト文字列。
   * @param out_root パース結果のルート。通常はDictionaryとなる。
   * @param out_error パース失敗時にエラーメッセージが格納される。
   * @return パースに成功した場合はtrueを返す。
   */
  static bool parse(const godot::String &yaml_text, godot::Variant &out_root,
                    godot::String &out_error);
};

} // namespace karakuri

#endif // KARAKURI_YAML_LITE_H
