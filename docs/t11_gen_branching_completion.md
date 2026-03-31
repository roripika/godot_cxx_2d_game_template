# T11 完了メモ — gen_scenario_branching.py

**ステータス**: 完了 ✅  
**作成日**: 2026-04-01  
**commit**: `cb77704`  
**ファイル**: `tools/gen_scenario_branching.py` (433 行)

---

## 概要

`generator_spec_schema.md` v1.0 に沿った Structured Spec YAML を受け取り、  
`scenarios/generated/` 配下に Branching Basic の YAML 骨格と review.md を出力する。

```
[spec.yaml] → gen_scenario_branching.py → [<name>_<ts>.yaml]
                                        → [<name>_<ts>_review.md]
```

---

## 入力スコープ

| 項目 | 内容 |
|:---|:---|
| 対応テンプレート | `branching_basic` のみ。その他の値は V-01 で即時拒否 |
| 入力形式 | YAML ファイル（Structured Spec）。自由文は非対応 |
| 必須フィールド | `template` / `scenario_name` / `branches`（2〜4要素）/ `check_type` / `scenes.terminal_clear` / `scenes.terminal_fail` |
| 省略可フィールド | `goal.*`（コメント用）/ `branches[].label`（review.md 用）/ `branches[].location`（省略時は自動補完）/ `scenes.start`（省略時: `investigation`）/ `terminal_result.*`（省略時: `solved` / `wrong`）|
| コマンドライン | `python3 tools/gen_scenario_branching.py <spec.yaml> [--out-dir <dir>]` |

---

## 自動補完の範囲（Generator が無条件で行うこと）

| 自動補完項目 | ルール |
|:---|:---|
| `branches[].location` 省略時 | `{id}_location` で自動生成 |
| `scenes.start` 省略時 | `investigation` |
| `terminal_result.clear` 省略時 | `solved` |
| `terminal_result.fail` 省略時 | `wrong` |
| pos-0 sacrifice | terminal シーン全てに `end_game` × 2 を自動配置（start シーンは対象外）|
| タイムスタンプ付きファイル名 | `<scenario_name>_<YYYYMMDD_HHMMSS>.yaml` |

**自動補完しないもの（spec で明示が必要なもの）:**
- `branches[].id`（証拠 ID）
- `check_type`（`all_of` / `any_of` の選択）
- `scenes.terminal_clear` / `scenes.terminal_fail`（シーン名）

---

## Fail-Fast バリデーション（V-01〜V-11）

いずれか 1 件でも失敗すると **YAML を生成せず exit 1** で停止する。

| 規則 | チェック内容 |
|:---|:---|
| V-01 | `template` が `branching_basic` |
| V-02 | `scenario_name` が `[a-z0-9_]` 1〜40 文字 |
| V-03 | `branches` が 2〜4 要素 |
| V-04 | `branches[].id` が `[a-z0-9_]` 1〜30 文字 |
| V-05 | `branches[].id` がリスト内で重複しない |
| V-06 | `check_type` が `all_of` または `any_of` |
| V-07 | `scenes.terminal_clear` / `terminal_fail` が両方指定されている |
| V-08 | `terminal_clear` ≠ `terminal_fail` |
| V-09 | `scenes.start` が terminal シーンと異なる |
| V-10 | `terminal_result.*` が有効な result 値 |
| V-11 | `branches[].location` が指定された場合 `[a-z0-9_]` |

---

## Human Gate 依存箇所

Generator は Human Gate を **スキップする実装を持たない**。  
以下は Generator の責務外であり、常に人間が行う。

| Gate | タイミング | 依存理由 |
|:---|:---|:---|
| **HG-1** | spec 作成前（Generator 呼び出し前） | template 選択の妥当性はゲーム設計の判断 |
| **HG-2** | YAML 生成直後 | `branches[].location` の自動補完値が意図通りか、シーン名が正しいかは人間検証 |
| **HG-3** | validate 実行後 | `validate_scenario.py` exit 1 の修正は人間が行う（自動修正ループなし）|
| **HG-4** | runtime 実行前 | pos-0 sacrifice の実動作確認は Godot が必要 |

Generator の完了メッセージは必ず HG-2→HG-3 の手順を案内する。

