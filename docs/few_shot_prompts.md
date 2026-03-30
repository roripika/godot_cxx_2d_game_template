# Few-Shot Prompts — Karakuri シナリオ生成ガイド (T10)

**目的**: AI が「どのテンプレートを選ぶか」を正しく判断し、最小の YAML を誤りなく生成できるよう、  
4 テンプレートそれぞれについて入力例・選択理由・最小 YAML 出力例・よくある間違いを few-shot 形式で提供する。

**参照ツール**:
- タスク一覧: [`task_catalog.md`](task_catalog.md)
- テンプレート選択: [`template_selection_guide.md`](template_selection_guide.md)
- 静的検証: `python3 tools/validate_scenario.py <file>`

---

## テンプレート早見クイックフロー

```
「時間（ms / 秒）が判定軸になるか？」
  YES → Time/Clock Basic

「外部システムのイベント受信待機が必要か？（物理・センサー等）」
  YES → Event-Driven Basic

「グリッド移動 / ターン戦闘 / HP 管理か？」
  YES → Turn/Grid Basic

「証拠・フラグの組み合わせで分岐するか？（ADV / 推理）」
  YES → Branching Basic
```

---

## Shot 1 — Branching Basic（分岐推理・ADV）

### 入力例

> 「美術館で盗難事件が起きた。証拠 A（指紋）と証拠 B（目撃証言）が両方揃えば容疑者を逮捕できる。  
> 片方でも欠けていたら釈放になる。」

### テンプレート選択理由

- **判定軸が「フラグの組み合わせ」** → Branching Basic
- 時刻は不要（KernelClock 不使用）
- ループなし（一度分岐したら終わり）
- 外部イベント待機なし

### 最小 YAML 出力例

```yaml
start_scene: investigation

scenes:

  investigation:
    on_enter:
      - action: discover_evidence
        payload:
          evidence_id: fingerprint
          location: display_case

      - action: discover_evidence
        payload:
          evidence_id: testimony
          location: security_desk

      - action: check_condition
        payload:
          all_of:
            - evidence: fingerprint
            - evidence: testimony
          if_true:  arrest_scene
          if_false: release_scene

  arrest_scene:
    on_enter:
      - action: end_game          # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  release_scene:
    on_enter:
      - action: end_game          # pos-0 sacrifice
        payload: {result: wrong}
      - action: end_game
        payload: {result: wrong}
```

### よくある間違い ❌

```yaml
# ❌ 1. check_evidence を condition 判定に使う（単一フラグ用）
- action: check_evidence          # ← 2 フラグ結合には使えない
  payload:
    id: fingerprint
    if_true: arrest_scene
    if_false: release_scene

# ❌ 2. all_of と any_of を同時に書く
- action: check_condition
  payload:
    all_of: [...]
    any_of: [...]                 # ← 両方同時は ERR_INVALID_DATA

# ❌ 3. pos-0 sacrifice を terminal シーンに置かない
arrest_scene:
  on_enter:
    - action: end_game            # ← start_scene 以外は pos-0 がスキップされる
      payload: {result: solved}   #    もう 1 つ複製が必要

# ❌ 4. discover_evidence で location を省略する
- action: discover_evidence
  payload:
    evidence_id: fingerprint      # ← location は必須フィールド（validate で検出）
```

### validate チェックポイント

```bash
python3 tools/validate_scenario.py my_museum.yaml
# 確認項目:
#   ・check_condition の if_true / if_false が存在するシーン名か
#   ・discover_evidence に location があるか
#   ・end_game の result が solved/wrong/failed/lost/timeout のどれか
```

---

## Shot 2 — Turn/Grid Basic（ターン制グリッドゲーム）

### 入力例

> 「5×5 のダンジョンでプレイヤーがゴールを目指す。  
> プレイヤーは 1 ターンに移動か攻撃を選択できる。  
> エネミーの HP が 0 になればクリア。プレイヤーの HP が 0 になれば失敗。  
> それ以外は次のターンへ続く。」

### テンプレート選択理由

- **判定軸が「HP / 座標 / ターン数」** → Turn/Grid Basic
- FakeCommand でプレイヤー入力をモック
- 3 分岐ループ（clear / fail / continue）
- 時刻・外部イベント不要

