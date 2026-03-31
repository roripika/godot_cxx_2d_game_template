# Phase 3-C Generator Entry Memo

**ステータス**: 設計確定（実装未着手）  
**作成日**: 2026-04-01  
**前提**: Phase 3-B AI Assist Tooling すべて完了済み（`phase3_b_completion.md` 参照）

---

## 1. Purpose

AI が「テンプレート + 制約付き仕様」から **validator を通過する最小 YAML シナリオ** を生成できるようにする。

目標は完全自動生成ではない。  
**プロンプト → YAML 骨格 → human gate → validator → runtime** という短いサイクルを、  
既存 Assist Tooling の上に最小コストで乗せることが目的である。

---

## 2. Why now

Phase 3-B までで、以下が揃った。

| 資産 | 説明 |
|:---|:---|
| `task_catalog.md` | action 名・payload キーの唯一の正源（26 tasks） |
| `validate_scenario.py` | 18 エラー種別を SUGGEST 付きで検出 |
| `validate_scenario_guidance.md` | SUGGEST が出ないエラーの対処表 |
| `few_shot_prompts.md` | 4 テンプレート × 最小 YAML サンプル（validate 通過確認済み） |
| `template_selection_guide.md` | テンプレート間の差異・選択基準 |

**Generator は「これらの資産を入力に受け取る層」として初めて成立する。**  
資産がなければ Generator を作っても出力の品質が保証できない。今がその最小条件が揃った時点である。

---

## 3. Generator Scope（初期限定）

### 対象とすること

| 項目 | 内容 |
|:---|:---|
| **入力形式** | テンプレート選択済み + 制約付き仕様（構造化、自由文ではない） |
| **出力** | YAML 骨格（シーン構成・Task 列・分岐先のみ。値は仮埋め） |
| **検証目標** | `validate_scenario.py` exit 0 を目指す（自動修正ループはしない） |
| **対象テンプレート** | 初期は 1 テンプレートのみ（セクション 10 参照） |
| **ポータビリティ** | `pos-0 sacrifice` 等の既知 workaround を生成 YAML に自動的に埋め込む |

### 対象としないこと（Out of scope — セクション 4 で詳述）

- `src/core/` または `src/games/` の変更
- Task 実装クラスの自動生成
- runtime 自動修復ループ
- CI / commit / push の自動化
- UI / scene / debug overlay の生成
- real input 統合（物理エンジン・センサー等）
- 自由文そのままからのフルゲーム生成

---

## 4. Out of Scope

以下は **Phase 3-C の Generator では絶対に扱わない**。

| スコープ外 | 理由 |
|:---|:---|
| `src/core/` 変更 | Kernel は凍結。例外なし |
| `src/games/` への Task 実装追加 | 新 Task 追加は人間判断が必要な設計判断。Generator で自動化しない |
| runtime 自動修復（`exit 1` → 自動再生成ループ） | フィードバックループは Phase 3-C 後の検討項目 |
| 自由文入力（英日自然言語 → YAML 直接生成） | 構造化仕様なしでは validator 通過保証が困難 |
| 複数テンプレートをまたぐシナリオ生成 | テンプレート間遷移の設計が未定義 |
| commit / push 自動化 | human gate をバイパスする操作は禁止 |
| few_shot_prompts.md の自動更新 | few-shot 例は人間が品質確認してから追加する |
| task_catalog.md の自動更新 | gen_task_catalog.py で別管理。Generator は読み取り専用 |

---

## 5. Expected Inputs

Generator への入力は **構造化仕様（Structured Spec）** に限定する。  
自由文を受け取る場合は、Generator 呼び出し前に人間が構造化する（human gate 1）。

### 共通必須フィールド

```yaml
template: branching_basic     # 選択済みテンプレート（4 種のどれか）
scenario_name: my_game        # 出力 YAML のファイル名ベース

# ゲーム目標の制約
goal:
  clear_condition: |
    証拠 A と証拠 B が両方揃っている
  fail_condition: |
    証拠が 1 つでも欠けている
```

### テンプレート別追加フィールド

**Branching Basic 用:**
```yaml
branches:
  - id: evidence_a
    label: 「指紋」カード
  - id: evidence_b
    label: 「目撃証言」カード
check_type: all_of             # all_of / any_of
scenes:
  terminal_clear: arrest_scene
  terminal_fail:  release_scene
```

**Turn/Grid Basic 用:**
```yaml
grid: {width: 5, height: 5}
player: {hp: 3, start_x: 2, start_y: 2}
enemies:
  - {id: enemy_1, hp: 2, x: 3, y: 2}
win_condition: all_enemies_defeated
lose_condition: player_hp_zero
```

**Time/Clock Basic 用:**
```yaml
notes: [1000, 2000, 3000]      # ノーツ時刻（ms, 昇順, 3〜5要素）
taps:  [1000, 2000, 3000]      # FakeTap（smoke 用）
advance_ms: 1000
clear_hit_count: 3
max_miss_count: 0
```

