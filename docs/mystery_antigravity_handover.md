# Mystery Demo 引き継ぎ資料（antigravity）

## 1. 目的
- Mystery デモを `docs/mystery_design.md` 準拠で完成度を上げる。
- 実装は「再利用前提」で行い、デモ固有ロジックは `samples/mystery/**`、共通ロジックは Basic Game Karakuri（`src/karakuri/**`）に分離する。
- 今後の作業者は antigravity に固定する。

## 2. 現在の前提（2026-02-17 時点）
- 実行入口: `project.godot` の `run/main_scene` は `res://samples/mystery/karakuri_mystery_shell.tscn`。
- シナリオ実行: `KarakuriScenarioRunner` が `samples/mystery/scenario/mystery.yaml` を読む。
- 設計書: `docs/mystery_design.md`。
- 進捗チェック一次情報: `TASK.md` の `Priority: Mystery Demo Execution (Owner: antigravity)` セクション。

## 3. 非機能ルール（必須）
- `src/karakuri/**` の C++ は Doxygen 形式コメント必須。
- Basic Game Karakuri にデモ固有ロジックを入れない。
- 文字列は原則翻訳キー経由で扱い、直書きを避ける。
- PR には最低限 `Scope / Out of Scope / Test` を記載する。

## 4. 役割分担
- Designer:
  - 主編集対象: `samples/mystery/ui/**`、各 `.tscn` の見た目、テーマ。
  - 責務: レイアウト、視認性、操作導線、UIアニメーション。
- Planner:
  - 主編集対象: `samples/mystery/scenario/**`、翻訳データ（CSV/translation）。
  - 責務: シナリオ分岐、証拠配置、難易度、エンディング条件。
- Engineer (antigravity):
  - 主編集対象: `src/karakuri/**`、ランナー接続、NodePath契約、テスト導線。
  - 責務: データ駆動実行基盤、互換性、品質担保。

## 5. 実行順（マイルストーン）
1. M1 入口統一と旧導線整理
2. M2 YAML スキーマ契約
3. M3 C++責務寄せ
4. M4 モード分離
5. M5 多言語切替（EN/JA）
6. M6 役割分離の運用固定
7. M7 受け入れテスト固定
8. M8 ドキュメント同期

詳細チェックは `TASK.md` を唯一の進捗管理表として更新する。

## 6. Definition of Done（PR単位）
- 対象タスクのチェック項目が `TASK.md` で更新されている。
- `./dev.sh run mystery` で実行確認済み。
- 影響範囲（UI/Scenario/C++）が PR 本文に記載されている。
- 仕様差分がある場合、`docs/mystery_design.md` または本資料を同時更新している。

## 7. PR テンプレート（antigravity 用）
以下を PR 本文に貼り付ける。

```md
## Scope
- （このPRで実施したこと）

## Out of Scope
- （このPRで意図的にやらないこと）

## Changes
- [ ] UI
- [ ] Scenario(YAML)
- [ ] C++(Karakuri)
- [ ] Docs

## Test
- [ ] `./dev.sh run mystery` で起動確認
- [ ] 対象モードの遷移確認
- [ ] EN/JA 切替確認（該当時）
- [ ] HP/分岐/エンディング確認（該当時）

## Risks / Follow-up
- （既知課題、次PRへ持ち越す項目）
```

## 8. コミットメッセージ規約（推奨）
- 形式: `[Karakuri][Mystery][M{番号}] 要約`
- 例: `[Karakuri][Mystery][M5] Fix runtime locale switch in confrontation UI`

## 9. リスク一覧（先に見る）
- 旧シーンと新シーンの二重導線により、修正漏れが発生しやすい。
- 翻訳キー欠落時に英日混在表示が発生しやすい。
- NodePath変更で UI とランナー接続が壊れやすい。
- YAML 拡張時に後方互換を壊しやすい。

## 10. 最小報告フォーマット（作業完了時）
- Done:
  - （完了項目）
- Pending:
  - （未完項目）
- Evidence:
  - （確認コマンド、スクリーンショット、ログ）
- Next:
  - （次PRでやること）
