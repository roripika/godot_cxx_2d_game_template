# Asset Pipeline Overview

**ステータス**: draft v0.1  
**目的**: `godot_cxx_2d_game_template` を主軸として、各アセット生成ラインの責務・境界・連携契約を整理する。  
**前提**: 本リポジトリはゲーム進行ロジックと統合方針のハブであり、各アセットの生成実装詳細は原則として各専用リポジトリ側で管理する。

---

## 1. 基本方針

本プロジェクトでは、以下の考え方を採る。

1. ゲーム本体・進行ロジック・統合契約は `godot_cxx_2d_game_template` を主軸とする
2. UI / 音 / 画像 / モーション / エフェクト / 動画などの生成系は、原則として別ライン・別リポジトリで管理する
3. ただし完全分離ではなく、最終的に協調可能なように **ID / metadata / status / 出力契約** は共通化する
4. 主軸リポジトリには「全体俯瞰・境界・連携契約」を置き、各ラインの内部実装詳細は持ち込まない

---

## 2. 主軸リポジトリの役割

`godot_cxx_2d_game_template` が持つ役割は以下。

- ゲーム進行ロジック / scenario generator の基盤
- Human Gate / validation / regression の運用方針
- 各アセットラインとの連携契約
- 統合先としてのディレクトリ規約
- 実装優先度 / backlog / midpoint review などの司令塔ドキュメント

主軸 repo は **統治と統合** の場所であり、各アセット生成ラインの詳細実装を抱え込まない。

---

## 3. アセット種別ごとの分離方針

| 種別 | 主な生成物 | 推奨ライン | 主軸 repo に置くもの | 各ライン repo に置くもの |
|:---|:---|:---|:---|:---|
| シナリオ進行 | scenario YAML / review | 本 repo | backlog, HG, validation 方針, integration | generator 実装、fixture、回帰テスト |
| UI | SVG / PNG / PDF / UI metadata | 別 repo | UI 契約、出力先、ID 規則 | schema, validator, compiler, renderer |
| 効果音 | wav / ogg / cue metadata | 別 repo | SFX ID 規則、用途分類、統合先 | 生成 spec、書き出しロジック、品質検証 |
| BGM | wav / ogg / loop metadata | 別 repo | BGM ID、ループ運用、ステージ紐付け | 楽曲 spec、尺・ループ点管理、書き出し |
| ボイス | wav / ogg / line metadata | 別 repo | voice ID、話者ID、台本との対応 | 台本入力、音声生成、感情差分、検証 |
| キャラ画像 | PNG / PSD相当 / layered data | 別 repo | character asset ID、世界観契約 | prompt/spec、差分管理、出力変換 |
| キャラモーション | Spine / Live2D / animation clips | 別 repo | motion ID、接続ルール、統合先 | rig/clip 定義、生成/変換、検証 |
| エフェクト | spritesheet / particle config / movie | 別 repo | FX ID、用途分類、呼び出し契約 | 生成 spec、書き出し、短尺検証 |
| 動画 | mp4 / webm / metadata | 別 repo | movie ID、用途分類、統合先 | 絵コンテ入力、動画生成、エンコード |

---

## 4. 共通化すべき契約

アセット種別が分かれていても、以下は共通化する。

### 4-1. Asset ID

すべての生成物は一意な asset ID を持つ。

例:

- `ui/button/primary_001`
- `sfx/ui/click_001`
- `bgm/battle/theme_a`
- `voice/hero/line_012`
- `char/hero/base_pose_a`
- `motion/hero/attack_slash_01`
- `fx/slash/light_01`
- `movie/op/opening_v1`

### 4-2. Metadata

すべての生成物は、最低限以下の metadata を持つ。

- `asset_id`
- `asset_type`
- `source_spec`
- `generator_name`
- `generator_version`
- `model_or_backend`
- `created_at`
- `status`
- `reviewed_by` または `approved_by`
- `usage_scope`
- `notes`

### 4-3. Status

すべての生成物は共通の状態遷移を持つ。

- `draft`
- `generated`
- `reviewed`
- `approved`
- `integrated`
- `deprecated`

### 4-4. Human Gate

アセット種別ごとにレビュー観点は違っても、最低限次を共有する。

- 生成直後レビュー
- 品質確認
- 統合前確認
- 本採用判断

---

## 5. 連携方式の基本

各アセットラインは、可能な限り **CLI または明確な入出力契約** を持つ。

推奨:

- 入力: JSON / YAML / scriptable spec
- 実行: CLI またはバッチ処理
- 出力: 生成物 + metadata
- 検証: validator / smoke test / report

主軸 repo から見た各ラインは、まず **外部コンパイラ / 外部生成器** として扱う。

---

## 6. 主軸 repo に置くべきドキュメント

本 repo に置く候補は以下。

- `docs/asset_pipeline_overview.md`  
  全アセット種別の俯瞰表
- `docs/asset_contracts.md`  
  共通契約（ID / metadata / status / integration）
- `docs/integration_map.md`  
  どのゲーム機能がどのアセットラインに依存するか
- `docs/human_gate_asset_policy.md`  
  アセット種別ごとのレビュー境界

---

## 7. 各ライン repo に置くべきもの

各アセット専用 repo には、以下を置く。

- 生成 spec
- schema
- validator
- compiler / renderer / export 処理
- 生成物サンプル
- ライン専用テスト
- ライン専用 Human Gate
- 専用 backlog / roadmap

主軸 repo は各ラインの詳細実装仕様を持たない。

---

## 8. 現時点の推奨ライン

### 8-1. すでに成立しているライン

- scenario generator / validation / regression（本 repo）

### 8-2. 別ラインとして強く推奨

- UI asset pipeline
- audio asset pipeline
- character image pipeline
- motion pipeline
- effect pipeline
- video pipeline

---

## 9. 次アクション

1. 本 overview を主軸 repo に配置する
2. `docs/asset_contracts.md` を追加し、共通契約を固定する
3. UI asset pipeline との境界を最初の連携モデルとして確定する
4. audio / character / motion / effect / video を同じ型で段階追加する

---

## 10. 判断メモ

- 主軸 repo は「統合の司令塔」として機能する
- 各生成ラインは「専門ツール」として分離する
- 分離するが、ID / metadata / status / CLI 契約は共通化する
- 単一巨大 repo 化は避ける
- 最初から完全自動連携を目指さず、まずは境界定義と契約固定を優先する