### 最小 YAML 出力例

```yaml
start_scene: boot

scenes:

  boot:
    on_enter:
      # boot は start_scene → pos-0 sacrifice 不要
      - action: setup_roguelike_round
        payload:
          player_hp: 3
          enemy_hp:  2
          player_x:  2
          player_y:  2
          enemy_1_x: 3
          enemy_1_y: 2

      - action: load_fake_player_command
        payload:
          command: attack          # enemy_1 が隣接しているので即攻撃

      - action: apply_player_attack
        payload:
          target: enemy_1

      - action: apply_enemy_turn
      - action: resolve_roguelike_turn

      - action: evaluate_roguelike_round
        payload:
          if_clear:    clear
          if_fail:     fail
          if_continue: boot        # HP 残存 → 次ターン

  clear:
    on_enter:
      - action: end_game           # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  fail:
    on_enter:
      - action: end_game           # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

### よくある間違い ❌

```yaml
# ❌ 1. load_fake_player_command を省略して apply_player_attack を呼ぶ
- action: apply_player_attack     # ← last_action:type が空のまま → 判定不定
  payload:
    target: enemy_1

# ❌ 2. apply_player_attack に target を書かない
- action: apply_player_attack
  payload: {}                     # ← target は必須（validate で検出）

# ❌ 3. evaluate_roguelike_round の if_continue を自シーン以外にして無限ループ外れ
boot:
  on_enter:
    - action: evaluate_roguelike_round
      payload:
        if_clear:    clear
        if_fail:     fail
        if_continue: next_room    # ← next_room が undefined → validate ERROR

# ❌ 4. move と attack を同一ターンに両方書く
- action: apply_player_move
- action: apply_player_attack    # ← 移動と攻撃は排他。last_action:type が上書きされる
```

### validate チェックポイント

```bash
python3 tools/validate_scenario.py my_dungeon.yaml
# 確認項目:
#   ・load_fake_player_command の command が有効値か（attack/move_up 等）
#   ・apply_player_attack に target があるか
#   ・evaluate_roguelike_round の 3 フィールド（if_clear/if_fail/if_continue）が揃っているか
#   ・if_continue の参照先 scene が存在するか
```

---

## Shot 3 — Time/Clock Basic（リズム・時刻判定）

### 入力例

> 「3 つのノーツが 1000ms・2000ms・3000ms に流れてくる。  
> プレイヤーが各タイミングで正確にタップすれば perfect 判定。  
> 3 つ全部 perfect でクリア、1 つでも miss が出たら即失敗。」

### テンプレート選択理由

- **判定軸が「タップ時刻 vs ノーツ時刻の差分（ms）」** → Time/Clock Basic
- KernelClock を `advance_ms` で進めてから判定する
- 入力モックは FakeTap（`taps` 配列）
- notes/taps 長が 3〜5 に制限されている

### 最小 YAML 出力例

```yaml
start_scene: boot

scenes:

  boot:
    on_enter:
      # boot は start_scene → pos-0 sacrifice 不要
      - action: setup_rhythm_round
        payload:
          notes:           [1000, 2000, 3000]   # ノーツ時刻（ms, 昇順）
          taps:            [1000, 2000, 3000]   # FakeTap（diff=0 → perfect）
          advance_ms:      1000                 # 1 ステップ = 1000ms
          clear_hit_count: 3                    # クリアに必要な hit 数
          max_miss_count:  0                    # miss 0 で即 fail

      - action: evaluate_rhythm_round
        payload:
          if_clear:    clear
          if_fail:     fail
          if_continue: advance

  advance:
    on_enter:
      - action: advance_rhythm_clock   # pos-0 sacrifice
      - action: advance_rhythm_clock   # 実処理
      - action: evaluate_rhythm_round
        payload:
          if_clear:    clear
          if_fail:     fail
          if_continue: judge

  judge:
    on_enter:
      - action: load_fake_tap          # pos-0 sacrifice
      - action: load_fake_tap          # 実処理
      - action: judge_rhythm_note
      - action: evaluate_rhythm_round
        payload:
          if_clear:    clear
          if_fail:     fail
          if_continue: resolve

  resolve:
    on_enter:
      - action: resolve_rhythm_progress   # pos-0 sacrifice
      - action: resolve_rhythm_progress   # 実処理
      - action: evaluate_rhythm_round
        payload:
          if_clear:    clear
          if_fail:     fail
          if_continue: check_round

  check_round:
    on_enter:
      - action: evaluate_rhythm_round     # pos-0 sacrifice
        payload:
          if_clear: clear
          if_fail:  fail
          if_continue: advance
      - action: evaluate_rhythm_round     # 実処理
        payload:
          if_clear: clear
          if_fail:  fail
          if_continue: advance

  clear:
    on_enter:
      - action: end_game   # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  fail:
    on_enter:
      - action: end_game   # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

