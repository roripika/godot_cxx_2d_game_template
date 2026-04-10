# UI Task Execution Contract Template

**ステータス**: draft v0.1  
**作成日**: 2026-04-09  
**目的**: UI line 向けの task を契約駆動で投入するための、最小かつ再利用可能な実行契約テンプレートを定義する。

---

## 1. 目的

本ドキュメントは、UI asset integration task を
`godot_cxx_2d_game_template` 側で安全に起票するための contract template を定義する。

目的は以下。

1. UI line task を毎回同じ型で起票できるようにする
2. scope / out of scope / completion criteria / stop conditions を固定する
3. Human Gate と integration target を task 単位で追跡可能にする
4. 主軸 repo が UI line の内部実装詳細に踏み込みすぎないようにする

---

## 2. 適用範囲

本 template は以下の UI task に適用する。

- button
- panel
- dialog frame
- popup frame
- icon
- badge
- banner
- result UI の静的構成要素
- title / menu / HUD 系の静的 UI パーツ

### 対象外

- UI animation 詳細
- shader / effect 実装
- runtime layout engine
- input handling 実装
- UI compiler / renderer の内部改修
- 主軸 repo 側のゲームロジック変更

---

## 3. テンプレート本体

以下を task 起票時にコピーして使う。

```md
# UI TASK EXECUTION CONTRACT

## 0. Task Name
<task_name>

## 1. Objective
この task の目的を簡潔に書く。

例:
- `ui/title/primary_button_001` を生成し、title screen の main action button として利用可能な状態にする
- result screen 用 reward panel の最小 UI asset を生成し、Human Gate まで進める

## 2. Background
背景と前提を書く。

例:
- `ui_first_integration_scenario.md` で最初の対象が `ui/title/primary_button_001` に固定されている
- `ui_integration_contract.md` に従い、UI line は svg/png/metadata を返す
- `ui_human_gate_checklist.md` に従い AG-1〜AG-4 を適用する

## 3. Scope
今回やってよい範囲を書く。

例:
- 対象 `asset_id` の UI spec 作成または更新
- UI line 実行
- 生成物と metadata の取得
- Human Gate 用の report 作成
- 主軸 repo 側 docs / report の最小更新

## 4. Out of Scope
今回やってはいけない範囲を書く。

例:
- `src/` 配下の変更
- ゲーム本体コードの変更
- UI compiler / renderer の大規模改修
- runtime layout 実装
- 複数画面への一括展開
- 複数 asset の同時導入

## 5. Hard Constraints
必ず守る制約を書く。

例:
- 1 task 1 asset を原則とする
- `asset_id` は既定ルールに従う
- `ui_kind` / `usage_scope` / `integration_target` を必ず明記する
- Human Gate の証跡を残す
- 主軸 repo に UI line の内部詳細を持ち込まない
- 不明点が重大なら停止する

## 6. Inputs
参照すべき文書や spec を列挙する。

例:
- `asset_contracts.md`
- `human_gate_asset_policy.md`
- `ui_pipeline_responsibility.md`
- `ui_integration_contract.md`
- `ui_human_gate_checklist.md`
- `ui_first_integration_scenario.md`
- 対象 UI spec file

## 7. Expected Deliverables
最終成果物を書く。

例:
- 対象 UI asset（svg/png/metadata）
- Human Gate 用 report
- 主軸 repo 側の最小 decision log
- 必要なら README / backlog 更新

## 8. Work Steps
推奨する作業順を書く。

例:
1. 参照文書を読む
2. 対象 `asset_id` と `integration_target` を確認する
3. UI line 用 spec を準備する
4. UI line を実行する
5. 生成物を確認する
6. AG-1〜AG-4 の判定材料を report にまとめる
7. 主軸 repo 側の最小更新を行う

## 9. Completion Criteria
完了条件を機械的に近い形で書く。

例:
- 対象 `asset_id` が 1 件に固定されている
- svg/png/metadata が揃っている
- `usage_scope` / `integration_target` が明記されている
- AG-1〜AG-4 の判定結果を残している
- report に `PASS` / `REWORK` / `STOP` の根拠がある
- 変更差分が docs / report / UI asset task 範囲に収まっている

## 10. Stop Conditions
ここに当たったら止まる条件を書く。

例:
- 最初の対象 asset が 1 件に絞れない
- `asset_id` 契約が未確定
- UI line の出力形式が未確定
- UI compiler / renderer の内部改修が必要
- 主軸 repo に runtime 実装を入れないと成立しない
- Human Gate の判定材料が不足している

## 11. Output Format
report の見出しを固定する。

例:
- Summary
- Files Changed
- Asset ID
- UI Kind
- Usage Scope
- Integration Target
- Gate Results
- Result
- Risks / Open Issues
- Suggested Next Step

## 12. Execution Policy
実行姿勢を書く。

例:
- まず読む、次に最小変更で進める
- 1 task 1 asset を守る
- 推測で scope を広げない
- 主軸 repo の責務を超えない
- Human Gate 証跡を優先する
- 不要な整形や命名変更をしない

## 13. Instruction to Executor
実行命令を書く。

例:
上記制約を厳守し、この UI task を完了してください。
対象 `asset_id` を 1 件に固定し、必要最小限の差分で生成・確認・記録を進めてください。
禁止範囲に触れる必要がある場合は作業を中断し、理由を report に記載してください。
```

---

## 4. この template で必須にする項目

UI task では、最低限以下を必須とする。

- `asset_id`
- `ui_kind`
- `usage_scope`
- `integration_target`
- Gate 結果
- 最終 status
- report 参照先

---

## 5. 推奨する最初の具体適用

最初の適用対象は、`ui_first_integration_scenario.md` に従い、以下を想定する。

- `asset_id`: `ui/title/primary_button_001`
- `ui_kind`: `button`
- `usage_scope`: `title_screen_primary_action`
- `integration_target`: `title_screen/main_action_button`

---

## 6. 主軸 repo に残す最小証跡

各 UI task について、主軸 repo 側には最低限以下を残す。

- task 名
- `asset_id`
- `ui_kind`
- `usage_scope`
- `integration_target`
- AG-1〜AG-4 結果
- 最終 status
- reviewer / approver
- `REWORK` / `STOP` 理由
- report 参照先

---

## 7. 保留条件

以下に該当する場合、本 template の適用は保留または再整理する。

1. UI line の出力形式が未確定
2. 最初の対象 asset が 1 件に絞れていない
3. 主軸 repo 側で UI line 内部実装まで背負い始めている
4. Human Gate の証跡フォーマットが未確定
5. task が 1 asset を超えて広がっている

---

## 8. 次アクション

1. 本 template を使って `ui/title/primary_button_001` の最初の task 契約を起票する
2. 必要なら `asset_id_registry.md` を追加する
3. UI line 実行結果の最小 report テンプレートを整備する

---

## 9. 参照

- [`integration_backlog.md`](integration_backlog.md)
- [`ui_pipeline_responsibility.md`](ui_pipeline_responsibility.md)
- [`ui_integration_contract.md`](ui_integration_contract.md)
- [`ui_human_gate_checklist.md`](ui_human_gate_checklist.md)
- [`ui_first_integration_scenario.md`](ui_first_integration_scenario.md)
- [`asset_contracts.md`](asset_contracts.md)
