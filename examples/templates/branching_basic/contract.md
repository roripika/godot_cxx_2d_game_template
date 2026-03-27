# Branching Basic — WorldState Contract & Task Contract

このファイルは Branching ゲームが Kernel と交わす**契約書**です。  
実装コードは含みません。新しいゲームを作る際にこの表を自分のゲーム用に埋めてください。

---

## 1. WorldState キー設計原則

| 原則 | 内容 |
| :--- | :--- |
| Namespace | `<your_game>:` プレフィックスを全キーに付ける |
| Scope | SESSION（現セッション内の証拠）と GLOBAL（永続的な事実）を用途で厳密に分ける |
| 型 | `bool`（フラグ）のみ。証拠は「ある / ない」の 2 値で管理する |
| 分類 | 論理状態のみ。UI 表示状態・テキスト内容は書かない |

---

## 2. WorldState キー仕様テーブル（mystery_test 実装値）

### 重要: スコープ対応ルール

```
写す Task          →  書き込むスコープ  →  読み取る Task
discover_evidence  →  SCOPE_SESSION    →  check_condition
add_evidence       →  SCOPE_GLOBAL     →  check_evidence
```

**この 2 ペアを混用しないこと。** 詳細は後述の「禁止パターン」を参照。

### 2-a. SESSION スコープ証拠（`discover_evidence` が書き込む）

`check_condition` が読み取る。現セッション内のみ有効。

| キー | 型 | 書き込む Task | 例 |
| :--- | :--- | :--- | :--- |
| `<ns>:evidence:<id>` | bool | `discover_evidence` | `mystery_test:evidence:knife` |

### 2-b. GLOBAL スコープ証拠（`add_evidence` が書き込む）

`check_evidence` が読み取る。セッションをまたいで永続する。

| キー | 型 | 書き込む Task | 例 |
| :--- | :--- | :--- | :--- |
| `<ns>:evidence:<id>` | bool | `add_evidence` | `mystery_test:evidence:motive_money` |

### WorldState を直接書かないもの

以下は WorldState に書いてはいけない：

- ダイアログの表示テキスト（`show_dialogue` は WorldState を使わない）
- シーン遷移先の名前（YAML の payload で直接渡す）
- UI の表示状態・アニメーションフラグ

---

## 3. Task 契約（責務・入力・出力・禁止事項）

### 3-1. discover_evidence

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 証拠を発見し、SCOPE_SESSION に書き込む |
| **payload** | `evidence_id: <string>`（必須。旧来の `id:` も受け付ける） / `location: <string>`（必須、ログ用） |
| **読む WorldState** | `<ns>:evidence:<id>`（重複発見スキップのため） |
| **書く WorldState** | `<ns>:evidence:<id> = true`（SCOPE_SESSION） |
| **Fail-Fast** | `evidence_id` / `location` のどちらかが欠損したらエラー |
| **禁止** | SCOPE_GLOBAL には書かない |

### 3-2. add_evidence

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 動機・事実などの抽象的証拠を SCOPE_GLOBAL に書き込む |
| **payload** | `id: <string>`（必須） / `type: <string>`（任意、ログ用のみ・ランタイムに影響しない） |
| **読む WorldState** | なし（冪等書き込み） |
| **書く WorldState** | `<ns>:evidence:<id> = true`（SCOPE_GLOBAL） |
| **Fail-Fast** | `id` が欠損したらエラー |
| **禁止** | `discover_evidence` と混同しない |

### 3-3. check_condition

| 項目 | 内容 |
| :--- | :--- |
| **責務** | SESSION スコープの証拠フラグを多条件評価し、次シーンへ分岐する |
| **payload** | `all_of:` または `any_of:`（どちらか 1 つのみ、必須） + `if_true: <scene_id>`（必須） + `if_false: <scene_id>`（必須） |
| **読む WorldState** | SCOPE_SESSION: `<ns>:evidence:<id>` |
| **書く WorldState** | なし（読取専用） |
| **Fail-Fast** | `all_of` と `any_of` の両方/どちらもない場合、`if_true`/`if_false` の欠損でエラー |
| **禁止** | SCOPE_GLOBAL の証拠を読もうとしても false になる（mix 禁止） |

