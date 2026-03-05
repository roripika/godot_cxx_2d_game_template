#ifndef KARAKURI_I_INTERACTABLE_H
#define KARAKURI_I_INTERACTABLE_H

/**
 * @file i_interactable.h
 * @brief Basic Game Karakuri: 調査・インタラクト可能オブジェクトのインターフェース。
 *
 * ## 設計ポリシー
 * - このインターフェースは **karakuri 層のみ** に属する。
 * - 実装クラス（MysteryObject 等）は mystery 層に存在してよい。
 * - InteractionComponent はこのインターフェースのみを参照し、
 *   具体的なシナリオ・ミステリーの概念を知らない。
 *
 * ## 使い方
 * 1. 調査対象の Node クラスに IInteractable を多重継承させる。
 * 2. on_interact() で会話開始・フラグ更新等を実装する。
 * 3. _ready() で add_to_group("interactable") を呼ぶ。
 *    InteractionComponent が自動的に検知する。
 */

#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief プレイヤーが調査・インタラクト可能なオブジェクトの純粋仮想インターフェース。
 *
 * @note Godot の GDCLASS には属さない純粋 C++ ABC。
 *       動的型検査は dynamic_cast<IInteractable*> で行う。
 */
class IInteractable {
public:
  virtual ~IInteractable() = default;

  /**
   * @brief プレイヤーが調査アクションを実行したときに呼ばれる。
   *
   * 実装例:
   * - シナリオ JSON を読み込んで会話を開始する
   * - フラグを確認してセリフを分岐する
   * - 証拠品をインベントリに追加する
   */
  virtual void on_interact() = 0;

  /**
   * @brief 調査アイコン（「！」等）に表示するラベルを返す。
   * @return 空文字列の場合はデフォルトアイコンを表示。
   */
  virtual godot::String get_interaction_label() const { return ""; }
};

} // namespace karakuri

#endif // KARAKURI_I_INTERACTABLE_H
