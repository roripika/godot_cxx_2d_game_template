# validate_scenario.py — Guidance 仕様 (T9)

**ファイル**: `tools/validate_scenario.py`  
**Phase**: 3-B / T8 実装済み（2026-03-29）  
**目的**: AI が生成した YAML シナリオを実行前に静的検証し、エラーごとに修正案を提示する。

---

## 1. 出力フォーマット

```
  ERROR  <filename>  scene='<scene_id>'  action[<N>]
         <エラーメッセージ（何が問題か）>
         SUGGEST: <修正案（どう直すか）>
```

- `SUGGEST:` 行はエラーに対応する修正案がある場合のみ出力される。
- 複数エラーがある場合は全て列挙してから exit code 1 で終了する。
- `SUGGEST:` がない行は「構造的エラー」で自動提案が困難なケース。

---

## 2. エラー種別と SUGGEST 対応表

| # | エラー種別 | SUGGEST 提示 | アルゴリズム |
|:---:|:---|:---:|:---|
| E1 | 不明な action 名 | ✅ | Did you mean? (difflib) + 全 action リスト |
| E2 | 不明な payload key | ✅ | Did you mean? (difflib) + そのアクションの既知キーリスト |
| E3 | 必須 payload フィールド欠落（単一） | ✅ | `Add to payload:  key: <value>` |
| E4 | 必須 payload フィールド欠落（OR 選択） | ✅ | `Add one of: key1: <value>  OR  key2: <value>` |
| E5 | 不正な enum 値（end_game result） | ✅ | Did you mean? + 有効値リスト |
| E6 | 不正な enum 値（add_evidence type） | ✅ | Did you mean? + 有効値リスト |
| E7 | 不正な event 名（wait_for_billiards_event） | ✅ | Did you mean? + 有効 event リスト |
| E8 | 不正な command 値（load_fake_player_command） | ✅ | Did you mean? + 有効 command リスト |
| E9 | 存在しない scene 参照（if_true / if_false / if_clear 等） | ✅ | Did you mean? + Available scenes リスト |
| E10 | timeout が 0 以下または非数値 | ✅ | YAML スニペット例（`timeout: 10.0  # seconds`） |
| E11 | YAML パースエラー | ❌ | YAML 構文の問題はパーサーメッセージをそのまま表示 |
| E12 | トップレベルキー欠落（`start_scene` / `scenes`） | ❌ | 構造の最小要件を満たしていないため自動提案不可 |
| E13 | `start_scene` が存在しない scene を参照 | ❌ | シーン一覧出力のみ（Did you mean? は未実装） |
| E14 | parallel 内の禁止 action | ❌ | 禁止アクション名と禁止理由を表示するのみ |
| E15 | parallel.tasks が空 / 非配列 | ❌ | 構造エラーのみ |
| E16 | setup_rhythm_round の notes/taps 配列制約違反 | ❌ | 数値範囲/長さエラーのみ（SUGGEST なし） |
| E17 | scene 本体が mapping でない | ❌ | 型エラーのみ |
| E18 | on_enter が配列でない | ❌ | 型エラーのみ |

---

## 3. SUGGEST が出る条件（提案できること）

### 3-A. action 名の typo

`difflib.get_close_matches(cutoff=0.6)` を使用。

```
# 入力
- action: shov_dialogue

# 出力
ERROR ... Unknown action 'shov_dialogue'.
SUGGEST: Did you mean: 'show_dialogue'?  Valid actions: [...]
```

**適用範囲**: 全アクション名（ACTIONS dict の全キー）。

---

### 3-B. payload key の typo

```
# 入力
- action: record_billiards_event
  payload:
    eventt: shot_committed    # typo

# 出力
ERROR ... Unknown payload key 'eventt' for action 'record_billiards_event'.
SUGGEST: Did you mean: 'event'?  Known keys for 'record_billiards_event': ['event']
```

**適用範囲**: 全アクション（non-special の `all_known_keys` + special の `SPECIAL_KNOWN_KEYS`）。

---

### 3-C. 必須フィールド欠落

```
# 入力（id が欠落）
- action: add_evidence
  payload:
    type: item

# 出力
ERROR ... Missing required payload field 'id'.
SUGGEST: Add to payload:  id: <value>
```

OR グループ（いずれか一方が必須）の場合:

```
ERROR ... Missing required payload field: one of ['evidence_id', 'id'] must be present.
SUGGEST: Add one of these to payload: evidence_id: <value>  OR  id: <value>
```

---

### 3-D. enum 値の間違い

| アクション | フィールド | 有効値 |
|:---|:---|:---|
| `end_game` | `result` | `failed`, `lost`, `solved`, `timeout`, `wrong` |
| `add_evidence` | `type` | `achievement`, `discovery`, `dummy`, `evidence`, `item`, `motive`, `state`, `tool`, `trait` |
| `wait_for_billiards_event` | `events[]` の各要素 | `ball_pocketed`, `balls_stopped`, `cue_ball_pocketed`, `shot_committed` |
| `load_fake_player_command` | `command` | `attack`, `move_down`, `move_left`, `move_right`, `move_up` |

```
# 例
ERROR ... end_game 'result' ... got 'victry'.
SUGGEST: Did you mean: 'victory'?  Valid results: [...]
```

> `victory` は有効値ではなく正解は `solved` または `won`（実際には `solved`/`wrong`/`failed`/`lost`/`timeout`）。  
> cutoff=0.6 で一致しない場合は Did you mean 部分は省略され、Valid values リストのみ表示される。

