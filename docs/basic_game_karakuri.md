# Basic Game Karakuri

## 位置付け
**Basic Game Karakuri** は、全デモ/全ゲームで共通利用する Utility Framework です。

- デモ固有ロジックは含めない
- 依存の向きは「デモ → Karakuri」を基本とする（逆依存しない）

## 管理場所
- C++（GDExtension）: `src/karakuri/`

## コメント規約（必須）
`src/karakuri/**` 配下の C++ ソースは **必ず Doxygen 形式**でコメントを書くこと。

### 例（クラス）
```cpp
/**
 * @brief シーン遷移を統一するサービス。
 *
 * フェード、履歴、戻る等をここに集約する想定。
 */
class SceneFlowService {
  // ...
};
```

### 例（関数）
```cpp
/**
 * @brief シーンを切り替える。
 * @param path res:// で始まるシーンパス
 * @return 成功時 true
 */
bool change_scene(const godot::String &path);
```

## スコープ（最初に揃えるもの）
- Localization（locale永続化、locale変更通知、UI再描画トリガ）
- SceneFlow（フェード付き遷移、戻る、メインメニュー復帰）
- SaveData（最小のセーブ/ロード、デモ別namespace）
- Debug（オン/オフ、コマンド）

## 現在の実装（2026-02-19）
- `KarakuriLocalizationService`:
  - 実装: `src/karakuri/karakuri_localization_service.h`, `src/karakuri/karakuri_localization_service.cpp`
  - Autoload: `project.godot` の `KarakuriLocalization`
  - 役割:
    - locale prefix の `user://` 永続化
    - 起動時の locale 復元
    - 実行中切替 API（`set_locale_prefix`）
    - `locale_changed(locale)` signal 通知