**Event-Driven Basic 用:**
```yaml
shot_limit:   3
target_count: 1
timeout: 0.1                   # smoke 用（本番は 30.0）
win_event: ball_pocketed
```

### 入力の制約

- `notes` は 3〜5 要素、昇順必須（Generator 側で事前チェックする）
- `template` は 4 種のどれか（その他の値は即時拒否）
- 自由文フィールド（`clear_condition` 等）はコメントとして YAML に埋め込むのみ

---

## 6. Expected Outputs

### メイン出力: YAML 骨格

```
scenarios/generated/<scenario_name>_<timestamp>.yaml
```

- シーン名・Task 列・payload の仮値・分岐先シーン名を含む
- **pos-0 sacrifice を自動配置**（非 start_scene の全シーン）
- `validate_scenario.py` で exit 0 になることを目標とする
- 仮値（例: `player_hp: __FILL_IN__`）は `__FILL_IN__` プレースホルダーで明示

### サブ出力（任意）

| 出力物 | 説明 |
|:---|:---|
| `scenarios/generated/<scenario_name>_review.md` | 生成 YAML の意図・仮値箇所リスト・確認ポイント（人間向け） |

### 出力しないもの

- `.tscn` ファイル（Godot シーン）
- C++ ソースコード
- `src/` 配下のすべてのファイル

---

## 7. Human Gate Points

どこで人間確認を入れるかを定義する。  
Generator は human gate を **スキップする実装をしてはならない**。

```
[HG-1] Template 選択後 ──────────────────────────────────
   入力: template 名 + goal 文
   確認: 「このテンプレートで本当に目標を達成できるか？」
   承認後: 構造化仕様の記述へ進む
   根拠: template_selection_guide.md のクイック選択フロー

[HG-2] YAML 生成後 ──────────────────────────────────────
   入力: 生成済み YAML ファイル
   確認: シーン構成・Task 列・分岐先が意図と合っているか
        __FILL_IN__ プレースホルダーを実値に置き換える
   承認後: validate_scenario.py を実行

[HG-3] Validator 実行後 ─────────────────────────────────
   入力: validate 結果（exit 0 or exit 1 + SUGGEST）
   確認: exit 1 の場合は SUGGEST を読んで手動修正
        validate_scenario_guidance.md を参照
   承認後: runtime 実行へ進む

[HG-4] Runtime 実行前 ───────────────────────────────────
   入力: validate 通過済み YAML
   確認: Godot で開いて煙テスト（3 経路: clear / fail / continue）
        pos-0 sacrifice が機能しているか目視確認
   承認後: 正式シナリオとして scenarios/ に移動 / commit
```

---

## 8. Validation / Review Flow

```
[入力仕様]
     │
     ▼
[Generator（YAML 骨格生成）]
     │  pos-0 sacrifice 自動埋め込み
     │  __FILL_IN__ プレースホルダー挿入
     ▼
[HG-2: 人間レビュー]
     │  プレースホルダーを実値に置き換え
     ▼
[validate_scenario.py]
     │  exit 0?
     │    NO  ─→ SUGGEST 確認 ─→ 手動修正 ─→ 再 validate
     ▼ YES
[HG-3: validate 結果確認]
     ▼
[runtime 実行（Godot）]
     │  煙テスト 3 経路
     ▼ PASS
[HG-4: 最終確認]
     ▼
[scenarios/ に配置 / commit]
```

**Generator が自動修正ループに入ることは禁止。**  
`validate_scenario.py` exit 1 の修正は常に HG-3 で人間が行う。

---

## 9. Risks and Non-Goals

### リスク

| リスク | 対策 |
|:---|:---|
| Generator が `VALID_ACTIONS` の更新を見落とす | task_catalog.md を読み取り専用入力とし、Generator 自身は VALID_ACTIONS を持たない |
| pos-0 sacrifice の自動配置が誤る | HG-2 で目視確認を義務化。Time/Clock は「非 boot 全シーン」のルールを固定 |
| `__FILL_IN__` の未置換が validate を通過する | Generator が出力する review.md にプレースホルダー一覧を掲載する |
| テンプレート選択の誤り | HG-1 を template_selection_guide.md と連動させ、選択根拠をレビューカードに記録 |
| 生成 YAML が scenarios/ に混入する | 生成物は `scenarios/generated/` に隔離。人間が明示的に移動するまで正式品にならない |

### Non-Goals（このメモが解決しようとしないこと）

- Generator が常に validator を通過する YAML を生成できることの保証
- runtime 実行時のすべての問題を静的に予防すること
- 人間よりも速くシナリオを生成すること
- 既存シナリオ（mystery / billiards / roguelike / rhythm）のリファクタリング

---

## 10. Suggested First Implementation Target

### 推奨: **Branching Basic**

4 テンプレートのうち最初に Generator 化するなら Branching Basic を推奨する。

**理由:**

