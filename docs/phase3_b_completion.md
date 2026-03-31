# Phase 3-B 完了メモ — AI Assist Tooling

**ステータス**: 完了 ✅  
**期間**: 2026-03-29 〜 2026-03-31  
**タスク範囲**: T7 / T7.1 / T8 / T9 / T10

---

## 目的

Phase 3-A で整備したジャンル別テンプレートを **AI が正しく使える** ための補助ツール群を最小限で構築する。  
- 実装コード（`src/core/` / `src/games/`）には一切触れない  
- ドキュメント生成・シナリオ静的検証・few-shot 例示のみで完結させる

---

## 成果物一覧

| タスク | 成果物 | commit | 種別 |
|:---:|:---|:---:|:---:|
| T7 | `tools/gen_task_catalog.py` — タスクカタログ自動生成スクリプト | `b0f2c2f` | ツール |
| T7 | `docs/task_catalog.md` — 生成物（26 tasks / 4 modules） | `b0f2c2f` | 常時参照 |
| T7.1 | `docs/task_catalog.md` スコープ定義節追加・「26 tasks」根拠明記 | `4587ce4` | 補強 |
| T8 | `tools/validate_scenario.py` — 全 18 エラー種別に `SUGGEST:` 行追加 | `4587ce4` | ツール |
| T9 | `docs/validate_scenario_guidance.md` — Guidance 仕様書 | `d56c1a5` | 参照 |
| T10 | `docs/few_shot_prompts.md` — 4 テンプレート × few-shot 例示 | `1895d42` | **YAML 作成時補助** |

---

## AI Assist Tooling の到達点

### ツール 1: Task カタログ（常時参照）

**ファイル**: `docs/task_catalog.md` / `tools/gen_task_catalog.py`

| 項目 | 内容 |
|:---|:---|
| カバレッジ | 4 モジュール / 26 tasks / Waiting 使用 4 件 |
| 抽出項目 | クラス名・責務・必須 payload key・省略可 key・WS 書き込み/読み込み |
| 再生成 | `python3 tools/gen_task_catalog.py`（ソースから完全再生成） |
| 位置づけ | **YAML を書く前に必ず参照**。action 名と payload 構造の唯一の正源 |

### ツール 2: シナリオ静的検証（CI 組み込み済み）

**ファイル**: `tools/validate_scenario.py` / `docs/validate_scenario_guidance.md`

| 項目 | 内容 |
|:---|:---|
| 静的検出 | 18 エラー種別（action/payload/scene 参照/enum 値/型/制約違反） |
| SUGGEST | 10 種に `Did you mean?` / スニペット提示（difflib, cutoff=0.6） |
| 対象 | billiards / mystery / roguelike / rhythm（4 モジュール全対応） |
| 使い方 | `python3 tools/validate_scenario.py <file>` → `exit 0` まで修正 |
| Guidance | SUGGEST が出ないエラーは `validate_scenario_guidance.md` を参照 |

### 補助資料: Few-Shot Prompts（**YAML 作成時のみ参照**）

**ファイル**: `docs/few_shot_prompts.md`

> **位置づけ**: 常時参照資料ではない。0 から YAML を書く際または AI にシナリオ生成を依頼する際に開く補助資料。

| 項目 | 内容 |
|:---|:---|
| Shot | 4 テンプレート × （入力例 / 選択理由 / 最小 YAML / よくある間違い） |
| 最小 YAML | `validate_scenario.py` でエラーなし確認済み |
| 落とし穴 | pos-0 sacrifice 未配置・実行順序崩れ（検証ツールが検出できない典型ミス）|
| 付録 | クイック選択フロー（30 秒でテンプレートを 1 つに絞る） |

---

## 3 ツールの標準ワークフロー

