# Roguelike Fitness Test Design Package (Kernel v2.0)

## 1. 目的定義：何を検証するか
`roguelike_test` は単なるゲーム実装ではなく、Kernel v2.0 の以下の能力を極限まで検証する「論理性フィットネステスト」です。

- **Turn Progression (順序の安全性)**: Player -> Enemy -> Global Resolve という循環的なターンの進捗を、Kernel が状態不整合を起こさずに管理できるか。
- **Entity State Logic (論理エンティティ)**: Node/Instance 単位ではなく、WorldState 内の「分散した Key 群」としてエンティティ（HP, 位置）を扱い、それを Task が安全に処理できるか。
- **Grid Occupancy (排他制御の論理性)**: 物理エンジンを介さず、WorldState 上の座標 Key を参照して「移動可能か」を Task レベルで判定する Fail-Fast 能力。
- **`mystery_test` / `billiards_test` との違い**:
    - `mystery`: dialogue 等のシーケンス（静的グラフ）。
    - `billiards`: 物理エンジン（外部シグナル）との同期。
    - **`roguelike`**: 完全に Kernel 内部で完結する「論理的な状態遷移の連鎖」。

## 2. 最小ゲーム仕様（フィットネス構成）
- **規模**: 1部屋（5x5〜8x8）、壁あり。
- **Entity**: プレイヤー(1)、敵(2)。
- **行動**: 移動（上下左右）、攻撃（移動先が敵なら攻撃。または隣接攻撃）。
- **フロー**:
    1. Player Turn 開始（Input 待機 Task）
    2. Player 行動決定 -> `move_entity` / `attack_entity`
    3. Enemy Turn 開始（AI 行動決定 Task）
    4. Enemy 行動決定 -> `move_entity` / `attack_entity`
    5. Turn Resolve（死亡判定、勝利判定）
- **終了条件**:
    - **Clear**: 敵の HP が 0。
    - **Fail**: プレイヤーの HP が 0。

## 3. Task 設計
| Task | 責務 | `validate_and_setup` | `execute` |
| :--- | :--- | :--- | :--- |
| `rogue_move` | 指定エンティティをグリッド移動。 | 全方向範囲内か、Payload に `entity_id` があるか。 | `WorldState` の `(x, y)` を更新。移動先が壁/他生物なら `Failed` 終了。 |
| `rogue_attack` | 攻撃判定とダメージ適用。 | `source_id`, `target_id` の存在確認。 | `target_id` の HP を減算。 |
| `rogue_wait_input` | プレイヤーの入力を待機（Fake command 含む）。 | なし。 | 入力が来るまで `Waiting`。入力確定で `Success`。 |
| `rogue_resolve` | ターン終了時の全判定。 | なし。 | HP <= 0 のエンティティ削除、勝敗フラグ更新。 |

## 4. WorldState Contract (Namespace: `rogue`)
論理状態を Key-Value で完全に表現します。

| Key (Scope: Session) | 型 | 用途 |
| :--- | :--- | :--- |
| `rogue:entities:[id]:pos` | Vector2i | グリッド座標。 |
| `rogue:entities:[id]:hp` | int | ヒットポイント。 |
| `rogue:map:grid:[x]:[y]` | string | "empty", "wall" などの通行フラグ。 |
| `rogue:turn:current_actor` | string | "player", "enemy_1", "enemy_2" |
| `rogue:status:result` | string | "ongoing", "victory", "game_over" |

**Validator 方針**:
- `rogue:entities:*` 以下の Key の存在を、シナリオ開始時に pre-scan する。
- Payload 禁止の原則に基づき、Task 経由でのみ更新を許可。

## 5. YAML シナリオ構造（例）
```yaml
# roguelike_basic.yaml
flow:
  - id: init_map
    action: rogue_init_fixed_map
  - id: game_loop
    loop:
      # Player Turn
      - action: rogue_wait_input
        id: player_move
      - action: rogue_move
        payload: { entity_id: "player", direction: "${last_input}" }
      
      # Enemy Turn
      - action: rogue_ai_decision
        payload: { entity_id: "enemy_1" }
      - action: rogue_move
        payload: { entity_id: "enemy_1", direction: "${ai_result}" }

      # Resolution
      - action: rogue_resolve
      - action: rogue_check_result
        on_success: "game_loop" # 継続
        on_fail: "end_game"    # 終了
```

## 6. Kernel リスク分析
- **弱点**: WorldState へのアクセス頻度。1ターンに大量の Key を読み書きするため、Kernel のオーバーヘッドが露出する可能性がある。
- **破綻ポイント**: **「移動先の予約」と「実際の移動」の不一致**。Task A が移動可能と判定し、Task B が割り込むような並列 TaskGroup を組むと死ぬ。
- **対策**: `roguelike_test` ではシーケンシャルな Task 連鎖によってのみ Turn を進めることで「決定論的直列化」を強制する。

## 7. 実装優先順位
1. **Grid 基盤**: `WorldState` に 5x5 のマップを書き込む `init` タスク。
2. **Entity 基盤**: `player` の位置移動タスク (`rogue_move`)。
3. **Turn Loop**: `rogue_wait_input` (Fake command) による 1-player 移動ループ。
4. **Enemy/Battle**: 敵の追加と HP 減算。
5. **Validator**: `rogue:` namespace 以下の不正な Key 書き込みを防ぐ。

## 8. 今やらないこと
- 手続き型マップ生成（今回の fitness test 目的から外れる）。
- アイテム/装備システム（エンティティ状態が複雑になりすぎる）。
- 視界（FOV）判定（論理性より計算コストの検証に寄ってしまうため）。
