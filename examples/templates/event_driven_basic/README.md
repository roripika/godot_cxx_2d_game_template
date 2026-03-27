# Event-Driven Basic Template

**カテゴリ**: Event-Driven / TaskResult::Waiting / Fake-event Timeout  
**参照実装**: `src/games/billiards_test/`  
**完了メモ**: `docs/billiards_test_completion.md`

---

## このテンプレートが解決すること

**外部システム（物理エンジン・ネットワーク・センサー等）から非同期に届くイベント**を  
Kernel のゲームロジックで受け取り、イベントの種類と蓄積状態から  
clear / fail / continue を判定するゲームロジックを  
**`src/core` を一切変更せずに** Kernel 上に載せるための最小構成を示します。

---

## いつ使うか

| 条件 | 判断 |
| :--- | :--- |
| 外部システム（物理エンジン等）が「何かが起きた」イベントを送ってくる | ✅ 使う |
| イベントが来るまで Task がフレーム単位で待機する必要がある | ✅ 使う |
| 「ショット→飛翔→停止」のように非同期のフェーズがある | ✅ 使う |
| イベントの累積（ポケット数・ショット数等）で勝敗を判定する | ✅ 使う |
| ビリヤード・ピンボール・パチンコ・落下物・爆発判定など | ✅ 使う |
| smoke テスト時は実物理なしで決定論的に再現したい | ✅ 使う（Fake-event タイムアウトで対応） |
| ターン制・グリッド移動が主軸 | ❌ 使わない（Turn/Grid テンプレートを使う） |
| ms 単位の timing window が主軸 | ❌ 使わない（Time/Clock テンプレートを使う） |
| 証拠収集・分岐エンディングが主軸 | ❌ 使わない（Branching テンプレートを使う） |

---

## このテンプレートがカバーしない領域

- 実際の物理エンジン（Jolt / Bullet 等）との結線（シグナル受信・完全リターン）
- 複数イベントの並列待機・AND 条件
- イベントのキューイング（複数イベントを順番に処理）
- ネットワーク遅延・リトライ付き待機
- timeout 値のフレームレート非依存化
- 物理座標（x/y/velocity）を WorldState に記録する実装パターン

---

## テンプレートの構成

```
examples/templates/event_driven_basic/
├── README.md            ← このファイル（用途・境界説明）
├── contract.md          ← WorldState キー仕様 + 4 Task 契約 + Waiting ルール
├── scenario/
│   └── event_driven_smoke.yaml  ← clear/fail(foul)/fail(limit)/continue の最小 YAML
└── verification.md      ← 動作確認手順 + Waiting メカニズム検証 + 実測値
```

---

## このパターンの核心設計①：TaskResult::Waiting

他の 3 テンプレート（Turn/Grid / Branching / Time/Clock）では使われない  
**`TaskResult::Waiting`** がこのパターンの最大の特徴です。

```
execute() の戻り値:
  TaskResult::Success  → 次の Task へ進む
  TaskResult::Failed   → エラー終了
  TaskResult::Waiting  → 今フレームは何もせず、次フレームも execute() を呼ぶ
```

`WaitForBilliardsEventTask` の動作:

```
start_time_ = KernelClock::now()  ← validate_and_setup で記録

execute() が毎フレーム呼ばれる:
  now = KernelClock::now()
  if now - start_time_ >= timeout_:
      event:last_name = "balls_stopped"  ← Fake-event 注入
      return TaskResult::Success
  else:
      return TaskResult::Waiting         ← 待機
```

---

## このパターンの核心設計②：Fake-event タイムアウト

smoke テストでは物理演算を実行しないため、「イベントが来た」を模倣する必要があります。  
このテンプレートでは **タイムアウト経過を「balls_stopped が来た」とみなす** Fake-event パターンを採用します。

```yaml
- action: wait_for_billiards_event
  payload:
    events:  [balls_stopped]
    timeout: 0.1   # smoke 用: 100ms でタイムアウト → balls_stopped 注入
```

