# Branching Basic — 動作確認手順と実測値リファレンス

このファイルは Branching テンプレートを使ったゲームの  
**runtime 検証（全エンディング経路テスト）** の手順と期待値を定義します。

---

## 1. 確認の方針

Branching ゲームは以下の経路すべてで動作が確認されている必要があります。

| 経路 | 終了条件 | エンディング | `check_condition` 評価 |
| :--- | :--- | :--- | :--- |
| **solved** | 必要な全証拠が SESSION に存在する | `result: solved` | all_of → true |
| **wrong** | 証拠が不足している | `result: wrong` | all_of → false |

**Turn/Grid との差異**: mystery パターンは「continue（ループ）」経路がありません。  
証拠収集フェーズが完了すると必ずいずれかのエンディングに到達します。

---

## 2. Smoke テスト手順

### 前提

- Godot エディタまたは debug scene から対象シナリオを実行できること
- `discover_evidence` が DEBUG ログを stdout に出力することを確認できること

### ステップ 1: debug シーンを用意する

```
samples/<your_game>/
└── <your_game>_debug_scene.tscn  ← debug overlay + ScenarioRunner 配置
```

mystery の例: `samples/mystery/karakuri_mystery_shell.tscn`

### ステップ 2: solved 経路を確認する

YAML の `investigation` シーンに全証拠の `discover_evidence` が含まれた状態で実行する。

```yaml
# solved 経路の investigation シーン例
- action: discover_evidence
  payload:
    evidence_id: knife
    location: kitchen

- action: discover_evidence
  payload:
    evidence_id: fingerprint
    location: handle

- action: check_condition
  payload:
    all_of:
      - evidence: knife
      - evidence: fingerprint
    if_true: solved_scene
    if_false: wrong_scene
```

**確認すべき状態（mystery_test 実測値）**:

| 確認項目 | 期待値 |
| :--- | :--- |
| SESSION: `evidence:knife` | `true` |
| SESSION: `evidence:fingerprint` | `true` |
| `check_condition` 評価 | all_of → `true` |
| シーン遷移 | `solved_scene` |
| `end_game result` | `solved` |

### ステップ 3: wrong 経路を確認する

証拠 B の `discover_evidence` をコメントアウトまたは YAML から削除して実行する。

```yaml
# wrong 経路の investigation シーン例（fingerprint を発見しない）
- action: discover_evidence
  payload:
    evidence_id: knife
    location: kitchen
# ↑ fingerprint の discover_evidence を削除

- action: check_condition
  payload:
    all_of:
      - evidence: knife
      - evidence: fingerprint   # SESSION にないため false
    if_true: solved_scene
    if_false: wrong_scene       # ← こちらに分岐
```

**確認すべき状態**:

| 確認項目 | 期待値 |
| :--- | :--- |
| SESSION: `evidence:knife` | `true` |
| SESSION: `evidence:fingerprint` | `false`（未発見） |
| `check_condition` 評価 | all_of → `false` |
| シーン遷移 | `wrong_scene` |
| `end_game result` | `wrong` |

---

## 3. スコープ混用のデバッグ方法

`check_condition` が常に false を返す場合、スコープ混用を疑う。

**確認手順**:

1. 証拠を書いている Task を確認する。
   - `discover_evidence` → SCOPE_SESSION → `check_condition` で読む（正しい）
   - `add_evidence` → SCOPE_GLOBAL → `check_evidence` で読む（正しい）

2. 書いた Task と読む Task のスコープが一致しているか確認する。

3. DEBUG ログを確認する：
   ```
   [DiscoverEvidenceTask] Evidence discovered: knife (kitchen)
   [CheckConditionTask] Evaluating all_of: knife=true fingerprint=false → false
   ```

---

## 4. 確認チェックリスト

実装完了の判断基準として以下をすべて満たすこと。

- [ ] `solved` 経路が runtime で確認された
- [ ] `wrong` 経路が runtime で確認された
- [ ] DEBUG ログで全 `discover_evidence` の書き込みが確認された
- [ ] `check_condition` の評価ログが期待どおりに出力された
- [ ] `src/core/` のファイルに変更が加えられていない
- [ ] Validator が不正 payload（欠損フィールド、all_of+any_of 両方付き）でエラーを返す
- [ ] non-start シーンに pos-0 sacrifice パターンが適用されている
- [ ] SESSION と GLOBAL のスコープが意図どおりに使い分けられている

---

## 5. 既知の落とし穴

### 5-1. スコープ混用バグ（Branching 特有）

Turn/Grid にはない Branching 固有の落とし穴です。

```yaml
# ❌ add_evidence で書いて check_condition で読む → 常に false
- action: add_evidence
  payload:
    id: motive_money
- action: check_condition
  payload:
    all_of:
      - evidence: motive_money  # SESSION にない → false
```

```yaml
# ✅ 正しい SESSION パターン
- action: discover_evidence
  payload:
    evidence_id: motive_money
    location: diary
- action: check_condition
  payload:
    all_of:
      - evidence: motive_money  # SESSION にある → true
```

### 5-2. pos-0 スキップバグ

シーン遷移後は pos-0 がスキップされます。`investigation` 以外のシーン（`solved_scene`, `wrong_scene`）の pos-0 に同一 Task のダミーを置く。

```yaml
solved_scene:
  on_enter:
    - action: end_game        # pos-0 ← ダミー（スキップされる）
      payload:
        result: solved
    - action: end_game        # pos-1 ← 実際に実行される
      payload:
        result: solved
```

詳細: `docs/infrastructure_backlog.md`

### 5-3. 重複 discover_evidence

同一証拠を 2 度 `discover_evidence` しても警告なくスキップされます（意図的な設計）。  
ただし `add_evidence` は重複チェックなく上書きします（冪等）。

---

## 6. 参照実装の実測値（mystery_test、2026-03-27 確認済み）

mystery_test における全経路の確認済み実測値です。

### solved 経路（mystery_test）

```
[DiscoverEvidenceTask] Evidence discovered: knife (kitchen)
[DiscoverEvidenceTask] Evidence discovered: fingerprint (handle)
[CheckConditionTask] all_of: knife=true fingerprint=true → result=true
→ Scene: accuse_chef
→ end_game result: solved
```

### wrong 経路（mystery_test）

```
[DiscoverEvidenceTask] Evidence discovered: knife (kitchen)
# fingerprint は発見されない
[CheckConditionTask] all_of: knife=true fingerprint=false → result=false
→ Scene: accuse_butler
→ end_game result: wrong
```

---

## 7. 自ゲームの実測値記録欄（ここを埋める）

ゲーム名: _______________  
確認日: _______________

| 経路 | 確認状態 | 発見済み証拠（SESSION） | `check_condition` 評価 | エンディング `result` |
| :--- | :--- | :--- | :--- | :--- |
| solved | ⬜ 未確認 | | all_of → true | |
| wrong | ⬜ 未確認 | | all_of → false | |