---

## smoke テスト結果（2026-04-01）

| ケース | spec の内容 | 期待動作 | 結果 |
|:---|:---|:---|:---:|
| 最小構成 | 必須フィールドのみ（2 証拠 / all_of） | exit 0 / validate exit 0 | ✅ |
| 推奨構成 | goal / label / location 全指定（museum_heist） | exit 0 / validate exit 0 / 期待出力サンプルと構造一致 | ✅ |
| V-02 エラー | `scenario_name: "My Scenario!"` | exit 1 / YAML 未生成 | ✅ |
| V-06 エラー | `check_type: both` | exit 1 / YAML 未生成 | ✅ |
| V-08 エラー | terminal_clear = terminal_fail = `win_scene` | exit 1 / YAML 未生成 | ✅ |

**期待出力サンプルとの diff（コメント・空行除く）**: 差分なし ✅

---

## 出力ファイルの位置づけ

| ファイル | 位置づけ | git add のタイミング |
|:---|:---|:---|
| `<name>_<ts>.yaml` | YAML 骨格。`__FILL_IN__` なし（全値が spec から確定） | HG-4 通過後に人間が手動で行う |
| `<name>_<ts>_review.md` | HG-2 チェックリスト。証拠フラグ一覧・確認ポイント・HG-3 コマンドを掲載 | 通常はバージョン管理しない（作業メモ） |

---

## 既知制約

| 制約 | 内容 |
|:---|:---|
| **branches 4 件超は非対応** | V-03 で拒否。5 件以上の証拠 ID が必要なゲームには不適 |
| **start シーン名は 1 つのみ** | 複数 start フローや conditional start は非対応 |
| **any_of の「n 件中 m 件以上」は非対応** | `any_of` は「1 件以上が true」の固定動作。閾値指定は Kernel レベルの拡張が必要 |
| **シーン間の複数ホップ分岐は非対応** | investigation → sub_scene_1 → terminal という中間シーンを含む構造は手書きが必要 |
| **review.md はバージョン管理の対象外が原則** | タイムスタンプ付きで生成されるため、commit 対象は個別判断 |
| **validate_scenario.py の VALID_ACTIONS との乖離** | Generator は action 名を文字列リテラルで出力。`VALID_ACTIONS` 更新時は Generator 側の変更不要だが、validate で発見されるまで乖離に気づかない |
| **PyYAML 依存** | `.temp_venv` 外では `pip install pyyaml` が別途必要 |

---

## 次段への判断材料

### このタスクで得られた知見

- Branching Basic は構造が最小（3 シーン / ループなし）のため、Generator 化が最も容易だった
- spec → YAML の 1:1 マッピングが明確で、曖昧な変換ルールが発生しなかった
- `branches[].label` は YAML に出力する必要がなく、review.md 専用で十分だった

### 次の選択肢

| 候補 | 内容 | 優先度の根拠 |
|:---|:---|:---|
| **A: review.md 位置づけ整理** | review.md を常時参照しない補助出力として docs に明文化 | Generator を使うたびに review.md が増え続けるため、`.gitignore` 方針を確定する必要がある |
| **B: Turn/Grid Basic 入口設計** | `generator_spec_schema.md` に Turn/Grid 用フィールドを追加し、次の Generator の設計メモを作成 | Branching の次に構造が明確。FakeCommand の command 値が固定なので spec 設計が容易 |

**推奨: A → B の順**  
review.md の扱いを決めずに Generator を増やすと、`scenarios/generated/` が作業ゴミで汚染される。  
A（review.md 方針 + `.gitignore` 整理）を 1 タスクで片付けてから B に進むのが最小リスク。

---

## 参照ドキュメント

| ファイル | 役割 |
|:---|:---|
| `docs/generator_spec_schema.md` | 入力スキーマ定義（V-01〜V-11 の正源） |
| `docs/phase3_c_generator_entry.md` | Generator 全体方針・HG 定義 |
| `scenarios/generated/branching_basic_expected_output.yaml` | 期待出力サンプル（品質基準） |
| `docs/few_shot_prompts.md` | Shot 1（review.md の「よくある間違い」の出典） |
