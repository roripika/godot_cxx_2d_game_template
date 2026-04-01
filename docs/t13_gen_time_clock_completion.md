# T13 完了メモ — Time/Clock Basic Generator

**フェーズ**: Phase 3-C  
**完了日**: 2026-04-01  
**実装ファイル**: `tools/gen_scenario_time_clock.py`

---

## 1. R-5 / R-6 / R-7 確認結果

### R-5: terminal シーンの pos-0 skip 確認

**調査方法**: `scenario_runner.cpp` コード解析 + headless HG-4 smoke  
**smoke ファイル**:
- `src/games/rhythm_test/scenario/rhythm_r5_clear_smoke.yaml` — result: solved 確認
- `src/games/rhythm_test/scenario/rhythm_r5_fail_smoke.yaml` — result: failed 確認

**確定内容**:
- `evaluate_rhythm_round::execute()` → `load_scene_by_id()` が呼ばれる
- その `execute()` returns Success → `step_actions()` while ループが `pending_action_index_++`
- terminal シーンへの遷移後も pos-0 skip が発生する
- **terminal シーン（clear / fail）に pos-0 sacrifice（×2）が必要**

実行ログ:
```
[ScenarioRunner] load_scene_by_id: rhythm_clear
[ScenarioRunner] starting compiled actions for rhythm_clear. Action count: 2.0
[EndGameTask] Game ended with result: solved  ✅

[ScenarioRunner] load_scene_by_id: rhythm_fail
[EndGameTask] Game ended with result: failed  ✅
```

---

### R-6: 中間シーン sacrifice パターン確認

**smoke ファイル**: `rhythm_r5_clear_smoke.yaml`（3 ノーツ = 3 サイクル完走）  
**確定内容**:
- advance/judge/resolve/loop_gate の全中間シーンで pos-0 sacrifice 必要
- 3 ノーツサイクル（advance→judge→resolve→loop_gate×3 + advance→judge→resolve）が完走

実行ログ（抜粋）:
```
[ScenarioRunner] load_scene_by_id: advance
[ScenarioRunner] load_scene_by_id: judge
[ScenarioRunner] load_scene_by_id: resolve
[ScenarioRunner] load_scene_by_id: loop_gate
[ScenarioRunner] load_scene_by_id: advance  ← ループ継続
...（3 サイクル後）
[ScenarioRunner] load_scene_by_id: rhythm_clear  ✅
```

---

### R-7: headless frame delta による clock drift 確認

**smoke ファイル**: `rhythm_r5_clear_smoke.yaml`  
**仕様**: `notes=[1000,2000,3000]`, `taps=[1000,2000,3000]`, `advance_ms=1000`, `max_miss_count=0`  
**確定内容**:
- headless での `_process()` frame delta は `advance_ms` 比で無視できる範囲
- `scheduled_tap <= now_ms` 判定は干渉なし
- 全 perfect 判定 → result: solved を確認

---

## 2. Generator 出力仕様（gen_scenario_time_clock.py）

### 入力（Structured Spec）

```yaml
template: time_clock_basic
scenario_name: rhythm_minimal

notes: [1000, 2000, 3000]
taps:  [1000, 2000, 3000]

scenes:
  terminal_clear: rhythm_clear
  terminal_fail:  rhythm_fail
```

### 出力シーン構成（固定）

| シーン | pos-0 sacrifice | Task 列 |
|:---|:---:|:---|
| `boot` | 不要 | `setup_rhythm_round` + `evaluate_rhythm_round` |
| `advance` | ✅ × 2 | `advance_rhythm_clock` ×2 + `evaluate_rhythm_round` |
| `judge` | ✅ × 2 | `load_fake_tap` ×2 + `judge_rhythm_note` + `evaluate_rhythm_round` |
| `resolve` | ✅ × 2 | `resolve_rhythm_progress` ×2 + `evaluate_rhythm_round` |
| `loop_gate` | ✅ × 2 | `evaluate_rhythm_round` ×2（sacrifice + 実処理） |
| terminal_clear | ✅ × 2 | `end_game` ×2 |
| terminal_fail | ✅ × 2 | `end_game` ×2 |

### バリデーション規則（V-TC-01〜V-TC-12）

| 規則 ID | 内容 |
|:---|:---|
| V-TC-01 | template == "time_clock_basic" |
| V-TC-02 | scenario_name が [a-z0-9_] 1〜40 文字 |
| V-TC-03 | notes は 3〜5 要素・各正整数・厳密昇順 |
| V-TC-04 | taps は notes と同数・各 >= -1 |
| V-TC-05 | advance_ms > 0（省略可） |
| V-TC-06 | clear_hit_count は 1〜len(notes)（省略可） |
| V-TC-07 | max_miss_count >= 0（省略可） |
| V-TC-08 | terminal_clear / terminal_fail: 互いに異なる・空文字禁止 |
| V-TC-09 | terminal_clear / terminal_fail: 予約語禁止（boot/advance/judge/resolve/loop_gate） |
| V-TC-10 | terminal_result: 省略可。指定時は有効 result 値のみ |
| V-TC-11 | perfect_window_ms > 0（省略可） |
| V-TC-12 | good_window_ms >= perfect_window_ms（省略可） |

---

## 3. smoke テスト結果

| テスト | 種別 | 結果 |
|:---|:---:|:---:|
| V-TC-01: wrong template | エラー系 | ✅ exit 1 |
| V-TC-03: 昇順違反 | エラー系 | ✅ exit 1 |
| V-TC-08: terminal 同名 | エラー系 | ✅ exit 1 |
| V-TC-09: 予約語 boot | エラー系 | ✅ exit 1 |
| 最小構成（notes 3 件, validate） | 正常系 | ✅ exit 0 |
| 推奨構成（全オプション, validate） | 正常系 | ✅ exit 0 |

---

## 4. 期待出力サンプル

`scenarios/generated/time_clock_basic_expected_output.yaml`（validate exit 0 確認済み）

---

## 5. 既知制限（v1.0 スコープ外）

- 複数コマンドシーケンス（ノーツごとに独立したタップタイミング）の自動生成は非対応
  → `setup_rhythm_round` の `taps` 配列に手動で値を設定する
- `perfect_window_ms` / `good_window_ms` の実行時動的変更は非対応
- ノーツ数 > 5 は非対応（`SetupRhythmRoundTask` の `kMaxNotes=5` 制限による）
- 複数レーン・複数難易度の自動生成は非対応