```
[1] テンプレート選択
    → docs/few_shot_prompts.md の「クイック選択フロー」で 1 テンプレートに絞る
      （YAML作成時のみ開く）

[2] Task 確認
    → docs/task_catalog.md で対象テンプレートの action 名・payload キーを確認
      （常時参照）

[3] YAML 作成
    → examples/templates/<genre>/scenario/*.yaml を雛形として利用
      よくある間違いは few_shot_prompts.md の Shot N を確認

[4] 静的検証
    → python3 tools/validate_scenario.py <your_scenario.yaml>
    → SUGGEST を読んで修正 → exit 0 まで繰り返す

[5] SUGGEST なしエラーの対処
    → docs/validate_scenario_guidance.md で対応表を確認

[6] runtime 実行（Godot）
    → 静的検証では検出できない問題のみが残る
```

---

## 限界（Phase 3-B では解決しない・できない）

### 静的解析の限界

| 限界 | 理由 | 回避策 |
|:---|:---|:---|
| **pos-0 sacrifice 未配置の検出** | 「最初のタスクがダミーかどうか」は意味解析が必要 | `few_shot_prompts.md` の ❌ セクションで手動確認 |
| **シーンの到達可能性（dead-end）** | 分岐グラフのトポロジカル解析は未実装 | runtime で確認 |
| **実行順序依存性** | `advance_rhythm_clock` → `judge_rhythm_note` 等の順序制約は未チェック | `few_shot_prompts.md` Shot 3 の順序図を遵守 |
| **WorldState 値の意味的妥当性** | player_x の有効範囲は Kernel 依存 | runtime で確認 |
| **並列実行禁止の検出** | 設計判断が必要なため対象外 | `validate_scenario_guidance.md` 参照 |

### ツール生成側の限界

| 限界 | 理由 |
|:---|:---|
| `gen_task_catalog.py` が cpp コメントのみ依存 | リフレクション情報をコメントで管理しているため、コメント欠落は検出不可 |
| SUGGEST の cutoff=0.6 | 4 文字以下の typo は候補なし（cutoff を下げると誤候補が増える） |

---

## 次段への入口

### Phase 3-B で答えが出た判断

| 問い | 答え |
|:---|:---|
| AI が「テンプレート選択」を誤る頻度は高いか？ | **Yes** → few_shot_prompts.md の Selection Guide で対処済み |
| 0 から YAML を書く際に構造的ミスが多いか？ | **Yes** → few_shot_prompts.md の最小 YAML 例で対処済み |
| validate SUGGEST だけで修正困難なケースがあるか？ | **一部 Yes** → validate_scenario_guidance.md で対処済み |
| Scaffold Generator（雛形 YAML 生成スクリプト）は必要か？ | **保留** — few_shot_prompts.md が代替として機能している間は不要 |

### Phase 3-C 以降で検討すること

以下は Phase 3-B では意図的にスコープ外とした。Feature 要件が固まったときに着手する。

| 候補 | 条件 |
|:---|:---|
| Scaffold Generator (`tools/gen_scenario_scaffold.py`) | few_shot_prompts.md のコピペでは追いつかない量のシナリオ生成が必要になったとき |
| validate の順序制約チェック | 実行順序ミスによる runtime バグが頻発したとき |
| NL エラー解説（LLM 連携） | SUGGEST + Guidance で修正できないエラーが蓄積したとき |
| `gen_task_catalog.py` の型情報抽出強化 | payload key の型（int/float/list）が省略コメントに依存しなくなったとき |

---

## 引き継ぎ情報

**Phase 3-B で固まった制約（変更禁止）:**
- `src/core/` 改変禁止は Phase 3 全体を通して維持
- `validate_scenario.py` の `VALID_ACTIONS` / `VALID_EVENTS` は Task クラスの追加に合わせて更新する（`gen_task_catalog.py` は自動更新しない）

**参照先まとめ:**

| 目的 | ファイル | 層 |
|:---|:---|:---:|
| テンプレート選択（常時） | `docs/template_selection_guide.md` | B |
| Task 名・payload 確認（常時） | `docs/task_catalog.md` | B |
| YAML 作成時の例示 | `docs/few_shot_prompts.md` | 補助（作業時のみ）|
| 検証エラー対処 | `docs/validate_scenario_guidance.md` | B |
| pos-0 skip 回避 | `docs/infrastructure_backlog.md` | B |
| Phase 3 全体ロードマップ | `docs/phase3_planning.md` | B |
