# Template Package Spec — Phase 3-A T1

Phase 3-A T1 の目的は、テンプレート実装の量産ではなく、
**テンプレート 1 本当たりの必須構成を定義すること**である。

---

## 1. Template とは何か

Template とは、Kernel v2.0 の梺組みの上で
AI が安全にゲームロジックを書けるようにする「冋形」である。

具体的には以下を指す:

- 最小構成の Task セット（src/games/配下）
- 最小構成の YAML シナリオセット
- WorldState contract（スコープ・キー・型の宣言）
- Validator 必須条件
- Runtime 検証パターン

Template は **src/core を変更しない**。全て src/games/ 配下の Task + YAML で実現する。

---

## 2. Template Package 最小構成

Template 1 本に必要な項目を以下に定義する。

### 2.1 Template purpose

テンプレートがカバーするゲームロジックの種別を 1 文で表現する。
例: "Kernel の条件分岐とフラグ管理を検証するメインテンプレート"

### 2.2 When to use / When not to use

使うべきケースと使うべきでないケースを各 3 行以内で列挫する。

### 2.3 Minimum Task set

テンプレートとして最下限必要な Task 名と役割を列挫する。
3～6 Task 程度。上限は 10 Task（超えた場合はテンプレート分割を検討）。

### 2.4 Minimum YAML structure

最小構成の YAML シナリオが持つべきシーングループ、サポートするパス（clear/fail/continue）を宣言する。

### 2.5 WorldState contract

テンプレートが使用する WorldState キーの全量を記述する。

| key | scope | type | description |
| :--- | :--- | :--- | :--- |
| `round:result` | SESSION | String | clear / fail / 空文字列 |
| ... | ... | ... | ... |

### 2.6 Validator expectations

`validate_and_setup()` で必ず検証するペイロード必須項目を列挫する。
各 Task につき 2～5 行。

### 2.7 Runtime verification pattern

テンプレート標準の smoke 検証パターンは以下の通り。

1. debug scene に `WorldState + GameNode + ScenarioRunner` を配置
2. debug overlay で WorldState 全キーを監視
3. `scripts/capture_rhythm_debug.gd` のようなスクリプトで自動メジャーショット or ログ出力
4. clear / fail / continue の 3 経路全てを確認

### 2.8 Completion memo requirements

完了メモは以下を包む（`roguelike_test_completion.md` の構成を準拠）。

1. Purpose — この template が何を検証するか
2. Verified scope — runtime で確認済みの範囲
3. WorldState contract の要点
4. Debug overlay の位置づけ
5. Intentionally out-of-scope
6. Pending items

完了メモは AI が読むことを前提に、短く・正確に書く。

---

## 3. テンプレート標準フォルダ構成（推奨）

将来的に `examples/templates/<template_name>/` に配置する前提で、
テンプレート 1 本の推奨構成は以下の**4 ファイル**で完結する。

```
examples/templates/<name>/
├── README.md          # Template purpose / when to use / when not to use
├── contract.md        # WorldState contract + validator expectations
├── scenario/          # 最小構成 YAML 1～2 本
└── verification.md    # Runtime 検証パターン + 確認済み実測値
```

### 各ファイルの対応関係

| ファイル | Template Package 項目 | AI 向け利用 |
| :--- | :--- | :--- |
| `README.md` | 2.1 purpose / 2.2 when to use | テンプレ選択 |
| `contract.md` | 2.5 WorldState / 2.6 validator | Task 設計時 |
| `scenario/` | 2.4 YAML / 2.3 Task set | YAML 拡張 |
| `verification.md` | 2.7 runtime / 2.8 completion | 完了判断 |

**注**: `src/games/<module>/` 配下の Task 実装コードは template パッケージに含めない。
AI は `src/games/<module>/` を参照しながら Task を拡張する。

---

## 4. 既存モジュールとのマッピング

### mystery_test → Branching Template

| 項目 | 内容 |
| :--- | :--- |
| Template の核 | 条件分岐（if_flag / set_flag）、シーン遷移（goto）、dialogue |
| モジュール固有（テンプレに入れない） | mystery 僋事ごとのフラグ名、公顔確認メカニズム、具体的なシナリオやテキスト |
| 完結メモ | 成果物は `docs/mystery_design.md`。completion memo は未作成 |

**この template の主軽**: 分岐を制御する Task ペア（`if_flag` / `set_flag` / `goto`）。

### roguelike_test → Turn/Grid Template

| 項目 | 内容 |
| :--- | :--- |
| Template の核 | ターン進行、エンティティ状態（HP/座標）、陣卦判定ロジック |
| モジュール固有（テンプレに入れない） | 5x5 マップ定義、具体的な enemy HP 値、ゴール座標 |
| 完結メモ | `docs/roguelike_test_completion.md` — 3 経路確認済み |

**この template の主軽**: `resolve_turn` / `evaluate_combat` 系の Task がターン進行の骨格。

### rhythm_test → Time/Clock Template

| 項目 | 内容 |
| :--- | :--- |
| Template の核 | KernelClock 進行、タイミングウィンドウ判定、乃求め入力注入 |
| モジュール固有（テンプレに入れない） | ノート定義配列内容、具体的な BPM／第数 |
| 完結メモ | `docs/rhythm_test_completion.md` — 3 経路確認済み |

**この template の主軽**: `advance_rhythm_clock` / `judge_rhythm_note` / `evaluate_rhythm_round` の Task トリオ。

---

## 5. 今回やらないこと

以下は Phase 3-A T1 のスコープ外である。

- テンプレート実装の量産（T2/T3/T4 は T1 完了後）
- generator 化（雛形から YAML を自動生成など）
- real input 統合（docs/design/ に設計メモのみ）
- UI polish / 演出エフェクト
- Event-Driven Template（`billiards_test`）の formalization（後段候補）
- テンプレ間の自動変換
- Task 実装の複製（源モジュールの Task はそのまま参照する）

---

## 6. 次の最小ステップ（T2）

### 推奨: `roguelike_test` → Turn/Grid Template を先に formalize

**理由**:

1. `roguelike_test_completion.md` が最も詳細に整備されており、template の雛形として成熟している。
2. ターン進行は SRPG / RPG 等多岐にわたり最も AI が再利用しやすいジャンルである。
3. mystery_test は completion memo が未作成なため、formalization 前にまず completion memo を整備する必要がある。
4. rhythm_test と比べて Task 種別が少なく、template の最山返し性が高い。

**Turn/Grid Template T2 成果物イメージ**:

```
examples/templates/turn_grid_basic/
├── README.md
├── contract.md
├── scenario/turn_grid_smoke.yaml
└── verification.md
```
