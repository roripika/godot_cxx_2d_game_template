# Phase 2 差分整理レポート — 2026-03-19

## 概要

Phase 2（2-A validator 強化 / 2-B regression 整備 / 2-C billiards_test 最小骨格）の
実装過程で混入していた **Core 変更（Category B）** を差し戻し、  
Phase 2 スコープ内の変更のみをコミット `a6c5364` として確定した。

---

## 変更ファイル一覧と分類

| 分類 | ファイル | 変更内容 |
|:---:|---|---|
| **A** | `src/games/mystery_test/tasks/parallel_task_group.h/.cpp` | Phase 1 から未コミットのまま残っていたタスク群をリポジトリに追加 |
| **A** | `src/games/mystery_test/tasks/save_load_test_task.h/.cpp` | 同上 |
| **A** | `src/games/mystery_test/tasks/wait_for_signal_task.h/.cpp` | 同上 |
| **A** | `src/games/mystery_test/utils/scenario_validator.h/.cpp` | ScenarioValidator C++ 実装（GDScript から呼べる YAML 検証クラス） |
| **A** | `src/games/mystery_test/scenario/mystery_corrupted.yaml` | バリデーター拒否フィクスチャ（5種類の意図的エラーを含む） |
| **A** | `src/games/mystery_test/mystery_test_game.cpp` | 9 タスク登録を完成；不要な `scenario_validator.h` include を除去 |
| **A** | `src/games/mystery_test/tasks/check_condition_task.cpp` | UB バグ修正：`return TaskResult::Success;` と namespace 閉じ忘れを追加 |
| **A** | `src/games/billiards_test/tasks/wait_for_billiards_event_task.cpp` | `static godot::String[]` → `const char*[]`（エンジン初期化前クラッシュ対策） |
| **A** | `src/games/billiards_test/tasks/record_billiards_event_task.cpp` | 同上 |
| **A** | `src/register_types.cpp` | `ScenarioValidator` の include + `ClassDB::register_class<>` を 1 行追加のみ；初期化順序は HEAD を維持 |
| **B → 差し戻し** | `src/core/action_registry.h` | `godot::HashMap` → `std::map*`、`initialize()` API 追加 — **Core 非変更制約違反のため差し戻し** |
| **B → 差し戻し** | `src/core/action_registry.cpp` | `init_builtin_actions` → `initialize()`、コンストラクタ書き換え — 同上 |
| **B → 差し戻し** | `src/register_types.cpp`（初期化順変更部） | Layer 0 / `ActionRegistry::initialize()` 呼び出し追加 — Category B に依存するため除外 |

---

## 今回残した差分の説明

### 1. mystery_test: 未追跡タスク群のコミット
Phase 1 完了時点から `.gitignore` 対象外なのに未 `git add` のまま残っていた 6 ファイルを追加。  
これにより `git ls-files --others src/` に mystery_test 関連ファイルが表示されなくなった。

### 2. check_condition_task.cpp: UB 修正
コミット済みの HEAD では `execute()` の末尾に `return` 文がなく、かつ namespace 閉じ括弧も欠落していた。
```cpp
// 修正前（HEAD）: execute() に return がなく UB
if (runner_) {
    runner_->load_scene_by_id(target);
}
// ← ここで関数が終わっていた（UB）

// 修正後
    return TaskResult::Success;
}
} // namespace karakuri::games::mystery_test
```

### 3. billiards_test タスク: `const char*` クラッシュ対策
GDExtension 初期化前に `godot::String` の静的変数を構築すると `Abort trap: 6` が発生する。
```cpp
// 修正前（クラッシュ）
static const godot::String VALID_EVENTS[] = { "shot_committed", ... };

// 修正後（安全）
static const char* VALID_EVENTS[] = { "shot_committed", ... };
```
比較は `ev == v`（`godot::String == const char*`）でそのまま動作する。

### 4. register_types.cpp: ScenarioValidator のみ追加
`mystery_test_game.cpp` の `_ready()` 内での `ClassDB::register_class<ScenarioValidator>()` は不正（Node._ready はエンジン初期化後）なため、`register_types.cpp` の適切な場所に移動:
```cpp
// Mystery Test Game
ClassDB::register_class<karakuri::MysteryTestGame>();
...
ClassDB::register_class<karakuri::ScenarioValidator>();  // ← 追加
```

---

## 分離した差分の説明（Category B）

### ActionRegistry の内部構造変更について

**変更の動機**: GDExtension 起動時に `godot::HashMap` のコンストラクタが呼ばれ `Abort trap: 6` が発生するため、
`std::map<std::string, ...>*` へのポインタに変更し、エンジン初期化後に動的生成する設計へ。

**なぜ差し戻したか**:
- `src/core` の変更禁止制約に直接抵触する
- `ActionRegistry::initialize()` という新規公開 API の追加は contract 変更
- `register_types.cpp` の `s_action_registry->initialize()` 呼び出しと 1 対 1 で依存しており、
  片方だけ取り込むことが不可能

**今後の対応候補**（別 PR `fix/action-registry-gdextension-init-crash`）:
1. `ActionRegistry::ActionRegistry()` 内での `HashMap` 初期化を lazy に変える（Core 変更が必要）
2. GDExtension の初期化レベルを `MODULE_INITIALIZATION_LEVEL_CORE` にして HashMap が使えるタイミングを前倒し

---

## テスト確認

```
$ python3 tools/test_validator.py
============================================================
Validator Regression Tests
============================================================
  PASS  mystery_case.yaml is valid
  PASS  mystery_stress_test.yaml is valid
  PASS  mystery_timeout_test.yaml is valid
  PASS  diagnostic_test.yaml is valid
  PASS  mystery_corrupted.yaml is REJECTED
  PASS  billiards_fake_smoke.yaml is valid
  PASS  billiards_corrupted.yaml is REJECTED

ALL 7 TESTS PASSED
```

---

## Core 非変更の確認

```
$ git diff HEAD -- src/core/
（出力なし: 差分 0 行）
```

---

## コミット履歴

| ハッシュ | 内容 |
|---|---|
| `a6c5364` | fix(phase2): clean diff — Phase 2 scope only; revert Core changes |
| `121f686` | feat: add billiards_test skeleton (4 tasks, fake-event scenario) + validator guard rails |
| `a9da471` | fix: sync validator + yaml_spec with all 9 actions; add diagnostic_test.yaml |

---

## 未解決事項

| # | 内容 | 優先度 |
|---|---|:---:|
| 1 | ActionRegistry の GDExtension 起動クラッシュ（Category B）を Core 変更なしで解決する方法の検討 | 高 |
| 2 | billiards_test WorldState キー名を依頼仕様（`round:shots_taken` 等）に合わせるか確定 | 中 |
| 3 | `mystery_test_game.cpp` が `MysteryTestGame::_ready()` で `ScenarioValidator` を `ClassDB::register_class` しようとする残存コードの有無確認 | 低 |

---

## 次の最小ステップ

1. **別 PR**: `fix/action-registry-gdextension-init-crash` — Category B の ActionRegistry クラッシュ対策を Core 変更なしで解決
2. **billiards_test WorldState キー名確定**: 仕様書指定の個別フラグ方式に合わせるか、現状の集約 int 方式のままにするか決定
3. **Phase 3 の準備**: Template / Assist / Generator は Phase 3 以降。今回は触らない