### よくある間違い ❌

```yaml
# ❌ 1. notes が 2 要素または 6 要素（validate で検出）
- action: setup_rhythm_round
  payload:
    notes: [1000, 2000]           # ← 3〜5 要素が必須

# ❌ 2. taps 長が notes 長と一致しない（validate で検出）
- action: setup_rhythm_round
  payload:
    notes: [1000, 2000, 3000]
    taps:  [1000, 2000]           # ← 長さ不一致

# ❌ 3. notes が昇順でない（validate で検出）
    notes: [1000, 3000, 2000]     # ← 厳密昇順が必要

# ❌ 4. advance → judge → resolve → check_round のシーン順を崩す
boot:
  on_enter:
    - action: setup_rhythm_round
      payload: {...}
    - action: judge_rhythm_note   # ← clock を進める前に judge している
    - action: evaluate_rhythm_round

# ❌ 5. advance の pos-0 sacrifice を忘れる
advance:
  on_enter:
    - action: advance_rhythm_clock  # ← これが pos-0 → スキップされ clock が進まない
    - action: evaluate_rhythm_round

# ✅ 正しい書き方
advance:
  on_enter:
    - action: advance_rhythm_clock  # sacrifice（pos-0 = スキップされる）
    - action: advance_rhythm_clock  # 実処理（pos-1 以降は実行される）
    - action: evaluate_rhythm_round
```

### validate チェックポイント

```bash
python3 tools/validate_scenario.py my_rhythm.yaml
# 確認項目:
#   ・notes 長が 3〜5 か
#   ・taps 長が notes と同じか
#   ・notes が昇順か
#   ・advance_ms > 0 か
#   ・good_window_ms >= perfect_window_ms か（両方指定時）
#   ・evaluate_rhythm_round の 3 フィールドが揃っているか
```

---

## Shot 4 — Event-Driven Basic（外部イベント待機）

### 入力例

> 「ピンボールゲーム。ボールを打つと物理演算で転がり、的にぶつかれば得点。  
> 1 ショット打って、的に当たればクリア。手球がポケットに入ればファウルで失敗。  
> 3 ショット以内に的に当たらなければ時間切れ失敗。」

### テンプレート選択理由

- **物理演算の結果を「イベント受信」として待機する** → Event-Driven Basic
- `TaskResult::Waiting` が必要（他テンプレートにはない）
- Fake-event タイムアウトでテスト可能
- グリッドなし・時刻判定なし

### 最小 YAML 出力例

```yaml
start_scene: setup_round

scenes:

  setup_round:
    on_enter:
      # start_scene → pos-0 sacrifice 不要
      - action: setup_billiards_round
        payload:
          shot_limit:   3          # 3 ショットで失敗
          target_count: 1          # 的 1 つ落とせばクリア

      # smoke: timeout=0.1 で即タイムアウト → balls_stopped を自動注入
      # 本番: timeout=30.0 にして物理エンジンのシグナルを待つ
      - action: wait_for_billiards_event
        payload:
          events:  [balls_stopped]
          timeout: 0.1             # smoke 用

      # ショット記録
      - action: record_billiards_event
        payload:
          event: shot_committed

      # 的に当たった記録（clear 経路）
      - action: record_billiards_event
        payload:
          event: ball_pocketed

      # 分岐判定
      - action: evaluate_billiards_round
        payload:
          if_clear:    victory
          if_fail:     defeat
          if_continue: shoot_again

  shoot_again:
    on_enter:
      - action: wait_for_billiards_event  # pos-0 sacrifice
        payload:
          events:  [balls_stopped]
          timeout: 0.1
      - action: wait_for_billiards_event  # 実処理
        payload:
          events:  [balls_stopped]
          timeout: 0.1
      - action: record_billiards_event
        payload:
          event: shot_committed
      - action: evaluate_billiards_round
        payload:
          if_clear:    victory
          if_fail:     defeat
          if_continue: shoot_again

  victory:
    on_enter:
      - action: end_game   # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  defeat:
    on_enter:
      - action: end_game   # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

### よくある間違い ❌

```yaml
# ❌ 1. wait_for_billiards_event を省略して直接 record を呼ぶ
setup_round:
  on_enter:
    - action: record_billiards_event    # ← wait がない → イベントが確定する前に記録
      payload:
        event: shot_committed

