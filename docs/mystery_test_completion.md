# mystery_test Completion Memo

Date: 2026-03-27

このドキュメントは `mystery_test` の現時点での完了状態を固定する運用メモです。  
新しい設計提案ではなく、スコープ拡張も行いません。

---

## 1. Purpose

`mystery_test` はフルゲームのミステリー作品を出荷するためのモジュールではありません。

その目的は、**証拠収集 → 多条件評価 → 複数エンディング分岐** というパターンが  
`src/core` を一切変更せずに Kernel v2.0 上で安全に動作することを検証することです。

---

## 2. mystery_test が検証すること

| 検証項目 | 内容 |
| :--- | :--- |
| 証拠フラグ書き込み | `discover_evidence` / `add_evidence` が WorldState に正しく書き込む |
| SESSION / GLOBAL スコープ分離 | `discover_evidence` → SCOPE_SESSION、`add_evidence` → SCOPE_GLOBAL の使い分けが機能する |
| 多条件ロジック | `check_condition` の `all_of` / `any_of` 評価が正しく分岐する |
| エンディング分岐 | 証拠の有無によって `solved` / `wrong` の 2 経路に正しく分かれる |
| シナリオ進行 | `show_dialogue` → `discover_evidence` → `check_condition` → `end_game` の順次実行 |
| FakeCommand 不使用 | mystery_test はプレイヤーコマンドがなく、YAML が直接証拠を配置する決定論的モデル |
| Validator 動作 | 不正 payload で `validate_and_setup()` がエラーを返す |

---

## 3. 現在の検証済みスコープ

このメモが固定する実装範囲：

- 証拠 ID ベースの WorldState フラグ管理
- `all_of` 条件評価（すべての証拠が揃った場合に真）
- `any_of` 条件評価（いずれか 1 つが揃った場合に真）
- 2 エンディング分岐（`solved` / `wrong`）
- `show_dialogue`、`wait_for_signal`、`end_game` アクション
- `diagnostic_test.yaml` による診断シナリオ
- `mystery_stress_test.yaml` によるストレステスト
- Validator の Fail-Fast 動作（必須フィールド欠損時のエラー）
- `save_load_test` タスクによる GlobalScope 保存・ロード確認
- `parallel` タスクグループによる並列実行

---

## 4. Runtime 検証状態

| 経路 | 状態 | 確認内容 |
| :--- | :--- | :--- |
| `solved`（全証拠揃い） | ✅ CONFIRMED | `check_condition` all_of が true → `accuse_chef` シーン → `end_game result:solved` |
| `wrong`（証拠不足） | ✅ CONFIRMED | `check_condition` all_of が false → `accuse_butler` シーン → `end_game result:wrong` |

**注意**: mystery_test は Turn/Grid と異なり「continue（ループ）」経路が存在しません。  
証拠収集フェーズが完了した時点で必ずどちらかのエンディングに到達します。

---

## 5. WorldState Contract Snapshot

Namespace と Scope：

- Namespace: `mystery_test`
- 2 スコープを用途で使い分ける

### SCOPE_SESSION キー（`discover_evidence` が書き込む）

| キー | 型 | 書き込む Task | 用途 |
| :--- | :--- | :--- | :--- |
| `mystery_test:evidence:<id>` | bool | `discover_evidence` | 現セッション内の証拠発見フラグ |

SESSION は `check_condition` が読み取る。セッション終了で消去される。

### SCOPE_GLOBAL キー（`add_evidence` が書き込む）

| キー | 型 | 書き込む Task | 用途 |
| :--- | :--- | :--- | :--- |
| `mystery_test:evidence:<id>` | bool | `add_evidence` | セッションをまたぐ証拠・動機フラグ |

GLOBAL は `check_evidence` が読み取る。セッションをまたいで永続する。

### スコープ混用禁止ルール

```
# ❌ add_evidence で書いた証拠を check_condition で読もうとする
#    → check_condition は SCOPE_SESSION しか読まないため false になる
- action: add_evidence
  payload:
    id: motive_money
- action: check_condition
  payload:
    all_of:
      - evidence: motive_money   # ← SCOPE_SESSION に存在しない → 判定 false
    if_true: accuse
    if_false: skip

# ✅ 正しい使い方
#    SESSION 証拠: discover_evidence → check_condition
#    GLOBAL 証拠: add_evidence → check_evidence
```

---

## 6. Task 一覧（現行 mystery_test で登録済み）

| action 名 | カテゴリ | 戻り値 | Mutates WorldState | 分岐 |
| :--- | :--- | :--- | :--- | :--- |
| `show_dialogue` | Dialogue | Waiting | なし | なし |
| `add_evidence` | Mutation | Success | SCOPE_GLOBAL | なし |
| `discover_evidence` | Mutation | Success | SCOPE_SESSION | なし |
| `check_evidence` | Branch | Success | なし（読取専用） | あり |
| `check_condition` | Branch | Success | なし（読取専用） | あり |
| `wait_for_signal` | Timeout | Waiting/Failed | なし | なし |
| `end_game` | Terminal | Success | なし | なし |
| `save_load_test` | Diagnostic | Success/Failed | SCOPE_GLOBAL（一時） | なし |
| `parallel` | Composite | Waiting/Success/Failed | サブタスク依存 | なし |

詳細仕様: `docs/mystery_test_task_catalog.md`

---

## 7. Debug シーン

| ファイル | 用途 |
| :--- | :--- |
| `samples/mystery/karakuri_mystery_shell.tscn` | メイン実行シーン |
| `src/games/mystery_test/scenario/mystery_case.yaml` | 参照 smoke シナリオ |
| `src/games/mystery_test/scenario/diagnostic_test.yaml` | 診断シナリオ |

---

## 8. 意図的にスコープ外のもの

以下は現在の完了状態に含まれません。

- リアル入力との統合（クリック・UI ボタン連動）
- 3 つ以上のエンディング分岐
- HPストライク制（`take_damage` アクション）
- 証拠インベントリの UI 表示
- セーブ・ロードの本番運用（`save_load_test` は diagnostic 専用）
- 国際化・翻訳システムとの統合

---

## 9. 実装関連コミット（参考）

| コミット | 内容 |
| :--- | :--- |
| mystery_test 最小骨格 | Task クラス 5 種の初期実装 |
| mystery_test 追加 Task | `parallel`, `save_load_test`, `wait_for_signal` 追加 |
| mystery_test docs | Task カタログ・YAML 仕様書作成 |

ドキュメント参照:
- 設計書: `docs/mystery_design.md`
- Task カタログ: `docs/mystery_test_task_catalog.md`
- YAML 仕様書: `docs/mystery_test_yaml_spec.md`

---

## 10. 最終状態

`mystery_test` は Kernel v2.0 上での **証拠収集・多条件評価・エンディング分岐** パターンの  
有効な完了チェックポイントに到達しています。

`solved` / `wrong` 両経路で validator・runtime ともに確認済みです。  
`src/core` のコードは一切変更されていません。
