# Template Selection Guide

**対象テンプレート**: Branching Basic / Turn/Grid Basic / Time/Clock Basic  
**Phase 3-A 横断レビュー** — 2026-03-27 作成

---

## 0. TL;DR クイック選択フロー

```
ゲームの主軸は何か？
│
├─ 「時間（ms）」で入力と定刻イベントを比較する（リズム・QTE）
│    └─→  Time/Clock Basic
│
├─ 「証拠・フラグ」を集め、その組み合わせで結末が分岐する（ADV・推理）
│    └─→  Branching Basic
│
└─ 「ターン制・グリッド・HP」で勝敗を管理する（RPG・ローグライク）
     └─→  Turn/Grid Basic
```

---

## 1. 横断比較表

| 項目 | Branching Basic | Turn/Grid Basic | Time/Clock Basic |
| :--- | :--- | :--- | :--- |
| **参照実装** | `mystery_test` | `roguelike_test` | `rhythm_test` |
| **完了メモ** | `docs/mystery_test_completion.md` | `docs/roguelike_test_completion.md` | `docs/rhythm_test_completion.md` |
| **シーン数（smoke）** | **3** | **3** + ループ | **7** + ループ |
| **シーン構成** | investigation / solved_scene / wrong_scene | boot / clear / fail | boot / advance / judge / resolve / check_round / clear / fail |
| **Task 数（最小）** | 6 | 7 | 6 |
| **WorldState スコープ** | **SESSION + GLOBAL（デュアル）** | SESSION のみ | SESSION のみ |
| **KernelClock** | 不使用 | 不使用 | **使用（必須）** |
| **FakeCommand 系** | 不要 | `load_fake_player_command` | `load_fake_tap` |
| **ループ構造** | なし（1 回で終結） | boot へ戻るループ | advance → judge → resolve → check_round のループ |
| **pos-0 sacrifice 対象** | 2 シーン（endings） | 2 シーン（clear / fail） | **6 シーン（非 boot 全て）** |
| **分岐経路** | solved / wrong | clear / fail / continue | clear / fail / continue |
| **実装難易度** | ★☆☆ | ★★☆ | ★★★ |

---

## 2. 共通項目（3 テンプレート全て）

これらは Kernel の設計原則として全テンプレートに共通します。

| 共通原則 | 説明 |
| :--- | :--- |
| **src/core 変更禁止** | Kernel コアロジックは凍結。Task / YAML のみで拡張する |
| **Task 単一責務** | 1 Task は 1 つの責務だけを持ち、WorldState 読み書き以外の副作用を持たない |
| **分岐点 1 箇所集約** | シーン遷移は evaluate 系 Task 1 点に集約。Task が直接シーンを呼ばない |
| **Fail-Fast** | `validate_and_setup()` で payload を即時検証。不正値は早期 return |
| **pos-0 sacrifice** | 非 start シーンの pos-0 はスキップ既知バグ回避のため sacrifice を置く |
| **決定論的 smoke** | FakeCommand / FakeTap による再現可能シナリオで 3 経路（または 2 経路）を確認する |
| **4 点セット構成** | README / contract / scenario / verification の 4 ファイルで 1 テンプレートを形成 |
| **Scene 分岐は payload で指定** | `if_true:` / `if_false:` / `if_clear:` / `if_fail:` / `if_continue:` 等のキーで分岐先シーン名を YAML に書く |

---

## 3. 各テンプレートの識別特徴

### 3-1. Branching Basic ← これを選ぶ判断材料

**識別キーワード**: 証拠・フラグ・条件分岐・複数エンディング・ノーループ

- **省力点**: FakeCommand が不要。プレイヤー入力がない決定論的モデル
- **特有の複雑さ**: WorldState がデュアルスコープ（SESSION + GLOBAL）  
  → `discover_evidence` と `add_evidence` / `check_condition` と `check_evidence` の 2 ペアを使い分ける必要がある
