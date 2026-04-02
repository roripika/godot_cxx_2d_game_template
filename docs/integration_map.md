# Integration Map

**ステータス**: draft v0.1  
**作成日**: 2026-04-03  
**目的**: `godot_cxx_2d_game_template` を主軸として、各アセットラインがどのゲーム機能・画面・実装層と接続するかを整理する。

---

## 1. 目的

本ドキュメントは、複数のアセット生成ラインを
「どこでゲーム本体に統合するか」という観点で整理する。

目的は以下。

1. 各ラインの接続先を明確にする
2. 同じアセットが複数箇所でどう使われるかを把握する
3. 主軸 repo が統合ハブとして持つべき視点を固定する

---

## 2. 基本方針

- 各アセットラインは独立して生成・検証される
- 主軸 repo は最終統合先として依存関係を管理する
- 実装順は「ゲーム進行に近いもの」から優先する
- 統合時は asset contract に従う
- 各ライン内部の詳細実装はこの map には書かない

---

## 3. 接続レイヤ整理

| レイヤ | 内容 | 主な対象 |
|:---|:---|:---|
| L1 | ゲーム進行 / scenario / state | scenario generator, branching, event flow |
| L2 | UI / presentation | UI, HUD, windows, icons, banners |
| L3 | キャラクタ表現 | char image, expression, pose, motion |
| L4 | 演出 / feedback | fx, camera, hit effect, transition |
| L5 | 音響 | sfx, bgm, voice |
| L6 | 映像外部出力 | movie, trailer, tutorial video |

---

## 4. アセットラインごとの接続先

| asset line | 主な接続先 | 主軸 repo 側で管理するもの | 主な review 観点 |
|:---|:---|:---|:---|
| scenario generator | scene/task/state flow | scenario YAML, HG, validation, regression | 進行破綻、条件不足、HG 通過可否 |
| UI pipeline | HUD / menu / popup / result | asset ID, screen 紐付け, integration path | 視認性、可読性、操作導線 |
| SFX pipeline | UI操作音 / combat hit / notify | 用途分類、再生ポイント ID | ノイズ、用途適合、長さ |
| BGM pipeline | title / field / battle / result | scene 紐付け、loop運用 | 世界観、一貫性、ループ品質 |
| Voice pipeline | dialogue / system voice / battle call | line ID, speaker ID, subtitle 紐付け | 話者一貫性、感情、明瞭性 |
| Character image pipeline | portrait / standing / card art | character ID, variant ID | 世界観、一貫性、差分整合 |
| Motion pipeline | idle / attack / reaction / loop | motion ID, attach先, integration先 | 接続自然さ、命名、再利用性 |
| FX pipeline | hit / slash / aura / UI flash | effect ID, trigger point | 見やすさ、尺、過剰演出の有無 |
| Movie pipeline | opening / ending / ad / tutorial | movie ID, scene usage | 尺、解像度、音同期、用途一致 |

---

## 5. ゲーム機能から見た依存関係

### 5-1. バトル系

依存しやすいライン:

- scenario generator
- UI pipeline
- SFX pipeline
- BGM pipeline
- character image pipeline
- motion pipeline
- FX pipeline
- voice pipeline

### 5-2. 会話 / ADV 系

依存しやすいライン:

- scenario generator
- UI pipeline
- character image pipeline
- voice pipeline
- BGM pipeline

### 5-3. 結果画面 / 報酬画面

依存しやすいライン:

- UI pipeline
- SFX pipeline
- BGM pipeline
- FX pipeline
- movie pipeline（必要なら）

### 5-4. タイトル / チュートリアル

依存しやすいライン:

- UI pipeline
- BGM pipeline
- voice pipeline
- movie pipeline

---

## 6. 統合優先度の考え方

統合優先度は、主に以下で決める。

1. ゲーム進行への近さ
2. 他ライン依存の強さ
3. 人間レビュー負荷
4. 差し替え頻度
5. 実装コスト

### 現時点の推奨順

1. scenario generator
2. UI pipeline
3. SFX / BGM
4. character image pipeline
5. FX pipeline
6. motion pipeline
7. voice pipeline
8. movie pipeline

---

## 7. 最初の連携モデル

最初の具体連携モデルとして、UI line を基準例とする。

理由:

- 入出力が比較的明確
- CLI 化しやすい
- 画面単位で統合しやすい
- asset contract を当てはめやすい

その後、audio line を同じ型で追加する。

---

## 8. 主軸 repo に必要な統合ドキュメント

本 map を踏まえ、主軸 repo に最低限必要なのは以下。

- `asset_pipeline_overview.md`
- `asset_contracts.md`
- `integration_map.md`

必要に応じて追加:

- `human_gate_asset_policy.md`
- `asset_id_registry.md`
- `integration_backlog.md`

---

## 9. 次アクション

1. UI pipeline を最初の接続例として正式化する
2. audio pipeline の責務表を追加する
3. character / motion / fx / movie を同型テンプレートで整理する
4. 将来的には integration backlog を別文書で持つ

---

## 10. 判断メモ

- 主軸 repo は「統合地点」を示す
- 各ライン repo は「生成の詳細」を持つ
- 依存関係が多いものほど、主軸 repo 側で先に境界を固定する
- UI と audio は比較的早期に連携モデルを作りやすい
- motion / voice / movie は後続でもよいが、ID と metadata 契約だけは先に揃える