# ❌ 2. timeout: 0 を指定する（validate で検出）
- action: wait_for_billiards_event
  payload:
    events:  [balls_stopped]
    timeout: 0                         # ← > 0 が必須

# ❌ 3. 未知の event 名を使う（validate で検出）
- action: wait_for_billiards_event
  payload:
    events: [ball_fell_in_ocean]       # ← VALID_EVENTS 外
    timeout: 0.1

# ❌ 4. shoot_again の pos-0 sacrifice を忘れる
shoot_again:
  on_enter:
    - action: wait_for_billiards_event  # ← pos-0 → スキップ → 待機されない
      payload:
        events:  [balls_stopped]
        timeout: 0.1

# ❌ 5. evaluate_billiards_round のフィールドを 1 つ省略する（validate で検出）
- action: evaluate_billiards_round
  payload:
    if_clear:    victory
    if_fail:     defeat
    # if_continue が欠落 → ERR_INVALID_DATA

# ❌ 6. ファウル判定を record の前に独自ロジックで書く
# → ファウル優先判定は evaluate_billiards_round が内部で行う
#   record_billiards_event の順序だけ守れば自動的にファウル優先になる
```

### validate チェックポイント

```bash
python3 tools/validate_scenario.py my_pinball.yaml
# 確認項目:
#   ・wait_for_billiards_event の events が VALID_EVENTS 内か
#   ・timeout > 0 か
#   ・record_billiards_event の event が VALID_EVENTS 内か（単一値）
#   ・evaluate_billiards_round に if_clear/if_fail/if_continue が揃っているか
#   ・shoot_again に pos-0 sacrifice が配置されているか（validate は検出不可 → 手動確認）
```

---

## 4 テンプレート横断比較（few-shot 視点）

| 比較軸 | Branching | Turn/Grid | Time/Clock | Event-Driven |
|:---|:---:|:---:|:---:|:---:|
| ループ構造 | なし | あり（boot → boot） | あり（7シーン） | あり（shoot_again） |
| pos-0 sacrifice が必要なシーン | terminal のみ | terminal のみ | **全 non-boot シーン** | **shoot_again / terminal** |
| 必須の実行順序 | 自由 | setup → cmd → move/atk → enemy → resolve → eval | setup → advance → judge → resolve → check_round | setup → wait → record → eval |
| smoke の入力モック | 不要（WS 直接書き込み） | FakeCommand | FakeTap | Fake-event タイムアウト |
| validate で検出できない典型ミス | pos-0 未配置 | move/atk 同時実行 | advance 順序崩れ | pos-0 未配置 / wait 省略 |

---

## few-shot 利用ガイド

1. **上記の Shot 1〜4 を参照テンプレートとして prompt に含める**  
   → AI は「入力例 → 選択理由 → YAML 出力例」の対応パターンを学習済みとして扱える。

2. **新ゲームが既存例と異なる場合は差分だけ置き換える**  
   - 証拠 ID / ショット数 / ノーツ配置 / グリッドサイズ などはそのまま変数扱い。
   - Task 名は `task_catalog.md` を参照して正しいものを使う。

3. **YAML 生成後は必ず validate を実行する**  
   ```bash
   python3 tools/validate_scenario.py generated_scenario.yaml
   ```
   → SUGGEST を参考に修正 → exit 0 まで繰り返す。

4. **よくある間違い ❌ セクションを先にチェックする**  
   → validate では検出できない「pos-0 sacrifice 未配置」「実行順序崩れ」は手動確認が必要。
