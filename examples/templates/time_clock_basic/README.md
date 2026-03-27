# Time/Clock Basic Template

**カテゴリ**: Time-driven / KernelClock / Timing Window  
**参照実装**: `src/games/rhythm_test/`  
**完了メモ**: `docs/rhythm_test_completion.md`

---

## このテンプレートが解決すること

**KernelClock** を使って仮想時刻を進め、タップ／入力と音符などのイベントタイミングを  
ミリ秒単位で比較し、perfect/good/miss を判定して clear・fail・continue に分岐するゲームロジックを  
**`src/core` を一切変更せずに** Kernel 上に载せるための最小構成を示します。

---

## いつ使うか

| 条件 | 判断 |
| :--- | :--- |
| 「時間（ms）」が主軸のゲームである | ✅ 使う |
| 入力と定刻イベントの一致度を評価する（リズム・音ゲー・QTE） | ✅ 使う |
| タイミング許容範囲（ウィンドウ）があり perfect/good/miss を出す | ✅ 使う |
| ノーツ数・ヒット数・ミス数のカウントで勝敗を決める | ✅ 使う |
| 実時間のリアルタイム入力を使いたい（将来） | ✅ 使う（ただし現テンプレートは FakeCommand ベース） |
| グリッド座標・エンティティ移動が主軸 | ❌ 使わない（Turn/Grid テンプレートを使う） |
| 証拠収集・分岐エンディングが主軸 | ❌ 使わない（Branching テンプレートを使う） |

---

## このテンプレートがカバーしない領域

- AudioStreamPlayer との実時刻同期（BGM 再生との連携）
- 100 ノーツ超の大規模負荷
- リアルタイムキーボード/タップ入力の統合
- エフェクト・演出・BGM
- 長押し・スライドなどの複合ジェスチャー判定

---

## テンプレートの構成

```
examples/templates/time_clock_basic/
├── README.md          ← このファイル（用途・境界説明）
├── contract.md        ← WorldState キー仕様 + 6 Task 契約 + KernelClock 利用ルール
├── scenario/
│   └── time_clock_smoke.yaml  ← clear/fail/continue の最小 YAML
└── verification.md    ← 確認手順 + timing window 検証 + rhythm_test 実測値
```

---

## このパターンの核心設計：KernelClock

`KernelClock` は Kernel が提供する仮想タイムソースです。  
`now()` が `double`（秒）を返し、`advance(delta_seconds)` で決定論的に時刻を進めます。

```
KernelClock::advance(seconds)  ←  advance_rhythm_clock_task が呼ぶ
KernelClock::now()             ←  judge_rhythm_note_task が読む
```

**重要な特徴**:
- リアルタイムではない。YAML / Task が明示的に `advance()` するまで時刻は進まない
- したがって smoke シナリオは完全に決定論的（毎回同じ結果）
- `advance_ms` を YAML で指定することで「n ms 後の判定」を再現できる

---

## timing window の仕組み

```
note_time_ms  ←  ノーツが配置された仮想時刻（YAML で指定）
tap_time_ms   ←  FakeTap が注入した入力時刻（YAML で指定、-1 = 未入力）
now_ms        ←  KernelClock.now() * 1000

diff = |tap_time_ms - note_time_ms|

diff <= perfect_window_ms  → "perfect"
diff <= good_window_ms     → "good"
それ以外（または tap=-1 かつ now_ms > note_time_ms + good_window_ms）  → "miss"
```

デフォルト値（`setup_rhythm_round` で上書き可能）:
- `perfect_window_ms` = 50 ms
- `good_window_ms` = 150 ms

---

## シーン構成パターン（5 シーン + 2 エンディング）

Turn/Grid や Branching と異なり、rhythm ゲームは 1 ノーツ処理に  
**複数のシーンを順番に経由する**構造になります。

```
boot → [advance] → [judge] → [resolve] → [check_round] → advance（ループ）
                                                        → clear（終了）
                                                        → fail（終了）
```

各シーンの役割:
| シーン | 役割 | 中心 Task |
| :--- | :--- | :--- |
| `boot` | 初期化 + 初回評価 | `setup_rhythm_round` → `evaluate_rhythm_round` |
| `advance` | クロックを n ms 進める | `advance_rhythm_clock` → `evaluate_rhythm_round` |
| `judge` | 入力とノーツを突き合わせる | `load_fake_tap` → `judge_rhythm_note` → `evaluate_rhythm_round` |
| `resolve` | カウンタを更新しノーツ index を進める | `resolve_rhythm_progress` → `evaluate_rhythm_round` |
| `check_round` | ラウンド確認ループバック | `evaluate_rhythm_round` → advance or clear or fail |
| `clear` | クリアエンディング | `end_game result:solved` |
| `fail` | フェイルエンディング | `end_game result:failed` |

---

## 参照実装からの学び

`rhythm_test` は以下の設計判断を検証済みです。

1. **KernelClock を Task から直接使う**  
   `advance_rhythm_clock_task` が `KernelClock::advance()` を呼び、  
   `judge_rhythm_note_task` が `KernelClock::now()` を読む。  
   WorldState の `clock:now_ms` は「最後に確認した時刻のスナップショット」として  
   同期して書き込む（デバッグ可視化のため）。

2. **ノーツと FakeTap の時刻は WorldState に書かれる**  
   YAML の `notes: [1000, 2000, 3000]` は `setup_rhythm_round` が  
   `chart:note_0:time_ms`, `chart:note_1:time_ms`, ... として SESSION に書き込む。  
   `load_fake_tap` は `tap:last_time_ms` に単一の tap 時刻を書き込む（-1 = 未入力）。

3. **各ノーツは 1 サイクル（advance → judge → resolve → check）で処理される**  
   `chart:index` が 0 から `note_count - 1` まで進み、全ノーツ消化後に evaluate が最終判定する。

4. **evaluate_rhythm_round が唯一の分岐点**  
   clear/fail/continue の判定はここだけで行う。miss_count > max_miss_count なら  
   即時 fail（早期終了）。全ノーツ消化後に hit_count >= clear_hit_count なら clear。

5. **pos-0 sacrifice パターン（全 non-boot シーン）**  
   シーン遷移後 pos-0 がスキップされる既知バグを回避するため、  
   `advance / judge / resolve / check_round` のすべての pos 0 にダミーを置く。  
   詳細: `docs/infrastructure_backlog.md`
