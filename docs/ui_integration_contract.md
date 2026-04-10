# UI Integration Contract

**ステータス**: draft v0.1  
**作成日**: 2026-04-07  
**目的**: UI line を `godot_cxx_2d_game_template` に接続するための、最初の具体 integration contract を定義する。

---

## 1. 目的

本ドキュメントは、UI line を主軸 repo と協調させるため、
入力・出力・asset_id・metadata・status・配置・失敗時の扱いを固定する。

目的は以下。

1. UI line の入出力契約を明文化する
2. 主軸 repo 側で UI asset を統合可能な単位にする
3. `asset_contracts.md` と `ui_pipeline_responsibility.md` の内容を UI 向けに具体化する
4. 今後の UI line task を契約駆動で進められるようにする

---

## 2. 前提

- UI line は別 repo / 別ラインで実装される
- UI line は CLI または同等の明確な実行インターフェースを持つ
- 主軸 repo は UI line の内部実装を持たず、契約と統合先だけを保持する
- 最初の接続対象は静的 UI asset を優先する

---

## 3. 適用範囲

本 contract は、以下の UI asset に適用する。

- button
- panel
- dialog frame
- popup frame
- badge
- icon
- banner
- result UI の静的構成要素
- title / menu / HUD 系の静的パーツ

### 対象外

- UI animation 詳細
- shader / effect の詳細
- runtime layout 自動調整ロジック
- input handling
- 画面遷移制御
- UI line repo 内部の compiler / renderer 実装詳細

---

## 4. 入力契約

UI line へ渡す入力は、原則として **spec file + execution parameter** とする。

### 4-1. 最低限必要な入力

| 項目 | 必須 | 内容 |
|:---|:---:|:---|
| `asset_id` | 必須 | 一意な UI asset ID |
| `asset_type` | 必須 | `ui` |
| `ui_kind` | 必須 | `button` / `panel` / `dialog_frame` / `icon` など |
| `usage_scope` | 必須 | 使用画面または使用機能 |
| `variant` | 任意 | `primary` / `secondary` / `result` / `warning` など |
| `source_spec` | 必須 | UI spec ファイルへの参照 |
| `requested_outputs` | 必須 | `svg` / `png` / `pdf` のいずれか |
| `integration_target` | 必須 | 主軸 repo 側の利用先識別子 |
| `notes` | 任意 | 補足情報 |

### 4-2. 推奨形式

- spec: JSON または YAML
- 実行: CLI
- 出力指定: フラグまたは config file

### 4-3. `integration_target` の最小ルール

- `integration_map.md` の接続レイヤ・用途分類に対応づける
- 例: `title/menu`, `result/reward_panel`, `hud/combat`
- 主軸 repo 側には識別子のみ残し、UI line 側の内部 path は持ち込まない

---

## 5. 出力契約

UI line は、少なくとも以下を返せることを望ましい条件とする。

### 5-1. 生成物本体

- `.svg`
- `.png`
- `.pdf`（必要時）

### 5-2. metadata

`asset_contracts.md` と整合するため、少なくとも以下を含む。

| 項目 | 必須 | 内容 |
|:---|:---:|:---|
| `asset_id` | 必須 | 一意な UI asset ID |
| `asset_type` | 必須 | `ui` |
| `source_spec` | 推奨 | spec 参照 |
| `generator_name` | 推奨 | UI line 名 |
| `generator_version` | 推奨 | 生成ラインの版 |
| `model_or_backend` | 推奨 | 生成 backend 情報 |
| `created_at` | 必須 | 生成日時 |
| `status` | 必須 | `draft` / `generated` / `reviewed` / `approved` / `integrated` / `deprecated` |
| `reviewed_by` | 任意 | review 実施者 |
| `approved_by` | 任意 | 採用承認者 |
| `usage_scope` | 推奨 | 使用画面 / 用途 |
| `notes` | 任意 | 補足 |

### 5-3. report

少なくとも以下を返す。

- 実行結果
- 失敗理由（失敗時）
- 生成物一覧
- Human Gate の前段で見るべき注意点

---

