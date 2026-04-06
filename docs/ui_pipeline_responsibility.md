# UI Pipeline Responsibility Note

**ステータス**: draft v0.1  
**作成日**: 2026-04-03  
**目的**: UI line を最初の asset integration 対象とするため、`godot_cxx_2d_game_template` と UI line repo の責務境界を固定する。

---

## 1. 目的

本ドキュメントは、UI line を主軸 repo に接続する最初の具体例として、  
主軸 repo と UI line repo の責務を分離し、どこまでを各 repo が持つかを明文化する。

目的は以下。

1. UI line を最初の integration モデルとして固定する
2. 主軸 repo に置くべき情報と、UI line repo 側に残すべき情報を切り分ける
3. Human Gate、asset contract、integration map を UI に適用する前提を整える
4. 主軸 repo に UI 生成実装の詳細を持ち込まない

---

## 2. 前提

- 主軸 repo は `godot_cxx_2d_game_template` とする
- UI line は別 repo / 別ラインで実装・検証する
- UI line は CLI または同等の明確な入出力契約を持つことを前提とする
- 本ドキュメントは UI の内部実装仕様ではなく、責務境界を定義する

---

## 3. UI line の対象

本ドキュメントで対象とする UI asset は以下。

- button
- panel
- dialog frame
- popup frame
- HUD element
- icon
- badge
- banner
- result UI の最小構成
- title / menu 系の静的 UI パーツ

---

## 4. 対象外

本ドキュメントでは以下を対象外とする。

- UI animation の詳細実装
- UI 側の shader / effect 実装詳細
- 画面全体レイアウトの自動決定アルゴリズム
- UI line repo 内の compiler / renderer 実装
- UI line 専用 prompt 詳細
- Live UI behavior のコード実装
- 実ゲーム内の input handling 実装

---

## 5. 主軸 repo の責務

`godot_cxx_2d_game_template` 側が持つ責務は以下。

### 5-1. 統合責務

- UI asset の利用先画面を管理する
- UI asset の `asset_id` と画面・用途の紐付けを管理する
- UI line をどの順序で integration するかを backlog 管理する

### 5-2. 契約責務

- `asset_contracts.md` に従う共通契約を保持する
- `integration_map.md` に従う接続先を保持する
- `human_gate_asset_policy.md` に従う Human Gate 観点を保持する

### 5-3. 記録責務

- UI integration task の status
- Gate ごとの判定
- 最終統合先
- `REWORK` / `STOP` の理由
- 採用可否の履歴

### 5-4. 主軸 repo に置く具体物

- UI responsibility note
- UI integration contract
- UI Human Gate checklist
- UI first integration scenario
- integration backlog
- 必要最小限の report / decision log

---

## 6. UI line repo の責務

UI line repo 側が持つ責務は以下。

### 6-1. 生成責務

- UI spec の定義
- schema / validator
- compiler / renderer
- SVG / PNG / PDF などの出力処理
- metadata 生成

### 6-2. 品質責務

- UI 生成ライン専用のテスト
- 出力物検証
- フォーマット検証
- ライン専用 Human Gate の詳細チェック

### 6-3. 実験責務

- prompt 調整
- renderer backend 比較
- compiler 詳細設計
- 生成パラメータ実験
- 差分比較の詳細ログ

### 6-4. UI line repo に置く具体物

- schema
- validator
- compiler
- renderer
- examples
- tests
- backend 切替
- ライン専用 docs

---

## 7. 責務境界表

| 項目 | 主軸 repo | UI line repo |
|:---|:---:|:---:|
| UI の利用先画面 | ✅ |  |
| asset_id 規則 | ✅ | 参照 |
| status 契約 | ✅ | 参照 |
| Human Gate の共通枠 | ✅ | 参照 |
| UI spec schema |  | ✅ |
| JSON 検証 |  | ✅ |
| JSON → SVG コンパイル |  | ✅ |
| PNG/PDF 書き出し |  | ✅ |
| UI 生成テスト |  | ✅ |
| 最終統合判断 | ✅ |  |
| 採用履歴 / decision log | ✅ |  |
| 実験ログ詳細 |  | ✅ |

---

## 8. Human Gate の責務分担

### 主軸 repo 側

- AG-1〜AG-4 の枠定義
- 最終判定の記録
- 採用 / 保留 / 差し戻しの履歴保持
- 統合対象画面との整合確認

### UI line repo 側

- UI 固有の詳細チェック
- 出力品質の個別判断
- renderer / compiler の挙動確認
- 実験差分の検証

---

## 9. 主軸 repo に残す最小記録

UI task ごとに、主軸 repo 側に最低限残すものは以下。

- task 名
- 対象画面
- 対象 UI 種別
- 対象 `asset_id`
- Gate ごとの判定
- 最終 status
- 承認者
- integration 先
- `REWORK` / `STOP` の理由

---

## 10. 主軸 repo に持ち込まないもの

以下は UI line repo 側に留める。

- SVG compiler の実装詳細
- renderer backend 比較結果の全文
- 実験用 prompt 群
- 大量の比較生成物
- 出力物ごとの細かい中間ログ
- ライン専用の detailed regression 設計

---

## 11. 初期 integration の前提

UI line の最初の integration は、以下を満たしてから進める。

1. `asset_contracts.md` と整合している
2. `human_gate_asset_policy.md` の UI 観点を適用できる
3. `ui_integration_contract.md` が存在する
4. 最初に接続する UI シナリオが 1 件に絞られている
5. 主軸 repo に残す証跡形式が定まっている

---

## 12. 完了条件

1. 主軸 repo の責務と UI line repo の責務が分離されている
2. 主軸 repo に置くもの / 置かないものが明記されている
3. Human Gate の責務分担が整理されている
4. 次タスク（UI integration contract）へ進める前提が揃っている

---

## 13. Stop / 保留条件

以下の場合、この責務定義は保留または再整理する。

1. UI line repo 側の出力形式が未確定
2. asset_id 規則が未確定
3. 主軸 repo 側で UI compiler 詳細まで持とうとしている
4. 最初の integration 対象が複数候補のままで絞れない
5. Human Gate の責務が主軸 repo と UI line repo で重複しすぎている

---

## 14. 次アクション

1. `IB-02 UI line integration contract` を起票する
2. `IB-03 UI line human gate checklist` を起票する
3. `IB-04 UI line first integration scenario` で最初の接続対象を 1 件に絞る

---

## 15. 参照

- [`asset_pipeline_overview.md`](asset_pipeline_overview.md)
- [`asset_contracts.md`](asset_contracts.md)
- [`integration_map.md`](integration_map.md)
- [`human_gate_asset_policy.md`](human_gate_asset_policy.md)
- [`integration_backlog.md`](integration_backlog.md)