実際のゲームでは `timeout` を十分大きくし、  
物理エンジンのシグナルが Task の `complete_instantly()` を呼んで早期リターンさせます。

---

## このパターンの核心設計③：evaluate での clear/fail/continue 分岐

```
evaluate_billiards_round の判定優先順位:

  1. cue_ball_pocketed == true        → if_fail（ファウル：最優先）
  2. pocketed >= target_count         → if_clear
  3. shots_taken >= shot_limit        → if_fail（コスト上限）
  4. それ以外                          → if_continue（ループ継続）
```

`if_continue` は `shoot_again` シーンへ戻すことで、  
ショットサイクルを何度でも繰り返せます。

---

## シーン構成パターン（2 コアシーン + 2 エンディング）

```
setup_round → (1ショットサイクル: wait → record → evaluate)
  → if_clear    → victory（終了）
  → if_fail     → defeat（終了）
  → if_continue → shoot_again（ループ）
                    └─ wait → record → evaluate
                                → if_clear → victory
                                → if_fail  → defeat
                                → if_continue → shoot_again（再帰ループ）
```

各シーンの役割:
| シーン | 役割 | 中心 Task チェーン |
| :--- | :--- | :--- |
| `setup_round` | 初期化 + 最初のショットサイクル | setup → wait → record(s) → evaluate |
| `shoot_again` | 続行ショットサイクル（ループ） | wait → record(s) → evaluate |
| `victory` | クリアエンディング | `end_game result:solved` |
| `defeat` | 失敗エンディング | `end_game result:failed` |

---

## 参照実装からの学び

`billiards_test` は以下の設計判断を検証済みです。

1. **物理座標は WorldState に書かない**  
   `BilliardsManager` が Jolt Physics でボールの座標・速度を計算するが、  
   Task 側は「ポケットに入ったか」「ファウルか」「停止したか」という  
   **イベント名（文字列）だけ**を WorldState に書く。  
   これにより物理 ↔ ゲームロジックの境界が明確になる。

2. **`record_billiards_event` がイベント種別を if-else で振り分ける**  
   1 Task が全イベント種別を受け持ち、WorldState 更新を集約する。  
   Task を増やさず、payload の `event:` 文字列で動作を切り替える。

3. **`shoot_again` は自己参照ループを形成する**  
   `evaluate_billiards_round` の `if_continue: shoot_again` が  
   同一シーンへ遷移することでループを実現。  
   ループ終了条件（shots_taken >= shot_limit または pocketed >= target_count）を  
   evaluate が担うため、YAML 側はループ脱出条件を書く必要がない。

4. **evaluate の判定はファウル優先**  
   `cue_ball_pocketed` を最初に評価することで、  
   「的球をポケットしても手球も落とした場合はファウル（fail）」という  
   ゲームルールをコードで明示している。

5. **Fake-event タイムアウトは smoke 専用 hack である**  
   `timeout: 0.1` は実際のゲームでは使わない。本番では物理エンジンから  
   `complete_instantly()` を呼ぶことで Waiting を早期終了させる。  
   smoke と本番の切り替えは `timeout` 値と物理シグナル配線のみで行う。

---

## 新しいゲームに適用する手順（概要）

1. `src/games/<your_game>/tasks/` に 4 Task を実装する。  
   特に `wait_for_<your>_event_task` が `TaskResult::Waiting` を返すロジックを書く。
2. `scenario/` 以下に `event_driven_smoke.yaml` を参考とした YAML を配置する。  
   smoke では `timeout: 0.1`（短め）、本番では `timeout: 30.0`（十分長め）。
3. `contract.md` の WorldState キー設計を自分のゲーム用に埋める。
4. `verification.md` の確認手順で全経路（clear / fail-foul / fail-limit / continue）を検証する。

実装の出発点としては `src/games/billiards_test/` をリファレンスとして読む。  
コードはコピーせず、Task の**責務定義**と**Waiting パターン**を参照すること。
