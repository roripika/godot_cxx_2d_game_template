# Phase 3-C 完了レポート — 4 Generator 到達点

**フェーズ**: Phase 3-C (Scaffold Generator Design & Implementation)  
**完了日**: 2026-04-01  
**対象**: T11〜T14（branching / turn_grid / time_clock / event_driven の 4 Generator）

---

## 1. 到達点サマリー

| # | タスク | Generator | template | commit | バリデーション規則 | 行数 |
|:---:|:---|:---|:---|:---:|:---:|:---:|
| T11 | gen_scenario_branching | `tools/gen_scenario_branching.py` | `branching_basic` | `cb77704` | V-01〜V-11 | 433 |
| T12 | gen_scenario_turn_grid | `tools/gen_scenario_turn_grid.py` | `turn_grid_basic` | `ffff729` | V-TG-01〜V-TG-09 | 390 |
| T13 | gen_scenario_time_clock | `tools/gen_scenario_time_clock.py` | `time_clock_basic` | `d258134` | V-TC-01〜V-TC-12 | ~380 |
| T14 | gen_scenario_event_driven | `tools/gen_scenario_event_driven.py` | `event_driven_basic` | `bde85b1` | V-ED-01〜V-ED-12 | 370 |

全 Generator の smoke テスト（エラー系 + 正常系 + headless HG-4）を通過済み。  
`validate_scenario.py` の exit 0 も全件確認済み。

---

## 2. 共通アーキテクチャ

4 Generator は全く同じパイプラインで動作している。

```
Structured Spec YAML
        ↓
  [1] YAML 読み込み
        ↓
  [2] Fail-Fast バリデーション（V-XX-YY）
        ↓  ← エラー時: exit 1 / YAML 未生成
  [3] YAML 骨格生成（Action リスト）
        ↓
  [4] review.md 生成（HG チェックリスト）
        ↓
  scenarios/generated/<name>_<ts>.yaml
  scenarios/generated/<name>_<ts>_review.md
```

### 全 Generator が共通して実装していること

| 共通要素 | 内容 |
|:---|:---|
| Structured Spec YAML 入力 | 自由文不可。`template` フィールドで Generator を選択 |
| Fail-Fast バリデーション | 1 件でも失敗 → YAML 未生成 / exit 1 |
| タイムスタンプ付きファイル名 | `<name>_YYYYMMDD_HHMMSS.yaml` |
| `validate_scenario.py` との連携 | 生成後に手動実行を review.md で案内 |
| review.md 出力 | HG-2 チェックリスト + HG-3 コマンド自動挿入 |
| `--out-dir` オプション | 出力先ディレクトリを変更可能 |

---

## 3. pos-0 sacrifice の配置規則（全 Generator 統一）

pos-0 skip（`load_scene_by_id` → `start_actions(0)` → while ループ skip）への対処として、以下のルールが全 Generator で統一されている。

| シーン種別 | sacrifice 要否 | 根拠 |
|:---|:---:|:---|
| **boot**（`_ready()` 経由で遷移） | **不要** | `_ready()` は step 外。while ループが pos-0 から正常実行される |
| **continue**（evaluate → `load_scene_by_id`） | **必要 × 2** | pos-0 が skip → pos-1 が actual。sacrifice で pos-0 を埋める |
| **terminal**（evaluate → `load_scene_by_id`） | **必要 × 2** | 同上。`end_game` × 2 でガード |
| **boot ループ再突入**（turn_grid の boot→boot） | **不要** | R-1 確認済み。`setup` 系 Task が pos-0 skip で再実行されず状態保持になる設計内動作 |

**例外: time_clock の中間シーン（advance / judge / resolve / loop_gate）**  
全てが evaluate → `load_scene_by_id` 経由で遷移するため、全て sacrifice × 2 が必要。

---

## 4. Human Gate 依存箇所（4 Generator 共通）

Generator は Human Gate を**自動通過する機能を持たない**。以下は全 Generator 共通の Gate 構造。

| Gate | タイミング | 依存理由 |
|:---|:---|:---|
| **HG-1** | spec 作成前 | template 選択・ゲームロジック設計は人間の判断 |
| **HG-2** | YAML 生成直後 | 自動補完値の意図確認・シーン名の妥当性確認 |
| **HG-3** | validate 実行後 | `validate_scenario.py` exit 1 の修正は人間が行う |
| **HG-4** | Godot headless 実行 | pos-0 sacrifice の実動作・WorldState フロー確認 |

Generator は HG-2 → HG-3 コマンドを review.md に自動挿入するが、**通過するかどうかは人間が決定する**。

---

## 5. 各テンプレートの特徴と制約

### branching_basic（T11）

- **特徴**: 証拠 ID リスト → `check_type`（all_of/any_of）で条件分岐
- **boot sacrifice**: 不要（start シーン専用）
- **terminal sacrifice**: 必要
- **HG 依存**: `branches[].location` の自動補完値（`{id}_location`）がシーン名として適切かどうか

