#ifndef BASIC_GAME_KARAKURI_H
#define BASIC_GAME_KARAKURI_H

/**
 * @file karakuri.h
 * @brief Basic Game Karakuri の共通ヘッダ。
 *
 * Basic Game Karakuri は全デモ/全ゲームで使い回す Utility Framework。
 * このディレクトリ配下の C++ は Doxygen 形式のコメント必須。
 */

namespace karakuri {

/**
 * @brief Karakuri のバージョン情報。
 *
 * 破壊的変更が発生した際にバージョンを更新し、デモ側の追従判断に使う。
 */
struct Version {
  /** @brief Major version */
  static constexpr int kMajor = 0;
  /** @brief Minor version */
  static constexpr int kMinor = 1;
  /** @brief Patch version */
  static constexpr int kPatch = 0;
};

} // namespace karakuri

#endif // BASIC_GAME_KARAKURI_H

