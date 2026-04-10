# Integration Backlog

**ステータス**: draft v0.1  
**作成日**: 2026-04-03  
**目的**: `godot_cxx_2d_game_template` を主軸として、各アセットラインをどの順で、どの条件で統合していくかを順序付きで管理する。

---

## 1. 目的

本ドキュメントは、asset pipeline を主軸 repo に接続していくための実行 backlog を定義する。

目的は以下。

1. 机上の方針を、順序付きの実行タスクへ落とす
2. 最初の連携対象を UI line に固定し、以降の型を作る
3. Human Gate / contract / integration の各文書を、実運用へ接続する
4. 各ラインを無秩序に増やさず、主軸 repo 側で統合順を管理する

---

## 2. 基本方針

- 最初の連携対象は UI line とする
- 1 タスクで広げすぎず、1 ラインずつ段階統合する
- 各ラインの内部実装は各 repo 側で進める
- 主軸 repo 側では、接続契約・Human Gate・統合順序を管理する
- `src/` やゲーム本体コードへ直接入る前に、docs と contract を固定する

---

## 3. 優先順（現時点）

1. UI pipeline
2. audio pipeline（SFX / BGM）
3. character image pipeline
4. FX pipeline
5. motion pipeline
6. voice pipeline
7. movie pipeline

理由:
- UI は入出力が比較的明確で、CLI 連携の最初のモデルにしやすい
- audio は UI に続いて用途分類が比較的固定しやすい
- motion / voice / movie はレビュー負荷と依存関係が大きいため後続

---

## 4. 順序付き実行バックログ

| ID | 優先度 | task | status | deliverable | completion criteria |
|:---|:---:|:---|:---:|:---|:---|
| IB-01 | P0 | UI line responsibility note | Done | `docs/ui_pipeline_responsibility.md` | UI line の責務・主軸 repo との境界・対象外が明文化されている |
| IB-02 | P0 | UI line integration contract | Done | `docs/ui_integration_contract.md` | UI line の入出力、asset_id、metadata、出力配置、連携方法が固定されている |
| IB-03 | P0 | UI line human gate checklist | Done | `docs/ui_human_gate_checklist.md` | UI 向け AG-1〜AG-4 の具体チェック項目が固定されている |
| IB-04 | P1 | UI line first integration scenario | Done | `docs/ui_first_integration_scenario.md` | 最初に接続する画面・UI 種別・成果物・統合手順が決まっている |
| IB-05 | P1 | UI line task execution contract template | Done | `docs/ui_task_execution_contract_template.md` | UI line 用の契約駆動テンプレートが固定されている |
| IB-06 | P1 | audio line rollout note | Done | `docs/audio_pipeline_rollout_note.md` | audio line を UI line の次に接続する責務表と順序が整理されている |

---

## 5. 各タスクの概要

### IB-01 UI line responsibility note

目的:
- UI line を最初の接続例とするため、主軸 repo と各 UI repo の責務境界を固定する

含める内容:
- 主軸 repo に置くもの
- UI line repo に置くもの
- 対象アセット
- 対象外
- Human Gate の責務分担

### IB-02 UI line integration contract

目的:
- UI line が主軸 repo と協調するための最初の具体 contract を作る

含める内容:
- 入力 spec
- 出力物（svg/png/pdf/metadata）
- asset_id 規則
- status
- integration path
- 失敗時の扱い

### IB-03 UI line human gate checklist

目的:
- `human_gate_asset_policy.md` の UI 部分を、実運用可能な checklist に落とす

含める内容:
- AG-1〜AG-4 ごとの確認項目
- PASS / REWORK / STOP の例
- 主軸 repo に残す証跡

### IB-04 UI line first integration scenario

目的:
- 最初に接続する UI ケースを 1 つに絞る

候補例:
- タイトル画面の primary button
- 結果画面の報酬パネル
- ダイアログ UI の最小セット

含める内容:
- 画面名
- UI 種別
- 想定 asset_id
- 生成ライン
- 統合先
- Human Gate

### IB-05 UI line task execution contract template

目的:
- UI line 専用の契約駆動タスク投入テンプレートを固定する

含める内容:
- scope
- out of scope
- completion criteria
- stop conditions
- output format

### IB-06 audio line rollout note

目的:
- UI line の次に audio line を入れる準備をする

含める内容:
- SFX / BGM / voice の切り分け
- 最初に扱うのは SFX/BGM か
- 主軸 repo 側で必要な責務表
- UI line の型を audio line にどう転用するか

---

## 6. 完了条件

本 backlog の初期完了条件は以下。

1. UI line 向けの責務表・integration contract・Human Gate checklist が揃っている
2. 最初の UI 具体接続シナリオが 1 件に絞られている
3. UI line 用の task execution contract template がある
4. audio line を次段として整理した責務メモがある

---

## 7. Stop / 保留条件

以下に該当する場合、integration backlog は保留または再整理する。

1. UI line 側 repo の責務が未確定
2. asset_id 契約が確定していない
3. status 契約の実運用方法が未定
4. Human Gate の証跡形式が未定
5. 主軸 repo に各ライン内部実装を書き込み始めて境界が崩れている
6. 最初の統合対象画面が複数候補のままで絞れない

---

## 8. 実行ポリシー

- UI line を最初のモデルケースとする
- 1 タスク 1 deliverable を原則とする
- docs 先行で境界を固定し、その後に実装連携へ進む
- 各ライン repo の内部詳細を主軸 repo に持ち込まない
- 抽象方針だけで止まらず、最終的に具体接続シナリオまで落とす

---

## 9. 次アクション

1. `ui/title/primary_button_001` を対象に、最初の UI task 契約を作る
2. `audio_integration_contract.md` を起票し、SFX / BGM の入出力契約を固定する
3. `asset_id_registry.md` の要否を判断し、必要なら UI/SFX/BGM の例を追加する

---

## 10. 参照

- [`asset_pipeline_overview.md`](asset_pipeline_overview.md)
- [`asset_contracts.md`](asset_contracts.md)
- [`integration_map.md`](integration_map.md)
- [`human_gate_asset_policy.md`](human_gate_asset_policy.md)
