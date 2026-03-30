# Phase 3-B 前半 完了メモ（AI Assist Tooling）

**ステータス**: 前半完了 ✅  
**期間**: 2026-03-29 〜 2026-03-30  
**タスク範囲**: T7 / T7.1 / T8 / T9

---

## 目的

Phase 3-A で整備したジャンル別テンプレートを **AI が正しく使える** ための補助ツール群を最小限で構築する。  
実装コード・Core には一切触れず、ドキュメント生成とシナリオ静的検証のみで完結させる。

---

## 成果物一覧

| タスク | 成果物 | commit |
|:---:|:---|:---:|
| T7 | `tools/gen_task_catalog.py` — タスクカタログ自動生成スクリプト<br>`docs/task_catalog.md` — 生成物（26 tasks / 4 modules） | `b0f2c2f` |
| T7.1 | `docs/task_catalog.md` のスコープ定義節追加<br>「30 tasks」誤記を「26」に訂正・根拠明記 | `4587ce4` |
| T8 | `tools/validate_scenario.py` guidance 強化<br>全エラー種別に `SUGGEST:` 行を追加 | `4587ce4` |
| T9 | `docs/validate_scenario_guidance.md`<br>— Guidance 仕様書（18 エラー種別・対応表・AI 利用ガイド） | `d56c1a5` |

---

## 各ツールの責務と現状

### `tools/gen_task_catalog.py` + `docs/task_catalog.md`

| 項目 | 内容 |
|:---|:---|
| **入力** | `src/games/*/tasks/*.cpp`（読み取り専用） |
| **出力** | `docs/task_catalog.md`（Markdown、再生成可） |
| **抽出項目** | クラス名・責務・必須 payload key・省略可能 payload key・WS 書き込みキー・WS 読み込みキー・Waiting 有無 |
| **再生成** | `python3 tools/gen_task_catalog.py` |
| **カバレッジ** | 4 モジュール / 26 tasks / Waiting 使用 4 件 |

### `tools/validate_scenario.py` (T8 強化済み)

| 項目 | 内容 |
|:---|:---|
| **入力** | YAML シナリオファイル |
| **出力** | `ERROR + SUGGEST` 形式の標準出力 |
| **SUGGEST 提供** | 18 エラー種別のうち 10 種に対応（セクション4参照） |
| **Did you mean?** | `difflib.get_close_matches(cutoff=0.6)` |
| **対象シナリオ** | mystery / billiards / roguelike / rhythm（4 モジュール全対応） |
| **CI 統合** | `exit 0` = ALL VALID、 `exit 1` = エラーあり |

---

## SUGGEST カバレッジ（T8 実績）

| エラー種別 | SUGGEST |
|:---|:---:|
| 不明な action 名（typo） | ✅ Did you mean? |
| 不明な payload key（typo） | ✅ Did you mean? |
| 必須 payload 欠落 | ✅ Add to payload スニペット |
| enum 値誤り（result / type / event / command） | ✅ Did you mean? + 有効値リスト |
| 存在しない scene 参照 | ✅ Did you mean? + Available scenes |
| timeout 値エラー | ✅ YAML スニペット例 |
| YAML 構文エラー | ❌ スコープ外 |
| parallel 禁止 action | ❌ 設計判断にかかわるため |
| rhythm notes/taps 制約 | ❌ 文脈依存 |
| WorldState 意味的整合性 | ❌ runtime 依存 |

---

## 限界（現時点で解決できないこと）

1. **pos-0 sacrifice の未配置を静的検出できない**  
   シナリオの先頭タスクがダミーかどうかは意味解析が必要。  
   → 回避策は `docs/infrastructure_backlog.md` を参照。

2. **シナリオの到達可能性を検証できない**  
   分岐グラフのトポロジカル解析は未実装。dead-end シーンは runtime まで検出不可。

3. **WorldState の値の意味的妥当性**  
   `player_x: 100` が有効な座標かどうかは Kernel 依存。静的検証の限界。

4. **タスク間の実行順序依存性**  
   例：`judge_rhythm_note` の前に `setup_rhythm_round` が必要、といった順序制約は未チェック。

---

## AI Assist Tooling の標準ワークフロー（確定）

```
[1] タスク選択
    → docs/task_catalog.md でモジュール・Task クラス・payload keys を確認

[2] YAML シナリオ作成
    → examples/templates/<genre>/scenario/*.yaml を雛形として利用

[3] 静的検証
    → python3 tools/validate_scenario.py <your_scenario.yaml>
    → SUGGEST を読んで修正 → exit 0 まで繰り返す

[4] docs/validate_scenario_guidance.md
    → SUGGEST が出ないエラーの対処先を確認

[5] runtime 実行（Godot）
    → 静的検証では検出できない問題のみ残る
```

---

## Phase 3-B 後半への引き継ぎ

### 現状の到達点

| 層 | ツール | 状態 |
|:---|:---|:---:|
| Task 発見 | `task_catalog.md` | ✅ 完成 |
| シナリオ検証 | `validate_scenario.py` (SUGGEST 付き) | ✅ 完成 |
| 検証ガイド | `validate_scenario_guidance.md` | ✅ 完成 |
| シナリオ生成支援 | Few-shot Prompts / Generator | ⏳ 未着手 |

### Generator Design へ進む条件

以下の質問に Yes なら Generator Design（T10〜）に進む価値がある:

1. **AI が「どのテンプレートを選ぶか」を誤る頻度は高いか？**  
   → Yes なら Few-shot Prompts で Selection Guide を組み込む。

2. **YAML を 0 から書く際に構造的なミスが多いか？**  
   → Yes なら Scaffold Generator（雛形 YAML 生成スクリプト）が有効。

3. **validate_scenario.py の SUGGEST だけでは修正が困難なケースがあるか？**  
   → Yes なら NL によるエラー解説（Schema-driven Guidance 文章化）が必要。

### Generator Design の最小候補（phase3_planning.md より）

| 候補 | 内容 | 優先度 |
|:---|:---|:---:|
| Few-shot Prompts | `docs/few_shot_prompts.md` — 各ジャンル別の YAML 生成例（in/out ペア） | 高 |
| Scaffold Generator | `tools/gen_scenario_scaffold.py` — genre 指定で雛形 YAML を出力 | 中 |
| NL Error Guidance | `validate_scenario_guidance.md` のエラー解説を NL 文章化 | 低（T9 で代替済み） |

---

## 参照ドキュメント

- [`task_catalog.md`](task_catalog.md) — Task カタログ（自動生成、26 tasks）
- [`validate_scenario_guidance.md`](validate_scenario_guidance.md) — 検証 Guidance 仕様
- [`phase3_planning.md`](phase3_planning.md) — Phase 3 全体ロードマップ
- [`phase3_a_completion.md`](phase3_a_completion.md) — Phase 3-A 完了メモ
- [`template_selection_guide.md`](template_selection_guide.md) — 4 テンプレート選択ガイド