- **ループしない**: 証拠収集が完了すると必ず ending に至る。「continue で boot に戻る」パターンがない
- **Task の役割が宣言的**: YAML が「何を発見するか」を制御し、Task は証拠の書き込みと条件評価に集中する

**向くゲームジャンル**: ADV・ビジュアルノベル・推理ゲーム・エスケープルーム・ポイントアンドクリック

---

### 3-2. Turn/Grid Basic ← これを選ぶ判断材料

**識別キーワード**: ターン制・グリッド座標・HP・エンティティ・敵行動

- **省力点**: KernelClock 不要。時間概念がなくシンプル
- **特有の複雑さ**: FakeCommand で入力モデルを構築する。エンティティ（player / enemy）が複数存在する
- **ループする**: boot を中心に「1 ターン処理 → 勝敗チェック → boot に戻る」のループを形成
- **Task チェーンが長い**: `setup → load_command → move → attack → enemy_turn → resolve → evaluate` と順番に Task を実行する

**向くゲームジャンル**: ローグライク・チェス・戦略 RPG・パズル（手番制）・ダンジョン探索

---

### 3-3. Time/Clock Basic ← これを選ぶ判断材料

**識別キーワード**: KernelClock・ms 精度・timing window・ノーツ・QTE

- **必須要素**: KernelClock を利用する唯一のテンプレート。`advance` と `now` を Task から明示的に呼ぶ
- **特有の複雑さ**: 7 シーン構成で pos-0 sacrifice が 6 シーン分必要。1 つでも欠けると判定がずれる
- **ループが多段**: advance → judge → resolve → check_round の 4 シーンを 1 ノーツごとに周回する
- **早期終了 fail**: `miss_count > max_miss_count` で全ノーツ消化を待たずに即 fail する（ループが途中終了する）

**向くゲームジャンル**: リズムゲーム・音ゲー・QTE・チャージアタック・タイミング判定・ジャスト押し

---

## 4. 差分深掘り：選択ミスを防ぐポイント

### 「FakeCommand か FakeTap か、それとも不要か」

| テンプレート | 入力モデル | YAML 上の表現 |
| :--- | :--- | :--- |
| Branching | **入力なし**（YAML が証拠配置を宣言） | `discover_evidence` を `on_enter` に並べる |
| Turn/Grid | **FakeCommand**（コマンド文字列） | `load_fake_player_command` の `command: move_down` |
| Time/Clock | **FakeTap**（タップ時刻の配列） | `setup_rhythm_round` の `taps: [-1, 1000, 2000, ...]` |

### 「WorldState のスコープが肝になる場面」

Branching だけが SESSION / GLOBAL の 2 スコープを使います。  
「証拠が揃っているのに条件評価が常に false」はほぼ必ずスコープ混用が原因です。

| やりがちミス | 原因 | 対処 |
| :--- | :--- | :--- |
| `check_condition` で常に false | GLOBAL 証拠を SESSION 評価 Task で読んでいる | `check_evidence`（GLOBAL 対応）に切り替える |
| `check_evidence` で常に false | SESSION 証拠を GLOBAL 評価 Task で読んでいる | `check_condition`（SESSION 対応）に切り替える |

Turn/Grid と Time/Clock は SESSION のみなのでこの問題は起きません。

### 「pos-0 sacrifice の影響範囲」

| テンプレート | sacrifice 必要数 | 最も厄介な落とし穴 |
| :--- | :--- | :--- |
| Branching | 2（solved_scene / wrong_scene） | ending だけなので影響が小さい |
| Turn/Grid | 2（clear / fail） | ending だけなので影響が小さい |
| **Time/Clock** | **6（advance / judge / resolve / check_round / clear / fail）** | **ループ内の中間シーンに sacrifice を忘れると、クロックが半進み・判定スキップが発生** |

---

## 5. ゲームジャンル → テンプレート マッピング表

