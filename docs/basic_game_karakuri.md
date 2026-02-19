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
- Localization（locale永続化、locale変更通知、UI再描画トリガ） ✅ 実装済み
- SceneFlow（フェード付き遷移、戻る、メインメニュー復帰）
- SaveData（最小のセーブ/ロード、デモ別namespace） ✅ 実装済み（`KarakuriSaveService`）
- Debug（オン/オフ、コマンド）

## 現在の実装（2026-02-20 更新）
- `KarakuriLocalizationService`:
  - 実装: `src/karakuri/karakuri_localization_service.h`, `src/karakuri/karakuri_localization_service.cpp`
  - Autoload: `project.godot` の `KarakuriLocalization`
  - 役割:
    - locale prefix の `user://` 永続化
    - 起動時の locale 復元
    - 実行中切替 API（`set_locale_prefix`）
    - `locale_changed(locale)` signal 通知
- `KarakuriSaveService`:
  - 実装: `src/karakuri/karakuri_save_service.h`, `src/karakuri/karakuri_save_service.cpp`
  - Autoload 不要（静的メソッド / GDExtension クラス）
  - 役割:
    - `AdventureGameStateBase` の flags/inventory/health を JSON 永続化
    - `user://karakuri/<demo_id>/save.json` に保存
    - YAML アクション `save` / `load` として ScenarioRunner に組み込み済み

## KarakuriScenarioRunner の配置方針（2026-02-20 決定）

`KarakuriScenarioRunner` は `src/karakuri/scenario/` に置く（B案採用）。

### 設計原則

- **汎用 Action** は `init_builtin_actions()` で自動登録される（`dialogue` / `goto` / `choice` / `set_flag` / `give_evidence` / `give_item` / `wait` / `if_flag` / `if_has_items` / `reset_game` / `change_root_scene`）。
- **デモ固有 Action** はRunnerコアに書かず、デモ側の初期化で注入する。
  - Mystery デモは `register_mystery_actions()` を呼ぶ（`testimony` / `take_damage` / `if_health_ge` / `if_health_leq`）。
  - 新デモは `register_action("kind", handler)` を直接呼ぶ。

### デモ側の呼び出し規約

```gdscript
# mystery_shell_ui.gd の _ready() にて
func _ready() -> void:
    $ScenarioRunner.register_mystery_actions()
```

新デモで独自 Action を追加する場合:

```gdscript
func _ready() -> void:
    $ScenarioRunner.register_action("my_action", func(payload):
        # 処理
        return true
    )
```

### 禁止事項

- `KarakuriScenarioRunner` の `init_builtin_actions()` にデモ固有ロジックを追加しない。
- Mystery 以外のデモが `register_mystery_actions()` を呼ばない。

## `user://` ファイル命名規約（2026-02-20 決定）

### ディレクトリ構成

```
user://karakuri/              ← Karakuri共通ファイルのルート
    locale.txt                ← KarakuriLocalizationService が書き出す
user://karakuri/<demo_id>/    ← デモ固有ファイルはここに入れる
    save.json                 例: Mysteryのセーブデータ
    progress.json             例: リズムゲームの進捗
```

### ルール

- `user://` 直下にはファイルを書かない。必ず `user://karakuri/` 以下に配置する。
- Karakuri共通ファイル（ロケール等）は `user://karakuri/<filename>` に置く。
- デモ固有ファイルは `user://karakuri/<demo_id>/<filename>` に置く。
  - `demo_id` の例: `mystery` / `rhythm` / `roguelike`
- 複数デモを同一プロジェクトで動かす場合の衝突を防ぐため、prefix を省略しない。

### 実装例（C++）

```cpp
// Karakuri共通
const String path = "user://karakuri/locale.txt";
// デモ固有
const String path = "user://karakuri/mystery/save.json";
```

`KarakuriLocalizationService` はデフォルトパスとして `user://karakuri/locale.txt` を使用しており、ディレクトリが存在しない場合は `DirAccess::make_dir_recursive_absolute()` で自動作成する。

---

## `KarakuriSaveService` — ゲームステート永続化（2026-02-20 実装）

### 概要

`AdventureGameStateBase`（flags / inventory / health）を JSON 形式でファイルに保存・ロードするサービス。  
`src/karakuri/karakuri_save_service.h` / `.cpp` に実装済み。

### 保存フォーマット（`user://karakuri/<demo_id>/save.json`）

```json
{
  "flags": { "spoke_to_witness": true, "found_evidence": false },
  "inventory": ["bloody_knife", "map"],
  "health": 2
}
```

### YAML からの呼び出し

```yaml
actions:
  - save: "mystery"          # user://karakuri/mystery/save.json に書き出す
  - load: "mystery"          # ファイルが存在しない場合は警告ログのみ（進行継続）
```

`save` / `load` は `KarakuriScenarioRunner::init_builtin_actions()` に組み込み済み。  
**デモ個別に `register_*_actions()` を呼ぶ必要はない**。

### GDScript からの直接呼び出し

```gdscript
# 静的メソッドなのでノードなしで呼べる
KarakuriSaveService.save_game("mystery")
KarakuriSaveService.load_game("mystery")
KarakuriSaveService.has_save("mystery")   # bool
KarakuriSaveService.delete_save("mystery")
```

### スナップショット API（`AdventureGameStateBase`）

セーブサービスが内部的に使用。GDScript から呼ぶことも可能:

```gdscript
var flags_dict = $AdventureState.get_flags_snapshot()      # Dictionary
var inv_array  = $AdventureState.get_inventory_snapshot()  # Array
$AdventureState.restore_flags_snapshot(flags_dict)
$AdventureState.restore_inventory_snapshot(inv_array)
```

