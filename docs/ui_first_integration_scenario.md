# UI First Integration Scenario

**ステータス**: draft v0.1  
**作成日**: 2026-04-08  
**目的**: UI line を主軸 repo に接続する最初の具体シナリオとして、対象画面・UI 種別・想定 asset・Human Gate・統合手順を 1 件に絞って定義する。

---

## 1. 目的

本ドキュメントは、UI line を最初に主軸 repo と接続する具体対象を 1 件に絞り、
以後の UI integration task の基準ケースを固定する。

目的は以下。

1. 最初の UI integration 対象を 1 ケースに限定する
2. `ui_pipeline_responsibility.md` / `ui_integration_contract.md` / `ui_human_gate_checklist.md` を実運用へ接続する
3. Human Gate の運用と証跡記録を、UI line の実例で確認できるようにする
4. 後続の UI asset integration を同じ型で増やせるようにする

---

## 2. 候補比較

初期候補は以下。

| 候補 | 概要 | 利点 | 懸念 |
|:---|:---|:---|:---|
| A | タイトル画面の primary button | 最小単位で扱いやすい。button は asset_id, variant, Human Gate の確認がしやすい | タイトル画面全体の情報設計までは見えない |
| B | 結果画面の報酬パネル | result UI として統合時の利用先が明確 | panel 系は情報量が増え、最初のケースとしてはやや重い |
| C | ダイアログ UI の最小セット | dialog frame / confirm button など、UI 群として見やすい | 最初から複数 asset が絡み、1 ケースとして重い |

### 判断

最初の統合対象は **A: タイトル画面の primary button** とする。

理由:

1. 最小単位で scope を限定しやすい
2. `asset_id` / `variant` / `usage_scope` の契約を確認しやすい
3. AG-1〜AG-4 の Human Gate を短い流れで検証しやすい
4. 失敗時の切り分けが容易

---

## 3. 対象シナリオ

### 3-1. 画面

- タイトル画面

### 3-2. UI 種別

- primary button

### 3-3. 用途

- ゲーム開始、またはメイン導線の primary action

### 3-4. 想定ラベル

- `START`
- `GAME START`
- 日本語環境では `はじめる`

※ 最終ラベルは後続 task で 1 件に確定してよい。本ドキュメントでは「primary action button」であることを優先定義する。

---

## 4. 想定 Asset 定義

### 4-1. 想定 `asset_id`

```text
ui/title/primary_button_001
```

### 4-2. 推奨 metadata

- `asset_id`: `ui/title/primary_button_001`
- `asset_type`: `ui`
- `ui_kind`: `button`
- `usage_scope`: `title_screen_primary_action`
- `variant`: `primary`
- `integration_target`: `title_screen/main_action_button`
- `status`: `draft` → `generated` → `reviewed` → `approved` → `integrated`

### 4-3. 出力物

- `.svg`
- `.png`
- metadata
- 必要なら `.pdf`

---

## 5. 統合先

### 5-1. 主軸 repo 側の integration target

```text
title_screen/main_action_button
```

### 5-2. 利用前提

- タイトル画面で最も目立つ primary action
- 他の secondary button より強い視認性を持つ
- UI line の最初の integration モデルケースとして利用する

---

## 6. Human Gate 適用方針

本シナリオでは、`ui_human_gate_checklist.md` をそのまま適用する。

### AG-1

確認すること:

- タイトル画面用であること
- primary button であること
- `asset_id` と `usage_scope` が明確であること
- サイズ前提と `integration_target` が定義されていること

### AG-2

確認すること:

- 文字が読めるか
- 余白や押下領域が不自然でないか
- world view から逸脱していないか
- 出力形式と metadata が揃っているか

### AG-3

確認すること:

- 視認性
- 可読性
- primary button としての強さ
- タイトル画面上での一貫性

### AG-4

確認すること:

- タイトル画面に仮配置したとき違和感がないか
- 他 UI と並んでも主導線として成立するか
- `approved` に進めるか

---

## 7. 成功条件

このシナリオが成立したとみなす条件は以下。

1. 対象 UI が 1 件に絞られている
2. `asset_id` / `ui_kind` / `usage_scope` / `integration_target` が定義されている
3. 出力物が明確である
4. Human Gate の適用先が明確である
5. 次タスクで UI line task execution contract を起票できる

---

## 8. 保留条件 / Stop 条件

以下の場合、本シナリオは保留または再整理する。

1. タイトル画面の primary action 自体が未確定
2. button ではなく panel や dialog を最初の対象にした方がよい合理的理由が出た
3. `asset_id` / `integration_target` の命名規則が未確定
4. 主軸 repo が UI line の内部デザイン詳細まで持とうとしている
5. 最初の対象を 1 件に絞れず、複数 UI を一括導入しようとしている

---

## 9. 次アクション

1. `IB-05 UI line task execution contract template` を起票する
2. `ui/title/primary_button_001` を対象にした最初の UI task 契約を作る
3. 必要なら `asset_id_registry.md` に UI 例を追加する
4. UI line repo 側でこのケースに対応する spec 形式を確認する

---

## 10. 判断メモ

- 最初の integration は最小成功例を優先する
- button は panel や dialog より scope が狭く、Gate 運用の確認に向く
- 最初から画面全体や複数 UI 群を扱わない
- このケースが通ったあとに result panel や dialog 系へ広げる

---

## 11. 参照

- [`integration_backlog.md`](integration_backlog.md)
- [`ui_pipeline_responsibility.md`](ui_pipeline_responsibility.md)
- [`ui_integration_contract.md`](ui_integration_contract.md)
- [`ui_human_gate_checklist.md`](ui_human_gate_checklist.md)
- [`asset_contracts.md`](asset_contracts.md)