---

### 3-E. 存在しない scene 参照

scene 参照エラーは以下のアクションで発生する:

| アクション | scene 参照フィールド |
|:---|:---|
| `check_evidence` | `if_true`, `if_false` |
| `check_condition` | `if_true`, `if_false` |
| `evaluate_billiards_round` | `if_clear`, `if_fail`, `if_continue` |
| `evaluate_roguelike_round` | `if_clear`, `if_fail`, `if_continue` |
| `evaluate_rhythm_round` | `if_clear`, `if_fail`, `if_continue` |

```
# 出力例
ERROR ... check_evidence 'if_true' references unknown scene 'win_scen'.
SUGGEST: Did you mean: 'win_scene'?  Available scenes: ['intro', 'win_scene', 'fail_scene']
```

---

### 3-F. timeout 値エラー

```
# ゼロ・負値
SUGGEST: timeout: 10.0  # seconds; omit to use the runtime default of 10.0

# 非数値
SUGGEST: timeout: 10.0  # numeric seconds
```

`wait_for_signal` のデフォルトは 5.0、`wait_for_billiards_event` のデフォルトは 10.0。

---

## 4. SUGGEST が出ない条件（提案できないこと）

以下のエラーは **SUGGEST なし**。理由を記す。

| エラー | 理由 |
|:---|:---|
| YAML 構文エラー | ブロック構造の修復は意味解析が必要でスコープ外 |
| `start_scene` / `scenes` キー欠落 | ファイル全体の再構成が必要 |
| `start_scene` が不明な scene を参照 | Did you mean? 未実装（将来候補） |
| `parallel` 内の禁止 action | 設計判断（並列実行の安全性）にかかわるため自動修正不可 |
| `setup_rhythm_round` の notes/taps 制約 | 要求値が文脈依存で適切なデフォルトが提示できない |
| scene body が mapping でない | YAML 構造の修復が必要 |
| WorldState の値の意味的妥当性 | 静的解析の限界（runtime 依存） |
| シナリオの到達可能性 | グラフ解析未実装（将来候補） |
| タスク実行順序の論理的整合性 | ゲームロジックが runtime に依存 |
| pos-0 sacrifice の未配置 | 制約は docs に記載されているが、静的検出の方法がなし |

---

## 5. Did you mean? の動作仕様

```python
difflib.get_close_matches(word, candidates, n=1, cutoff=0.6)
```

- `cutoff=0.6`: 60% 以上の類似度で一致とみなす。
- 完全一致に近い typo（1〜2文字違い）で反応する。
- `n=1`: 最も近い候補 1 件のみ表示。
- 一致なしの場合: `Did you mean?` 部分は省略し、valid values リストのみ表示。

**スコープ別の候補リスト**:

| スコープ | 候補リスト |
|:---|:---|
| action 名 | `ACTIONS.keys()`（全 26 アクション） |
| payload key（non-special） | そのアクションの `required` + `optional` キー全集合 |
| payload key（special） | `SPECIAL_KNOWN_KEYS[special_name]` |
| scene 参照 | その YAML ファイル内の全 scene ID |
| enum 値 | `VALID_*` 定数セット（action 別） |

---

## 6. AI 利用ガイド

### AI がシナリオを書いた後の標準フロー

```
1. python3 tools/validate_scenario.py <your_scenario.yaml>
2. ERROR が出たら SUGGEST を読んで修正する
3. エラーがなくなったら OK（exit 0）
4. それでも runtime で失敗する場合は SUGGEST 範囲外の問題（セクション4参照）
```

### SUGGEST を読む際の注意

- `Did you mean: 'X'?` は **候補を提示するだけ**。X が文脈的に正しいかは AI が判断する。
- `Available scenes: [...]` は YAML ファイル内に存在するシーン名。  
  参照したいシーンが一覧にない場合は、`scenes:` セクションに追加する必要がある。
- `Add to payload: key: <value>` の `<value>` は **プレースホルダー**。  
  実際の値は `docs/task_catalog.md` の各 Task エントリを参照して決定する。

### SUGGEST がない ERROR への対処

セクション4の「提案できないこと」に該当する。対処方法は:

| エラー種別 | 参照先 |
|:---|:---|
| YAML 構文エラー | `docs/SCENARIO_FORMAT.md` の基本構文 |
| `parallel` 内禁止 action | `docs/SCENARIO_FORMAT.md` の parallel 制約 |
| rhythm notes/taps 制約 | `examples/templates/time_clock_basic/contract.md` |
| pos-0 sacrifice | `docs/infrastructure_backlog.md` |
| ロジック的な誤り | `examples/templates/<module>/verification.md` |

---

## 7. 将来の拡張候補（スコープ外）

以下は現状未実装。実装する場合は `validate_scenario.py` の改修が必要。

| 機能 | 実装難度 | 価値 |
|:---|:---:|:---:|
| `start_scene` 参照エラーへの Did you mean? | 低 | 中 |
| 到達不能シーン（dead-end）の検出 | 中 | 中 |
| scene グラフのサイクル検出 | 中 | 低 |
| pos-0 sacrifice チェック（pos-0 がダミーか） | 高 | 高 |
| WorldState キーのクロスシーン整合性 | 高 | 高 |
| `setup_rhythm_round` notes/taps への具体的 SUGGEST | 中 | 中 |
