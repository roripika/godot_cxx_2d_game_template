# UI Human Gate Checklist

**ステータス**: draft v0.1  
**作成日**: 2026-04-08  
**目的**: UI line を主軸 repo に接続する際の、UI 向け AG-1〜AG-4 の具体チェック項目と判定記録形式を定義する。

---

## 1. 目的

本ドキュメントは、`human_gate_asset_policy.md` の UI 観点を、
UI integration 実務で使える checklist に具体化する。

目的は以下。

1. UI asset の Gate 判定を人によってぶらさせない
2. `PASS` / `REWORK` / `STOP` の基準を UI 向けに具体化する
3. 主軸 repo に残す証跡を最小限で固定する
4. UI line の最初の integration scenario に備える

---

## 2. 適用範囲

本 checklist は以下の UI asset に適用する。

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
- shader / effect の詳細
- runtime layout 調整
- 実装コードとしての input handling
- 画面遷移制御

---

## 3. 共通判定値

各 Gate の判定は以下のいずれかとする。

- `PASS`
- `REWORK`
- `STOP`

### 原則

- `PASS`: 次工程へ進めてよい
- `REWORK`: 方向性は合っているが修正が必要
- `STOP`: 要件不明、責務外、用途不整合などで進めるべきでない

---

## 4. AG-1 生成前確認

**目的**: 入力仕様・用途・配置前提が妥当かを確認する。

### チェック項目

- 使用画面が明確か
- UI 種別（button / panel / icon など）が明確か
- 使用目的が明確か
- 想定解像度またはサイズ前提が明確か
- variant（primary / secondary / warning など）が明確か
- `asset_id` が命名規則に従っているか
- `usage_scope` が具体的か
- `integration_target` が定義されているか

### PASS の目安

- 用途、画面、サイズ前提、ID、利用先が揃っている

### REWORK の目安

- 用途は分かるが、画面や variant が曖昧
- `asset_id` はあるが、命名規則に揺れがある

### STOP の目安

- 何に使う UI か不明
- 主軸 repo の責務外の指示が混ざっている
- 画面候補が複数あり、最初の統合対象が絞れていない

---

## 5. AG-2 生成直後レビュー

**目的**: 明らかな破綻・形式不備・用途逸脱を早期に除外する。

### チェック項目

- 文字が読めるか
- 余白が極端に崩れていないか
- ボタンやパネルとして不自然な形状になっていないか
- 透過や背景処理に明らかな破綻がないか
- 世界観から著しく逸脱していないか
- 出力形式（svg/png/pdf）が契約どおり揃っているか
- metadata が最低限揃っているか

### PASS の目安

- 明らかな破綻や形式不備がなく、次の品質確認に進める

### REWORK の目安

- 方向性は合っているが、余白・可読性・整形が弱い
- 出力形式や metadata の一部が不足している

### STOP の目安

- UI として用途を満たしていない
- 出力物が契約と大きく違う
- 形式不備が多く、個別修正より生成条件見直しが必要

---

## 6. AG-3 品質確認

**目的**: UI としての品質、一貫性、利用耐性を確認する。

### チェック項目

- 視認性が十分か
- 可読性が十分か
- 押下対象として自然か
- 同一画面内で並べたとき一貫性があるか
- variant 差分が意図どおりか
- サイズ変更や配置変更にある程度耐えるか
- 他 UI asset と比較して極端に浮いていないか

### PASS の目安

- 実画面へ持ち込む前提の品質がある
- UI 群の中で違和感が少ない

### REWORK の目安

- 単体では使えそうだが、既存 UI 群と並べると差が大きい
- 可読性や視認性に改善余地がある

### STOP の目安

- 世界観や表現方針が現行 UI 方針と衝突している
- 差分が大きすぎて、同一系統として扱えない

---

## 7. AG-4 統合前確認

**目的**: 実際のゲーム画面に統合可能かを確認する。

### チェック項目

- 想定画面に置いて違和感がないか
- 他 UI と並べても破綻しないか
- クリック / タップ対象として使えそうか
- 情報の優先順位が適切か
- 主軸 repo 側で記録すべき `asset_id` / `integration_target` / `status` が揃っているか
- `approved` に進める根拠があるか

### PASS の目安

- 実ゲーム統合に進めてよい
- 他 UI と合わせても大きな違和感がない

### REWORK の目安

- 単体品質は足りるが、実画面に置くと違和感がある
- 配置前提や variant 調整が必要

### STOP の目安

- 統合先画面自体が未確定
- UI 方針や責務分離に反する
- 別ラインの問題を UI asset 側で吸収しようとしている

---

## 8. 証跡として残す最小項目

UI task ごとに、主軸 repo 側へ最低限残す項目は以下。

- task 名
- `asset_id`
- `ui_kind`
- `usage_scope`
- `integration_target`
- AG-1 判定
- AG-2 判定
- AG-3 判定
- AG-4 判定
- 最終 status
- reviewer / approver
- `REWORK` / `STOP` の理由
- report 参照先

---

## 9. 証跡テンプレート（最小）

```yaml
task_name: <task_name>
asset_id: <asset_id>
ui_kind: <ui_kind>
usage_scope: <usage_scope>
integration_target: <integration_target>
gate_results:
  AG-1: PASS|REWORK|STOP
  AG-2: PASS|REWORK|STOP
  AG-3: PASS|REWORK|STOP
  AG-4: PASS|REWORK|STOP
final_status: draft|generated|reviewed|approved|integrated|deprecated
reviewer: <name_or_id>
approver: <name_or_id>
notes: <notes>
report_ref: <path>
```

---

## 10. 主軸 repo に残さないもの

- UI line 内部の大量比較画像
- renderer の詳細ログ全文
- prompt 試行の全履歴
- backend ごとのベンチマーク詳細
- 細かい中間出力の全部

---

## 11. 完了条件

1. UI 向け AG-1〜AG-4 のチェック項目が定義されている
2. PASS / REWORK / STOP の目安が UI 用に具体化されている
3. 主軸 repo に残す証跡項目が固定されている
4. 次タスク（first integration scenario）へ進める前提が揃っている

---

## 12. Stop / 保留条件

以下の場合、本 checklist は保留または再整理する。

1. UI の最初の統合対象が未確定
2. `asset_id` 契約が未確定
3. UI line の出力形式が未確定
4. 主軸 repo と UI line repo の責務境界が再度曖昧になっている
5. Gate 判定の対象が UI asset ではなく runtime 実装にずれている

---

## 13. 次アクション

1. `IB-04 UI line first integration scenario` を起票する
2. 最初に接続する画面・UI 種別・asset_id を 1 件に絞る
3. 必要に応じて `asset_id_registry.md` へ UI 例を追加する

---

## 14. 参照

- [`human_gate_asset_policy.md`](human_gate_asset_policy.md)
- [`asset_contracts.md`](asset_contracts.md)
- [`ui_pipeline_responsibility.md`](ui_pipeline_responsibility.md)
- [`ui_integration_contract.md`](ui_integration_contract.md)
- [`integration_backlog.md`](integration_backlog.md)
