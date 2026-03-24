# Rhythm Fitness Test Design Package (Kernel v2.0)

## 1. Why `rhythm_test`?
`rhythm_test` は、既存モジュール（Mystery, Billiards, Roguelike）ではカバーしきれない **「時間的厳密性（Time-driven Strictness）」** を検証するために選定されました。Kernel v2.0 の `KernelClock` が、ミリ秒単位の判定ウィンドウにおいて決定論的に動作することを証明します。

## 2. Minimalization Policy
- **純粋ロジック検証**: 演出や音声同期は次フェーズとし、論理判定の正確さに集中する。
- **決定論的入力**: 物理入力ではなく、YAML で定義された固定時刻入力（Fake Tap）を使用する。
- **拡張ポイントの限定**: 既存の Core に手を加えず、Task のみで全判定を実装する。

## 3. 目的定義：何を検証するか
`rhythm_test` は、Kernel v2.0 の「時間的決定論（Time-driven Determinism）」を検証する最終フィットネステストです。

- **KernelClock Synchronization**: 毎フレームの `KernelClock::now()` がロジックの判定ウィンドウとミリ秒単位で正確に噛み合うか。
- **Timing Window Logic**: 物理的な入力を介さず、「ある時刻 T に入力があった」という論理事実（Fake Tap）と、譜面（Note Schedule）の差分を決定論的に解決できるか。
- **State Normalization**: 連続的な「時間」という属性を、WorldState 上の離散的な「判定結果（Perfect/Miss）」へ安全に正規化できるか。
- **他モジュールとの違い**:
    - `mystery`: グラフ遷移（静的）。
    - `billiards`: 外部信号（物理ブリッジ）。
    - `roguelike`: 状態遷移（ターン）。
    - **`rhythm`**: 絶対時間軸（タイムライン同期）。

## 2. 最小ゲーム仕様（フィットネス構成）
- **規模**: 1 レーン、3〜5 ノーツの固定譜面。
- **判定幅 (Timing Windows)**:
    - **Perfect**: ±0.05s (50ms)
    - **Good**: ±0.15s (150ms)
    - **Miss**: それ以外（または 0.20s 超過）
- **入力**: `fake_tap_at(time)` タスクによる時刻指定入力。
- **終了条件**:
    - **Clear**: 全ノーツ消化後、Miss 数が規定以下。
    - **Fail**: 途中で Miss 上限に達する、または終了時条件未達。

## 3. Task 設計
| Task | 責務 | `validate_and_setup` | `execute` |
| :--- | :--- | :--- | :--- |
| `rhythm_init` | 譜面データの WorldState 展開。 | `notes` 配列の形式チェック。 | `rhythm_test:notes:[i]:time` 等を初期化。 |
| `rhythm_inject_tap` | 特定時刻での入力予約。 | `tap_time` が正数か。 | `rhythm_test:inputs:[id]` にタップ予定を登録。 |
| `rhythm_process` | 現在時刻とノーツを照合し判定。 | なし。 | `KernelClock::now()` を参照し、未判定ノーツと入力を照合。 |
| `rhythm_evaluate` | 最終成績による分岐。 | `if_clear`, `if_fail` 等の存在確認。 | 成績を判定し `ScenarioRunner` を介してシーン遷移。 |

## 4. WorldState Contract (Namespace: `rhythm_test:`)
| Key (Scope: Session) | 型 | 用途 |
| :--- | :--- | :--- |
| `rhythm_test:clock:now` | float | `KernelClock` から同期された現在時刻。 |
| `rhythm_test:notes:[i]:time` | float | ノーツのターゲット時刻。 |
| `rhythm_test:notes:[i]:hit` | string | 判定結果 ("none", "perfect", "good", "miss") |
| `rhythm_test:stats:perfect` | int | Perfect 合計数。 |
| `rhythm_test:stats:miss` | int | Miss 合計数。 |

**Validator 方針**:
- `notes` は開始時に pre-scan され、時刻が昇順であることを確認。
- `rhythm_test:` 以外の名前空間への書き込みを禁止。

## 5. YAML シナリオ構造（例）
```yaml
scenes:
  play:
    on_enter:
      # 1. 譜面初期化 (3ノーツ)
      - action: rhythm_init
        payload:
          notes: [1.0, 2.0, 3.0]
      
      # 2. 決定論的入力の予約 (1つだけ Good 判定を混ぜる)
      - action: rhythm_inject_tap
        payload: { time: 1.00 } # Perfect
      - action: rhythm_inject_tap
        payload: { time: 2.10 } # Good (100ms delay)
      - action: rhythm_inject_tap
        payload: { time: 3.00 } # Perfect
      
      # 3. 実行（タスクが時計を進め、判定を行う）
      - action: rhythm_run_engine
        payload:
          duration: 4.0
          if_finished: evaluate_result

  evaluate_result:
    on_enter:
      - action: rhythm_evaluate
        payload:
          if_clear: clear_scene
          if_fail:  fail_scene
```

## 6. Kernel リスク分析
- **弱点**: Godot の `_process(delta)` の変動。`KernelClock` が `delta` をそのまま累積すると、フレーム落ち時に「判定ウィンドウを飛び越える」可能性がある。
- **対策**: `rhythm_process` タスク内で、前回フレームから現在時刻までの「時間範囲」をスキャンし、その間にあるノーツを確実に処理するサブフレーム解決を導入する。
- **破綻ポイント**: **「音声とロジックのズレ」**。今回は fitness test のため音声は扱わないが、将来的に音声を入れる場合は `KernelClock` を音声サーバーのクロックに同期させる必要がある。

## 7. 実装優先順位
1. **Clock & WorldState**: `KernelClock` を `rhythm_test:clock:now` に投影するだけのループ。
2. **Deterministic Judge**: `notes` 時刻と `tap` 時刻の差分から文字列表記を得る純粋関数。
3. **Sequential Engine**: 指定秒数分 `advance` させ、その間のノーツを処理する `rhythm_run_engine` タスク。
4. **Validation**: 不正な譜面時刻（負の値や逆転）の排除。

## 8. 今やらないこと
- **リアルタイム音声同期**: フィットネステストの範囲を超える。
- **Long Note (Hold)**: 状態管理が複雑化するため後回し。
- **複数難易度 / BPM 変化**: 最小構成に絞る。
- **GUI アニメーション**: Debug Overlay（ASCII/Label）で十分。