| 理由 | 詳細 |
|:---|:---|
| **構造が最小** | シーン数 3 / ループなし / Task 数 6 / pos-0 sacrifice は terminal 2 シーンのみ |
| **KernelClock 不使用** | 時刻計算のない構造化仕様で済む |
| **FakeCommand 系不要** | WorldState に直接書き込む形式。入力モック生成が不要 |
| **入力→出力の対応が明確** | `evidence_id` リスト + `check_type(all_of/any_of)` → YAML の 1:1 マッピングが成立する |
| **few_shot_prompts.md の Shot 1 がそのまま参照例になる** | in/out ペアが確認済みで、Generator の出力品質基準として使える |
| **validate リグレッション対象** | mystery_test との対応が明確で、生成 YAML の検証も容易 |

**他テンプレートを選ばない理由:**

| テンプレート | 見送り理由 |
|:---|:---|
| Turn/Grid Basic | `load_fake_player_command` の command 値・座標系のバリデーションが複雑 |
| Time/Clock Basic | 6 シーン全 pos-0 sacrifice + notes/taps 長一致 + 昇順制約が Generator の初期実装に重い |
| Event-Driven Basic | `TaskResult::Waiting` + タイムアウト挙動の理解が必要。最後に着手が妥当 |

---

## 11. 既存 Assist Tooling との役割分担

Generator は **既存資産の上に乗る薄い層** として定義する。独自ルールを持たない。

```
┌─────────────────────────────────────────────────┐
│  Generator (Phase 3-C)                          │
│  入力: Structured Spec                          │
│  出力: YAML 骨格 + review.md                    │
│  依存: 下記の既存資産をすべて読み取り専用で使う  │
└─────────────────────────────────────────────────┘
          │ 読み取り専用
          ▼
┌─────────────────────────────────────────────────┐
│  Phase 3-B AI Assist Tooling                    │
│                                                 │
│  template_selection_guide.md                    │
│    → テンプレート名の有効値リスト（HG-1 の根拠）│
│                                                 │
│  task_catalog.md                                │
│    → action 名・payload キーの正源              │
│    → Generator はここから YAML を組み立てる     │
│                                                 │
│  few_shot_prompts.md                            │
│    → YAML 骨格の形式見本（生成後の自己チェック）│
│    → よくある間違いを事前回避するルールに転用   │
│                                                 │
│  validate_scenario_guidance.md                  │
│    → review.md に「よくある修正ポイント」として │
│       HG-3 向けに引用する                       │
└─────────────────────────────────────────────────┘
          │ 不変
          ▼
┌─────────────────────────────────────────────────┐
│  Kernel / Task 実装（凍結）                     │
│  src/core/ — 変更禁止                           │
│  src/games/ — Generator は読まない              │
└─────────────────────────────────────────────────┘
```

### 明確な分担ルール

| 資産 | Generator の扱い |
|:---|:---|
| `task_catalog.md` | action 名・payload 構造の唯一の参照源。Generator が独自に構造を定義しない |
| `template_selection_guide.md` | 有効な template 名リストの根拠。入力バリデーションに使う |
| `few_shot_prompts.md` | 生成後の形式チェック基準。**Generator への入力としては使わない**（循環参照を避ける） |
| `validate_scenario_guidance.md` | review.md への引用元。Generator は修正しない |
| `validate_scenario.py` | Generator の成否判定に使う。exit 0 = 骨格正常、exit 1 = HG-3 へ |

---

## 次の最小ステップ（実装着手前の確認事項）

このメモを承認後、実装に進む前に以下を確認する。

1. **Structured Spec の YAML スキーマを 1 枚定義する**  
   - `docs/generator_spec_schema.md`（branching_basic 用のみ）  
   - 必須フィールド / 省略可フィールド / バリデーション規則を列挙

2. **`scenarios/generated/` ディレクトリを作成する**  
   - 生成物の隔離場所を事前に確保する  
   - `.gitkeep` のみ置く（生成物は gitignore するか人間が判断）

3. **few_shot_prompts.md Shot 1 の YAML を「期待出力サンプル」として確定させる**  
   - Generator の出力品質基準として commit 済みのものを使う  
   - 追加の検証は不要（validate 通過確認済み）

4. **このメモを `docs/README.md` 層 B に追加する**  
   - フェーズ移行時のみ参照。常時参照には含めない

---

## 参照ドキュメント

| ファイル | 役割 |
|:---|:---|
| `docs/phase3_b_completion.md` | Phase 3-B 資産の到達点・限界（この Memo の前提） |
| `docs/template_selection_guide.md` | テンプレート選択基準・HG-1 の根拠 |
| `docs/task_catalog.md` | Action 名・Payload 構造（Generator の組み立て基盤） |
| `docs/few_shot_prompts.md` | 最小 YAML サンプル（形式見本・品質基準） |
| `docs/validate_scenario_guidance.md` | HG-3 向けエラー対処表 |
| `docs/infrastructure_backlog.md` | pos-0 skip 等の既知 workaround |
| `docs/phase3_planning.md` | Phase 3 全体ロードマップ |
