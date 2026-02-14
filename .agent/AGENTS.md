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

## 3. Basic Game Karakuri（共通Utility Framework）
全デモ/全ゲームで使い回す共通機構（ローカライズ、シーン遷移、セーブ、デバッグ等）の枠組みを **Basic Game Karakuri** と呼びます。

- 配置: `src/karakuri/`
- 禁止: デモ固有ロジックを Karakuri に入れない
- Doxygen必須:
  - `src/karakuri/**` 配下の C++（`.h/.hpp/.cpp`）は、公開APIに **必ず Doxygen 形式のコメント**を書く
  - 例: `/** ... */`, `/// ...`, `@param`, `@return`

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