## 6. Asset ID 契約

UI asset は `asset_contracts.md` に従い、以下形式を推奨する。

```text
ui/<group>/<name_or_variant>
```

例:

- `ui/button/primary_001`
- `ui/result/reward_panel_a`
- `ui/dialog/frame_standard`
- `ui/title/logo_badge_v1`

### 6-1. ルール

- `ui/` を先頭に置く
- `group` は画面または用途カテゴリで切る
- 同一用途の差分は suffix で表現する
- rename は極力避ける

---

## 7. Status 契約

UI asset の status は以下を用いる。

- `draft`
- `generated`
- `reviewed`
- `approved`
- `integrated`
- `deprecated`

### 7-1. 原則

- `generated` のまま統合しない
- `approved` 前に少なくとも 1 回は AG-2 以上を通す
- `integrated` は実際に主軸 repo 側の利用先が定まってから付与する

---

## 8. 配置契約

### 8-1. UI line repo 側

- 実験出力
- 一時生成物
- 比較画像
- 詳細ログ
- compiler / renderer 出力

は UI line repo 側に保持する。

### 8-2. 主軸 repo 側

主軸 repo 側には、以下のみを保持する。

- integration 用の contract / policy / checklist
- 採用対象 asset の記録
- `asset_id`
- 最終利用先
- 判定履歴
- 必要最小限の report

### 8-3. 原則

- タイムスタンプ付き作業生成物は主軸 repo に持ち込まない
- 採用済み stable asset のみ統合候補とする
- 生成中間物を管理対象に置く場合は `scenarios/generated/` 方針に従う

---

## 9. Human Gate 接続契約

UI asset は `human_gate_asset_policy.md` の UI 観点を適用する。

### AG-1

- 使用画面
- 用途
- レイアウト前提
- variant

### AG-2

- 明らかな崩れ
- 可読不能
- 余白破綻
- 世界観逸脱

### AG-3

- 視認性
- 可読性
- 一貫性
- 利用耐性

### AG-4

- 実画面統合時の違和感
- 他 UI との整合
- 利用可否

---

## 10. 失敗時契約

UI line が失敗した場合、最低限以下を返す。

- `asset_id`
- 失敗した入力 spec
- 失敗工程
- 再現可否
- `REWORK` か `STOP` かの推奨
- 次に人間が確認すべき点

### 10-1. 原則

- 主軸 repo 側には、失敗理由の要約のみ残す
- 詳細ログ全文は UI line repo 側に保持する

---

## 11. 主軸 repo に残す最小記録

各 UI integration task について、主軸 repo に残すものは以下。

- task 名
- `asset_id`
- `ui_kind`
- `usage_scope`
- Gate ごとの判定
- 最終 status
- 承認者
- integration target
- `REWORK` / `STOP` 理由
- report 参照先

---

## 12. 完了条件

1. UI line の入力と出力が定義されている
2. `asset_id` / metadata / status が UI 向けに具体化されている
3. 配置契約が定義されている
4. Human Gate と失敗時契約が定義されている
5. 次タスク（UI Human Gate checklist）へ進める前提が揃っている

---

## 13. Stop / 保留条件

以下の場合、本 contract は保留または再整理する。

1. UI line の出力形式が未確定
2. CLI または同等の実行インターフェースが未定
3. `asset_id` 規則が未確定
4. 主軸 repo に UI compiler / renderer 詳細を書き込み始めている
5. 最初の integration target が複数候補のままで絞れない

---

## 14. 次アクション

1. `IB-03 UI line human gate checklist` を起票する
2. `IB-04 UI line first integration scenario` を起票する
3. UI line の最初の対象 asset を 1 件に絞る
4. 必要なら `asset_id_registry.md` へ UI 例を追加する

---

## 15. 参照

- [`asset_contracts.md`](asset_contracts.md)
- [`integration_map.md`](integration_map.md)
- [`human_gate_asset_policy.md`](human_gate_asset_policy.md)
- [`integration_backlog.md`](integration_backlog.md)
- [`ui_pipeline_responsibility.md`](ui_pipeline_responsibility.md)
