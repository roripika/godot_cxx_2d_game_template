# Generator Commonization Candidates

**ステータス**: v1.0（候補整理）  
**作成日**: 2026-04-02  
**対象**: `tools/gen_scenario_*.py`（branching / turn_grid / time_clock / event_driven）

---

## 1. 目的

4 generator の共通化候補を整理し、Phase 4 で安全に着手できる順序を固定する。  
本メモは実装ではなく、共通化範囲の境界定義を目的とする。

---

## 2. 前提と非目的

前提:
- Human Gate（HG-1〜HG-4）は維持
- `src/core` 非変更
- 既存 validator / regression 非破壊

非目的:
- 4 generator の全面統合実装
- 自動修復ループ導入
- template 個別ルールの消去

---

## 3. 現状の共通パイプライン

4 generator は実装詳細は異なるが、実行フローはほぼ共通。

1. CLI で spec ファイルを受け取る
2. spec を読み込む（PyYAML）
3. `validate_spec` で Fail-Fast 検証
4. `generate_yaml` で YAML 骨格生成
5. `generate_review` で HG-2 用 review 生成
6. `scenarios/generated/<name>_<ts>.yaml` に出力
7. validate と HG-4 を案内して終了

---

## 4. 共通化候補（優先順）

| ID | 候補 | 優先度 | 期待効果 | 主なリスク | 判断 |
|:---|:---|:---:|:---|:---|:---|
| C-01 | CLI/入出力の共通ヘルパー（spec 読み込み・out-dir 解決・timestamp 命名） | High | 重複コード削減、実装差分の見通し改善 | 既存 exit code の差異吸収が必要 | 着手候補 |
| C-02 | 出力書き込み共通化（YAML/review の命名・保存・生成ログ） | High | 生成物運用の一貫性向上 | ログ文言差異の調整 | 着手候補 |
| C-03 | 共通バリデータ部品（identifier / scene 名 / result 値チェック） | Medium | ルール重複を減らせる | template 固有規則と混在しやすい | 条件付き候補 |
| C-04 | review.md の共通セクション（HG-3/HG-4案内、実行コマンド） | Medium | レビュー品質の均一化 | template 固有チェックの埋没 | 条件付き候補 |
| C-05 | `tools/gen_scenario.py` dispatcher（template で分配） | Medium | 呼び出し口を一本化 | 早期導入でデバッグ面倒化 | 後段候補 |
| C-06 | 統一テストヘルパー（generator 共通 smoke 実行） | High | 退行検知の強化 | テスト設計の先行合意が必要 | 着手候補 |

---

## 5. 共通化しない候補（現時点）

| 非候補 | 理由 |
|:---|:---|
| `validate_spec` の完全統一 | template ごとの入力制約が大きく異なり、可読性と保守性を落とす |
| `generate_yaml` の単一 DSL 化 | YAML 構造差が大きく、Phase 4 での変更範囲が過大 |
| runtime / validator 自動修復ループ | Human Gate 方針に反する |
| `src/core` 側の共通化前提改修 | Out of Scope |

---

## 6. 段階導入プラン（推奨）

1. Step 1: C-01/C-02 のみ（I/O と命名の共通化）
2. Step 2: C-06（共通 smoke ヘルパー）で退行検知を先に固める
3. Step 3: C-03/C-04 を必要最小限で投入
4. Step 4: C-05 dispatcher は上記安定後に限定導入

---

## 7. 次の実装タスクに渡す最小要件

次タスク（実装フェーズ）では以下のみを満たす。

1. 既存 4 generator の CLI 互換を壊さない
2. 既存 expected output 検証を壊さない
3. HG-2/HG-3/HG-4 の案内が後退しない
4. 変更差分は `tools/` と `tests/` に限定する

---

## 8. 参照

- [`phase3_c_completion.md`](phase3_c_completion.md)
- [`phase3_c_generator_entry.md`](phase3_c_generator_entry.md)
- [`phase4_entry.md`](phase4_entry.md)
- [`phase4_backlog.md`](phase4_backlog.md)

