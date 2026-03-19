# Phase 2 Completion Summary

Date: 2026-03-19

## 1. Purpose

Phase 2 の目的は、Kernel v2.0 の上で AI が安全に Task + YAML を拡張できるようにするために、次の 3 点を成立させることだった。

- **2-A: validator 強化**
- **2-B: diagnostic / regression 整備**
- **2-C: event-driven module の最小導入（billiards_test）**

このフェーズでは、Generator や Template/Assist には進まず、あくまで **安全に次モジュールを追加できる最小 guard rail** を整えることを目的とした。

---

## 2. Phase 2 Outcome

Phase 2 は、以下の条件を満たしたため **完了** とする。

### Completed
- `src/core` を変更せずに Phase 2 差分を整理した
- validator guard rail を追加した
- corrupted scenario を使った reject test を追加した
- `mystery_test` を継続的な基準モジュールとして維持した
- `billiards_test` の最小骨格を fake event ベースで導入した
- `billiards_test` の fake regression として以下の 3 経路を確認した
  - clear
  - foul fail
  - continue
- WorldState key を `billiards_test:` namespace の個別フラグ中心へ整理した

### Explicitly not included
- Core convenience hook の追加
- `ActionRegistry` public contract の変更
- 本物の physics callback 完全統合
- Template / Assist / Generator 設計

---

## 3. Guard Rails Established in Phase 2

Phase 2 で成立した最小 guard rail は以下である。

### Validator
- 未登録 action を reject
- required payload key 不足を reject
- unknown payload key を reject
- scene 遷移先の参照切れを reject
- billiards 用 event 名を定義済み集合として検証
- WorldState key namespace の最低限検証

### Regression / Diagnostic
- valid / invalid scenario を機械的に判定可能
- validator failure と runtime failure を分離して扱える
- `mystery_test` をベースラインとして維持
- `billiards_test` を fake event で回帰可能
- clear / fail / continue の経路確認が可能

---

## 4. Billiards Test Positioning

`billiards_test` は、ビリヤードゲームを完成させるための module ではない。  
その役割は、**physics / event-driven な外部イベント流入下でも Kernel の contract が破綻しないことを検証する最小 fitness test** である。

### What billiards_test validates
- external event wait
- event-driven flow control
- WorldState への論理状態正規化
- round resolution の分岐
- fake event 下での deterministic scenario progression

### What billiards_test does not validate yet
- real physics callback burst handling
- actual collision timing fidelity
- generalized target_count > 2 support
- production-level billiards rules

---

## 5. Current WorldState Contract for billiards_test

Phase 2 時点での `billiards_test` の論理状態は、以下の key を基準とする。

| Key | 型 | 用途 |
|---|---|---|
| `billiards_test:round:shots_taken` | int | shot_committed の累積カウント |
| `billiards_test:round:target_1_pocketed` | bool | 1球目ポケット済み |
| `billiards_test:round:target_2_pocketed` | bool | 2球目ポケット済み |
| `billiards_test:round:cue_ball_pocketed` | bool | ファウル判定 |
| `billiards_test:round:result` | str | "clear" / "fail" / "active" |
| `billiards_test:event:last_name` | str | 最後に記録されたイベント名 |

### Rule
WorldState には **論理状態のみ** を保存する。  
位置、速度、接触履歴などの frame-by-frame 物理情報は保存しない。

---

## 6. Verification Snapshot

Phase 2 完了時点で、validator regression は 9 ケース PASS である。

```
$ python3 tools/test_validator.py
  PASS  mystery_case.yaml is valid
  PASS  mystery_stress_test.yaml is valid
  PASS  mystery_timeout_test.yaml is valid
  PASS  diagnostic_test.yaml is valid
  PASS  mystery_corrupted.yaml is REJECTED
  PASS  billiards_fake_smoke.yaml is valid
  PASS  billiards_foul_smoke.yaml is valid
  PASS  billiards_continue_smoke.yaml is valid
  PASS  billiards_corrupted.yaml is REJECTED

ALL 9 TESTS PASSED
```

### Covered cases
- mystery valid scenarios（4件）
- mystery corrupted scenario（reject）
- billiards fake smoke（clear パス）
- billiards foul smoke（foul fail パス）
- billiards continue smoke（continue パス）
- billiards corrupted scenario（reject）

This means Phase 2 has reached the minimum level required to distinguish:

- YAML / DSL failure
- Task-level logic failure
- runtime behavior under fake event flow

---

## 7. Scope Boundary Confirmed

Phase 2 では、以下を **別トラック** として切り離した。

### Deferred / Separate Track
- `ActionRegistry` GDExtension init crash
- Core-side initialization order fixes
- registry ownership / memory model changes
- full physics callback integration

これらは Phase 2 の main line には含めない。  
必要なら別 PR / 別トラックで扱う。

---

## 8. Exit Criteria Satisfied

Phase 2 の exit criteria は次の通りであり、すべて満たした。

| 条件 | 状態 |
|---|---|
| Core non-modification preserved | ✅ `git diff HEAD -- src/core/` = 0 行 |
| validator minimum line established | ✅ 13 アクション定義済み |
| regression minimum line established | ✅ 9 テスト全 PASS |
| billiards_test minimal skeleton established | ✅ 4 タスク・3 シナリオ |
| fake-event flow regression established | ✅ clear / fail / continue |
| docs and state naming aligned | ✅ WorldState key spec 準拠に更新 |

よって、Phase 2 は **closed** とする。

---

## 9. Entry Definition for Next Phase

次フェーズでは、Phase 2 の成果を壊さずに、次のどちらかの方向へ進む。

### Option A: Additional module expansion
次の fitness test module を追加する。  
候補:
- `roguelike_test`
- `rhythm_test`

### Option B: event integration hardening
`billiards_test` を使って、real signal / real physics callback 連携を別段階で検証する。

---

## 10. Next Phase Rules

次フェーズでも以下は継続ルールとする。

- `src/core` を安易に拡張しない
- Core convenience hook を増やさない
- Task を拡張ポイントとして扱う
- validator / regression を先に壊さない
- fake で成立を確認してから本統合に進む
- WorldState を論理状態ストアとして維持する

---

## 11. Open Items (Not Blocking Phase 2 Closure)

以下は未解決だが、Phase 2 完了を妨げない。

| # | 内容 | 対応方針 |
|---|---|---|
| 1 | `ActionRegistry` の GDExtension init crash 対策 | 別 PR `fix/action-registry-gdextension-init-crash` |
| 2 | `billiards_test` の target_count > 2 一般化 | Phase 3 以降で判断 |
| 3 | real physics callback integration | Phase 3 以降 |
| 4 | 次モジュール（roguelike / rhythm）の選定 | 次フェーズ開始時に決定 |

---

## 12. Relation to Existing Docs

| ドキュメント | 関係 |
|---|---|
| [DEV_STATUS.md](../DEV_STATUS.md) | Phase 2 完了サマリー・コミット履歴を管理 |
| [docs/reports/phase2_diff_audit_report.md](reports/phase2_diff_audit_report.md) | 差分整理の詳細・Category A/B 分類 |
| [docs/architecture/](architecture/) | Kernel v2.0 の構造設計（本文書では変更なし） |

---

## 13. Final Statement

Phase 2 is complete.

At this point, the project has:
- a stricter validation boundary,
- a minimum regression baseline,
- and a first event-driven fitness module that works without changing Core.

This is the minimum stable platform required before moving to the next expansion step.