### turn_grid_basic（T12）

- **特徴**: boot ループ（continue = boot）+ attack/move コマンドの自動判別
- **boot sacrifice**: 不要（boot ループ再突入時も setup が pos-0 skip で再実行されないことがクリーンな設計）
- **terminal sacrifice**: 必要
- **HG 依存**: `first_command` が `attack` か `move_*` かでアクション列が変わるため、ゲームフロー設計を先に決める必要がある

### time_clock_basic（T13）

- **特徴**: advance/judge/resolve/loop_gate の 4 中間シーンと terminal を全自動生成。最も Action 列が多い
- **sacrifice**: 5 シーン（advance/judge/resolve/loop_gate/terminal×2）全てで必要
- **HG 依存**: `notes` / `taps` 配列のタイミング値が音楽仕様と合致しているかは人間確認が必須。ノーツ数 ≤ 5 の制限（`SetupRhythmRoundTask::kMaxNotes`）

### event_driven_basic（T14）

- **特徴**: `TaskResult::Waiting` ポーリングを含む唯一のテンプレート。continue シーンに wait × 2 自動配置
- **sacrifice**: continue に wait × 2（sacrifice + actual）、terminal に end_game × 2
- **HG 依存**: `wait_timeout` は headless smoke 用の短い値。実ゲームでは物理エンジンからの実イベントシグナル連携が必要（Kernel 外）

---

## 6. 4 Generator の限界（共通）

| 限界事項 | 内容 |
|:---|:---|
| **単テンプレート入力のみ** | 1 spec YAML = 1 template。複数テンプレートの組み合わせは非対応 |
| **VALID_ACTIONS との乖離検出なし** | Generator は action 名を文字列リテラルで出力。`VALID_ACTIONS` 追加時に validate で初めて乖離が判明する |
| **WorldState キー名のハードコード** | `round:shots_taken` 等のキー名を Generator 内に直書き。Task 側のキー変更時は Generator も変更が必要 |
| **中間ロジックの自動生成は非対応** | 条件分岐が複数ホップ（scene_A → scene_B → terminal）になる構造は手書きが必要 |
| **review.md の git 管理は原則対象外** | タイムスタンプ付きで生成。commit 対象は個別判断 |
| **テンプレート外ロジックの拒否** | template フィールドの値が 4 種以外の場合は全て exit 1 で拒否。新テンプレートは Generator の追加が必要 |

---

## 7. 次段候補

以下の 3 方向から選択する。整理のため優先度の参考を付記する。

### 方向 A: 共通化（Generator を統一インターフェースで束ねる）

- `tools/gen_scenario.py`（ディスパッチャ）を新設し、`template` フィールドを見て各 Generator に委譲
- メリット: 呼び出しコマンドが 1 本に統一される
- デメリット: 各 Generator のバリデーション仕様が異なるため統合が複雑。現時点では過早
- **推奨度**: 低（4 Generator が安定してから）

### 方向 B: 入力改善（Spec YAML スキーマを強化）

- Generator ごとに異なる入力フィールド名を統一化（例: `setup.*` を全テンプレートで統一）
- JSON Schema または Pydantic による Spec YAML ビジュアル検証ツール追加
- review.md の HG チェックリスト内容を充実させる（シーン名衝突チェックなど）
- **推奨度**: 中（HG-2 の品質が直接 Generator の信頼性に影響する）

### 方向 C: 統合テスト（T11〜T14 全 Generator を通した品質確認）

- `tests/test_generators.sh` 等で 4 Generator の smoke テストを自動化
- `scenarios/generated/` の期待出力サンプルとの diff チェック
- CI 相当の一括実行スクリプト
- **推奨度**: 高（現在は手動 smoke のみ。Generator の退行検出がない）

### 方向 D: 次テンプレート対応（Phase 3-C の continuation）

- 未実装テンプレートがあれば T15 として spec schema → Generator の手順を繰り返す
- **推奨度**: 現時点では新テンプレートの需要がないため保留

---

## 8. 関連ドキュメント

| ドキュメント | 内容 |
|:---|:---|
| [`docs/generator_spec_schema.md`](generator_spec_schema.md) | 全 Generator の Spec スキーマ定義（T11〜T14 節） |
| [`docs/t11_gen_branching_completion.md`](t11_gen_branching_completion.md) | branching 詳細 |
| [`docs/t12_gen_turn_grid_completion.md`](t12_gen_turn_grid_completion.md) | turn_grid 詳細 |
| [`docs/t13_gen_time_clock_completion.md`](t13_gen_time_clock_completion.md) | time_clock 詳細 |
| [`docs/t14_gen_event_driven_completion.md`](t14_gen_event_driven_completion.md) | event_driven 詳細 |
