# Audio Pipeline Rollout Note

**ステータス**: draft v0.1  
**作成日**: 2026-04-09  
**目的**: UI line の次段として audio line を接続するため、SFX / BGM / voice の切り分け、初期対象、責務境界、文書展開方針を定義する。

---

## 1. 目的

本ドキュメントは、`godot_cxx_2d_game_template` における audio line の初期 rollout 方針を定義する。

目的は以下。

1. audio line を SFX / BGM / voice に分解し、初期統合範囲を固定する
2. 主軸 repo と audio line repo の責務境界を明文化する
3. UI line で確立した文書型を audio line へ転用する
4. 後続タスクを 1 task 1 deliverable で起票可能にする

---

## 2. 前提

- audio line は UI line と同様に別 repo / 別ラインで運用する
- 主軸 repo は audio 生成実装を持たず、契約・判定・統合記録を保持する
- 共通契約は `asset_contracts.md`、接続先は `integration_map.md`、Gate 観点は `human_gate_asset_policy.md` に従う
- 初期 rollout は「最小成功例」を優先する

---

## 3. 適用範囲

本 rollout note は以下に適用する。

- SFX pipeline
- BGM pipeline
- voice pipeline（定義のみ、初期導入は保留）

### 対象外

- DAW / 音声合成エンジンの内部設定
- ノイズ除去・ミキシングの実装詳細
- runtime オーディオ再生コードの実装変更
- 字幕システムや会話進行ロジックの実装変更

---

## 4. Audio 種別の切り分け

### 4-1. SFX

- 用途: UI 操作音 / hit / notify / feedback
- 主な統合先: UI 操作イベント、戦闘イベント
- 代表 `asset_id`: `sfx/ui/click_001`, `sfx/battle/hit_light_001`

### 4-2. BGM

- 用途: title / field / battle / result のシーン音楽
- 主な統合先: シーン遷移、画面状態
- 代表 `asset_id`: `bgm/title/theme_a`, `bgm/battle/theme_a`

### 4-3. voice

- 用途: dialogue / system voice / battle call
- 主な統合先: 台詞再生、字幕、話者管理
- 代表 `asset_id`: `voice/hero/line_012`

---

## 5. 最初に扱う範囲

最初の rollout では **SFX / BGM を対象** とし、voice は除外する。

### 5-1. SFX / BGM を先行する理由

1. `usage_scope` と `integration_target` が比較的固定しやすい
2. UI line 後の拡張として、契約項目をほぼ同型で適用できる
3. Gate 判定時の切り分け（用途適合 / 長さ / ループ品質）が明確

### 5-2. voice を初期 rollout から除外する理由

1. 話者ID・台本対応・字幕同期など、依存情報が多い
2. AG-3 / AG-4 で感情表現・台詞整合まで評価が必要でレビュー負荷が高い
3. 会話進行やローカライズ方針と強く結合し、初期段階では責務境界が崩れやすい
4. SFX / BGM と比べて最小成功例を作りにくい

---

## 6. 責務境界

### 6-1. 主軸 repo の責務

- audio asset の `asset_id` / `usage_scope` / `integration_target` を管理する
- status（`draft`〜`integrated`）と Gate 判定結果を記録する
- 採用可否、`REWORK` / `STOP` 理由、承認履歴を保持する
- audio line の統合順序を backlog で管理する

### 6-2. audio line repo の責務

- 入力 spec / 生成設定 / backend 運用
- 生成（wav/ogg など）と metadata 出力
- ライン専用テストと品質検証
- 詳細ログ・比較データ・実験履歴の管理

### 6-3. 境界原則

- 主軸 repo は統合契約と証跡に集中し、生成実装詳細は持ち込まない
- 詳細ログ全文・実験出力は audio line repo 側に保持する
- 主軸 repo には最小 report と採用判断だけを残す

---

## 7. UI line 文書型の転用方針

audio line は、UI line で確立した以下の文書型を同順序で転用する。

1. responsibility note
2. integration contract
3. human gate checklist
4. first integration scenario
5. task execution contract template

### 7-1. 転用時に置換する主要キー

- `ui_kind` → `audio_kind`（`sfx` / `bgm` / `voice`）
- UI 用 `integration_target` → 音再生ポイント識別子
- UI 観点（視認性/可読性）→ 音観点（ノイズ/長さ/ループ品質/明瞭性）

### 7-2. 維持する共通キー

- `asset_id`
- `usage_scope`
- `integration_target`
- status
- AG-1〜AG-4 判定

---

## 8. 初期 rollout 完了条件

1. SFX / BGM を対象とする境界が文書化されている
2. voice 除外理由が明文化されている
3. 主軸 repo と audio line repo の責務分離が明確
4. UI line 文書型の転用手順が示されている
5. 次タスク（audio の contract/checklist/scenario 起票）へ進める前提が揃っている

---

## 9. Stop / 保留条件

以下の場合、本 rollout は保留または再整理する。

1. SFX / BGM / voice の区分が運用上分離できない
2. `asset_id` 規則が未確定
3. audio line の出力形式（wav/ogg/metadata）が未確定
4. 主軸 repo 側で音声生成の内部実装まで扱い始めている
5. 初期対象が 1 ケースに絞れず、同時導入が前提になっている

---

## 10. 次アクション

1. `audio_integration_contract.md` を起票し、SFX / BGM の入出力契約を固定する
2. `audio_human_gate_checklist.md` を起票し、SFX / BGM 観点の AG-1〜AG-4 を具体化する
3. 最初の audio 対象を 1 件に絞る（例: `sfx/ui/click_001` または `bgm/title/theme_a`）
4. voice は SFX / BGM の最初の統合フロー確立後に別タスクで段階投入する

---

## 11. 参照

- [`asset_contracts.md`](asset_contracts.md)
- [`integration_map.md`](integration_map.md)
- [`human_gate_asset_policy.md`](human_gate_asset_policy.md)
- [`integration_backlog.md`](integration_backlog.md)
- [`ui_pipeline_responsibility.md`](ui_pipeline_responsibility.md)
- [`ui_integration_contract.md`](ui_integration_contract.md)
- [`ui_human_gate_checklist.md`](ui_human_gate_checklist.md)
- [`ui_first_integration_scenario.md`](ui_first_integration_scenario.md)
- [`ui_task_execution_contract_template.md`](ui_task_execution_contract_template.md)