#### all_of / any_of の形式

```yaml
# all_of: 配列のすべての証拠が SESSION にある場合 → if_true
- action: check_condition
  payload:
    all_of:
      - evidence: knife
      - evidence: fingerprint
    if_true: accuse_chef
    if_false: accuse_butler

# any_of: 配列のいずれか 1 つが SESSION にある場合 → if_true
- action: check_condition
  payload:
    any_of:
      - evidence: witness_a
      - evidence: witness_b
    if_true: guilty
    if_false: innocent
```

### 3-4. check_evidence

| 項目 | 内容 |
| :--- | :--- |
| **責務** | GLOBAL スコープの 1 証拠フラグを評価し、次シーンへ分岐する |
| **payload** | `id: <string>`（必須） + `if_true: <scene_id>`（必須） + `if_false: <scene_id>`（必須） |
| **読む WorldState** | SCOPE_GLOBAL: `<ns>:evidence:<id>` |
| **書く WorldState** | なし（読取専用） |
| **Fail-Fast** | `id` / `if_true` / `if_false` の欠損でエラー |
| **禁止** | SCOPE_SESSION の証拠を読もうとしても false になる（mix 禁止） |

### 3-5. show_dialogue

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 1 行のセリフを表示し、確認まで実行を一時停止する |
| **payload** | `speaker: <string>`（必須） + `text: <string>`（必須） |
| **読む WorldState** | なし |
| **書く WorldState** | なし |
| **禁止** | WorldState にテキスト内容を書かない |

### 3-6. end_game

| 項目 | 内容 |
| :--- | :--- |
| **責務** | ゲームを終了しシナリオを停止する |
| **payload** | `result: <string>`（必須: `solved` / `wrong` / `failed` など） |
| **読む WorldState** | なし |
| **書く WorldState** | なし |
| **Fail-Fast** | `result` 欠損でエラー |

---

## 4. Task 追加時の契約拡張ルール

1. 新 Task のキー定義をこのファイルの 2 節に追記する。
2. SESSION 証拠を書く Task は `discover_evidence` パターンを踏襲する（重複スキップ付き）。
3. GLOBAL 証拠を書く Task は `add_evidence` パターンを踏襲する（冪等書き込み）。
4. 条件評価・分岐は `check_condition` または `check_evidence` の 1 Task に集約する。
5. `src/core` のコードは変更しない（絶対制約）。

---

## 5. 禁止パターン

```yaml
# ❌ add_evidence と check_condition のスコープ混用
- action: add_evidence
  payload:
    id: motive_money          # → SCOPE_GLOBAL に書く
- action: check_condition
  payload:
    all_of:
      - evidence: motive_money  # → SCOPE_SESSION を読む → 常に false → バグ
    if_true: guilty
    if_false: innocent

# ❌ WorldState にセリフを書く
- action: write_state
  payload:
    key: current_text
    value: "包丁を見つけた"    # → WorldState にテキストを書かない

# ❌ Task が直接シーンを呼び出す（ScenarioRunner に委ねるべき）
# → C++ Task 内で runner_->load_scene_by_id() する場合は
#    evaluate 系の Task のみに限定する

# ❌ all_of と any_of を両方書く
- action: check_condition
  payload:
    all_of:
      - evidence: knife
    any_of:
      - evidence: fingerprint  # → validate_and_setup() でエラー
    if_true: guilty
    if_false: innocent
```

---

## 6. 参照実装の WorldState 実測値（mystery_test、2026-03-27 確認）

| 経路 | 発見証拠（SESSION） | `check_condition` 評価 | エンディング |
| :--- | :--- | :--- | :--- |
| solved | knife=true, fingerprint=true | all_of → true | `result: solved` |
| wrong | knife=true, fingerprint=false | all_of → false | `result: wrong` |

**注意**: mystery_test は「continue（ループ）」経路が存在しません。  
証拠収集後は必ずいずれかのエンディングに到達します。
