# Godot C++ 2D Game Template - Agent Guidelines

このファイルは `godot_cxx_2d_game_template/` 配下（Godotプロジェクト）で作業するエージェント向けの行動指針です。

## 1. 言語
- 基本は **日本語**（README/設計書/コミットメッセージも日本語）
- コードコメントは原則日本語（ただし `src/karakuri/**` は Doxygen 形式で書く）

## 2. アーキテクチャ（最重要）
このプロジェクトは **Universal Game Template**（複数ジャンルのデモを共通基盤で回すテンプレ）です。

- `src/core`: ゲームのデータ/ルール/状態（可能な限り Godot シーングラフ依存を避ける）
- `src/views`: 表示・操作アダプタ（TileMapLayer 等、Godot依存はここに寄せる）
- `src/entities`: 操作主体（PlayerController など）
- `samples/**`: デモ固有の Godot シーン/スクリプト

### GDScript に書いてよい処理（正リスト）

原則 **「ViewはGDScript、ロジックはC++」**。以下のカテゴリのみ GDScript に書く。

| カテゴリ | 例 | 禁止事項 |
|---|---|---|
| ノード参照・接続 | `$Node` の取得、`connect()` | ゲームロジック判定 |
| 表示更新トリガ | `label.text = tr("key")` | 翻訳キー以外の文字列生成ロジック |
| シグナル中継 | `runner.on_clicked_at(pos)` を受けて転送 | クリック座標変換やHotspot判定 |
| C++ APIの呼び出し | `register_mystery_actions()`, `start_scenario()` | C++ メソッドの再実装 |
| Autoload参照 | `AdventureGameState.get_flag("x")` の結果でUIを出す | フラグ判定によるシナリオ分岐ロジック |
| アニメーション制御 | `AnimationPlayer.play()` | アニメーション条件の判定 |
| Scene遷移の起点 | ボタン押下で `$Runner.start_scenario()` 呼び出し | 遷移先のシーンパス生成ロジック |

疑問が生じたら「**GDScript 側がテスト不能なロジックを持っていないか**」を確認する。

## 3. Basic Game Karakuri（共通Utility Framework）
全デモ/全ゲームで使い回す共通機構（ローカライズ、シーン遷移、セーブ、デバッグ等）の枠組みを **Basic Game Karakuri** と呼びます。

- 配置: `src/karakuri/`
- 禁止: デモ固有ロジックを Karakuri に入れない
- Doxygen必須:
  - `src/karakuri/**` 配下の C++（`.h/.hpp/.cpp`）は、公開APIに **必ず Doxygen 形式のコメント**を書く
  - 例: `/** ... */`, `/// ...`, `@param`, `@return`

### KarakuriScenarioRunner の配置方針（2026-02-20 決定・B案）
`KarakuriScenarioRunner` は `src/karakuri/scenario/` に置く。ただしデモ固有 Action はRunnerコアから除外し、**デモ側の初期化で `register_action()` / `register_mystery_actions()` を呼んで注入する**。

| 種別 | 場所 | 例 |
|---|---|---|
| 汎用 Action | `init_builtin_actions()`（自動登録） | `dialogue` / `goto` / `choice` / `set_flag` 等 |
| デモ固有 Action | デモ側 `_ready()` で `register_mystery_actions()` を呼ぶ | `testimony` / `take_damage` / `if_health_ge` / `if_health_leq` |
| 追加カスタム Action | `register_action("kind", handler)` を直接呼ぶ | 新デモ独自Action |

Mystery shell での呼び出し例（`mystery_shell_ui.gd`）:
```gdscript
func _ready() -> void:
    $ScenarioRunner.register_mystery_actions()
```

C++バインディングを変更したら、同一コミットで接続先 GDScript を全て更新すること。

### C++バインディング変更時のルール（2026-02-20 決定）

GDExtension の公開 API（`ClassDB::bind_method` / `ADD_PROPERTY` / `ADD_SIGNAL`）を変更・削除した場合、**同一コミット内で** 接続先 GDScript を全て更新する。

| 変更種別 | 対応必須なファイル |
|---|---|
| メソッド名変更・引数追加 | 呼び出し元 GDScript 全て |
| シグナル名変更・引数変更 | `connect(...)` しているスクリプト全て |
| プロパティ名変更 | `@export`・`get_node`・エディタ設定（`.tscn`）|
| メソッド削除 | 依存スクリプトを同時に書き換えるか削除する |

**検索コマンド（変更前に必ず実行）:**
```bash
grep -rn "旧メソッド名" samples/ scripts/ --include="*.gd"
```

PR マージ後に GDScript 側が壊れているとエディタ起動時にエラーになるため、
ビルド後に `./dev.sh edit mystery` 等でエディタを起動し、エラーがないことを確認すること。

参考: `docs/basic_game_karakuri.md`

## 4. 進捗の一次情報
- 進捗/チェック状態: `TASK.md` を一次情報とする
- デモ計画: `docs/demo_plan.md`
- Adventure(Mystery)設計: `docs/mystery_design.md`

## 5. 実行/ビルド（macOS想定）
- ビルド: `./dev.sh build`
- 実行: `./dev.sh run <demo>`
- エディタ: `./dev.sh edit <demo>`

## 6. ローカライズ（英/日切替）
- 表示文字列は原則 `tr("key")` 経由（直書き禁止の方向）
- C++側は `TranslationServer.translate()` を利用して翻訳キーから解決する
- 翻訳キー/CSVはデモごとに管理（例: Mystery は `samples/mystery/translations/mystery_translations.csv`）

## 7. スモークテスト（`karakuri_scenario_smoke.gd`）

### 実行方法
```bash
godot --headless --script samples/mystery/scripts/karakuri_scenario_smoke.gd
```

## 8. ツール仕様・スクリプト作成に関するルール
- 新しい機能や作業自動化のために Python スクリプトやシェルスクリプトを作成する場合は、**必ずコード内に「何をするスクリプトか」「各処理の意図」を示すコメント（日本語）を記述する**こと。
- これにより、後から人間や他のAIエージェントが見たときに、処理内容と目的をすぐに理解できるようにする。
- 汎用的なツールは可能な限り `scripts/tools/` などの共有ディレクトリに作成し、使い捨てではなく再利用可能な設計を心がけること。

### 方針（2026-02-20 決定）

| 状況 | 必須度 |
|---|---|
| `KarakuriScenarioRunner` の C++ 変更を含む PR | **必須**（省略不可） |
| Mystery シナリオ YAML のみ変更した PR | **任意推奨** |
| GDScript/シーンのみ変更した PR | 任意 |
| Karakuri 以外（rhythm / roguelike 等）の変更 | 任意 |

- CI（GitHub Actions 等）が整備されるまでは**手動実行**とし、PR 本文の `Test` 欄に実行結果を記載する。
- CI 化する際は `./dev.sh smoke` コマンドをエントリポイントにして `.github/workflows/smoke.yml` から呼ぶ想定。

