# Human Gate Decision Matrix

**ステータス**: v1.0  
**作成日**: 2026-04-02  
**対象**: Phase 4（`docs/phase4_entry.md`）

---

## 1. 目的

HG-1〜HG-4 の判定条件を固定し、各 Gate で「何を見て」「どう判断し」「どこで止めるか」を明確化する。

---

## 2. 判定ステータス（共通）

- `PASS`: Gate 通過。次の Gate に進む
- `REWORK`: 修正して同じ Gate を再実行
- `STOP`: このタスクを停止し、設計判断へ戻す

補足:
- Generator は Gate 通過を自動確定しない
- 最終判定権限は常に人間が持つ

---

## 3. Gate 判定表（HG-1〜HG-4）

| Gate | 主入力 | 必須確認項目 | PASS 条件 | REWORK 条件 | STOP 条件 | 記録すべき証跡 |
|:---|:---|:---|:---|:---|:---|:---|
| HG-1（template 選択後） | template 名、タスク目的 | template 選択理由が目的と整合しているか | template と目的の不整合がない | 目的と template が弱く整合しない | template 選択が 2 案以上で確定不能 | 採用 template と選定理由（短文） |
| HG-2（YAML 生成後） | 生成 YAML、`*_review.md` | シーン構成、Task 列、分岐先、sacrifice 配置 | 構造上の矛盾がなく意図通り | シーン名・分岐先・payload の手修正で解消可能 | 仕様自体が未確定で修正方針が決まらない | チェック済み項目、修正点 |
| HG-3（validator 実行後） | `validate_scenario.py` の結果 | exit code、SUGGEST、修正方針 | exit 0 | exit 1 だが手修正で収束可能 | 既存不具合か今回変更か切り分け不能 | 実行コマンド、exit code、主要エラー |
| HG-4（runtime 実行前） | validate 通過 YAML、実行手順 | runtime 経路（clear/fail/continue※）と end 結果 | 実行経路が期待どおり | 軽微修正で再検証可能 | Core/設計変更なしでは成立しない | 実行条件、確認経路、最終判断 |

※ continue 経路は該当テンプレートに存在する場合のみ。

---

## 4. テンプレート別 HG-2 重点確認

| template | HG-2 重点確認 | 典型的 REWORK |
|:---|:---|:---|
| `branching_basic` | `branches[].location`、`check_type`、terminal result | 分岐先シーン名の不整合、result 値の見直し |
| `turn_grid_basic` | `first_command` と Action 列の整合、terminal 名 | move/attack 系フローの意図ズレ |
| `time_clock_basic` | `notes/taps`、中間シーンの sacrifice、terminal 分岐 | timing 値と判定結果のズレ |
| `event_driven_basic` | wait/evaluate 連鎖、continue/terminal の sacrifice | event 記録順と if 分岐のズレ |

---

## 5. Gate 実行チェックリスト

1. HG-1: template 選択理由を 1 行で記録したか
2. HG-2: `*_review.md` を使って構造確認したか
3. HG-3: `python3 tools/validate_scenario.py <yaml>` の結果を記録したか
4. HG-4: 対象 template の実行経路を確認したか
5. STOP 条件に該当した場合、作業継続を止めたか

---

## 6. 参照

- [`phase3_c_generator_entry.md`](phase3_c_generator_entry.md)
- [`phase3_c_completion.md`](phase3_c_completion.md)
- [`generated_dir_policy.md`](generated_dir_policy.md)
- [`phase4_entry.md`](phase4_entry.md)