| ジャンル例 | 推奨テンプレート | 理由 |
| :--- | :--- | :--- |
| 推理ゲーム・脱出ゲーム | **Branching** | 証拠収集 + 分岐エンディング |
| ビジュアルノベル (AGE) | **Branching** | 旗（フラグ）分岐に直接対応 |
| ローグライク・ダンジョン探索 | **Turn/Grid** | グリッド移動 + HP + ターン制 |
| チェス・ボードゲーム | **Turn/Grid** | 座標管理 + 手番制 |
| リズムゲーム・音ゲー | **Time/Clock** | ノーツ時刻 + timing window |
| QTE・タイミングアクション | **Time/Clock** | ms 精度の一致度評価 |
| タワーディフェンス (ターン制) | **Turn/Grid** | ターン単位の配置・攻撃 |
| デッキ構築ローグ（カード） | **Turn/Grid** + custom | Turn をカードドロー単位に解釈 |
| サスペンスADV + 時間制限QTE | **Time/Clock** + Branching 参照 | QTE を Time/Clock で、分岐を Branching パターンで |

---

## 6. 実装工数見積もりガイド

Kernel 上に新しいゲームを実装する際の工数の目安です（Task 1 本 = 1 日を単位に）。

| テンプレート | 最小 Task 数 | 参照実装の実績 |
| :--- | :--- | :--- |
| Branching | 6 | mystery_test: 6 Tasks |
| Turn/Grid | 7 | roguelike_test: 7 Tasks |
| Time/Clock | 6 | rhythm_test: 6 Tasks（ただしシーン設計が複雑） |

> **注意**: Task 数よりも「YAML シーン設計」と「WorldState キー設計」の習熟度が工数に影響します。  
> 初めての実装は `verification.md` の確認手順を最初に読み、期待値を把握してから着手することを推奨します。

---

## 7. テンプレート間の移行・組み合わせ

### 「Branching → Turn/Grid」（途中で方針転換）

証拠システムはそのまま残し、Turn/Grid の FakeCommand ループを追加するアプローチが有効です。  
ただし WorldState のスコープ設計が混在するため、キー命名に注意してください。

### 「Turn/Grid → Time/Clock」（タイミング要素を追加）

Turn の区切りとして KernelClock を使う（1 ターン = n ms 進める）拡張が可能です。  
Time/Clock テンプレートの `advance_rhythm_clock` パターンを Turn/Grid の `resolve_turn` 後に差し込む形です。

### 「複数テンプレートの混在」

1 つのゲームが複数のパターンを必要とする場合は、シーン単位で分離してください。  
例: ADV 本編 → Branching パターン、ミニゲーム → Time/Clock パターン（別シーングループ）

---

## 8. 次ステップ（T6 検討材料）

### billiards_test を formalize するか？

billiards_test が既存の 3 テンプレートのどれに該当するかを確認してください。

| 確認項目 | 調べ方 |
| :--- | :--- |
| KernelClock を使っているか | `src/games/billiards_test/tasks/` 内で `clock->advance` を grep |
| グリッド座標か物理座標か | WorldState キーに `x` / `y` / `angle` / `velocity` 等があるか |
| FakeCommand の形式 | `load_fake_*` Task が存在するか |
| ループ構造 | YAML シナリオの `if_continue:` / ループバックシーンがあるか |

**予測**: billiards_test が物理演算（速度・角度）を扱う場合は既存の 3 テンプレートには収まらず、  
**Event-Driven / Physics Basic** という新カテゴリのテンプレートが必要になる可能性があります。

### 草案レビュー後の推奨工程

1. このガイドを実際の新ゲーム開発で 1 回適用してフィードバックを得る
2. billiards_test 調査で T6 カテゴリを確定する（既存テンプレートの拡張 or 新規）
3. 必要であれば `examples/templates/README.md`（インデックスファイル）を追加する

---

*このドキュメントは Phase 3-A T5 の成果物です。3 テンプレートの横断レビューに基づく草案であり、実際のゲーム開発での適用後に更新してください。*
