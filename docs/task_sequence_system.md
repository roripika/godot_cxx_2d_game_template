# 汎用シーケンス管理システム (TaskRunner) 設計書

## 1. 目的
演出、音、ロジック、UI操作を「数珠つなぎ（直列）」または「一斉実行（並列）」で実行するための、柔軟な非同期タスク管理システムを構築します。
これにより、複雑なイベント演出をC++コードのハードコーディングではなく、データ定義によって量産可能にします。

## 2. アーキテクチャ構成 (2層分離)

### A. Karakuri層 (src/core/) - 汎用タスク基盤
特定のゲームジャンルに依存しない、タスクの実行・管理の仕組みを提供します。

- **TaskBase**: 全てのタスクの基底クラス。
  - `on_start()`: 開始処理。
  - `on_update(delta)`: 毎フレームの更新。
  - `is_finished()`: 完了判定。
- **SequencePlayer**: タスクの実行エンジン。
  - `SerialQueue`: タスクを一つずつ順番に実行。
  - `ParallelGroup`: 複数のタスクを同時に実行し、全て終わるのを待つ。
- **WaitTask**: 指定秒数待機、または特定のシグナル入力を待機する汎用タスク。

### B. Mystery層 (src/mystery/) - ミステリー専用アクション
Karakuriの `TaskBase` を継承し、ゲーム固有の具体的な振る舞いを実装します。

- **MysteryActions**:
  - `ZoomCameraTask`: キャラクターや証拠品にカメラを寄せる。
  - `ShowEvidenceUITask`: 証拠品ゲット画面を表示し、プレイヤーの入力を待つ。
  - `PlayMysterySoundTask`: ミステリー演出用のSEやBGMを再生。

---

## 3. クラス設計 (C++)

### [Core] TaskBase & SequencePlayer
```cpp
namespace karakuri {

// タスクの基底
class TaskBase : public RefCounted {
    GDCLASS(TaskBase, RefCounted)
public:
    virtual void on_start() {}
    virtual void on_update(double delta) {}
    virtual bool is_finished() const = 0;
};

// シーケンス実行エンジン
class SequencePlayer : public Node {
    GDCLASS(SequencePlayer, Node)
private:
    Array task_queue; // 現在実行待ち・実行中の TaskBase ポインタ
    bool is_parallel = false;

public:
    void add_task(const Ref<TaskBase> &p_task);
    void clear();
    bool is_running() const;

    // _process 等で毎フレーム呼ばれる
    void process_tasks(double delta);
};

// 待機タスクの実装例
class WaitTask : public TaskBase {
private:
    double remaining_time;
public:
    WaitTask(double p_time) : remaining_time(p_time) {}
    void on_update(double delta) override { remaining_time -= delta; }
    bool is_finished() const override { return remaining_time <= 0; }
};

}
```

### [Mystery] 具体的なアクションタスク
```cpp
namespace mystery {

// カメラズームタスク
class ZoomCameraTask : public karakuri::TaskBase {
private:
    Vector2 target_pos;
    float zoom_level;
    bool finished = false;
public:
    void on_start() override {
        // Tween 等を使ってカメラを動かし、完了時に finished = true にする
    }
    bool is_finished() const override { return finished; }
};

}
```

---

## 4. タスクのライフサイクル

1.  **生成**: JSON の `action` 定義から対応する `TaskBase` サブクラスをインスタンス化。
2.  **登録**: `SequencePlayer` のキューに追加。
3.  **開始 (`on_start`)**: キューの先頭に来た（または並列実行が始まった）瞬間に一度だけ呼ばれる。
4.  **更新 (`on_update`)**: 毎フレーム呼ばれ、内部の状態を更新する。
5.  **完了 (`is_finished`)**: 毎フレームチェックされ、`true` を返すと次のタスクへ移行（またはグループ完了）。
6.  **破棄**: 実行完了したタスクはキューから取り除かれる。

---

## 5. JSON シナリオ仕様案

```json
{
  "scene_id": "discovery_knife",
  "sequence": [
    { "type": "wait", "time": 0.5 },
    { "type": "play_se", "id": "highlight_jingle" },
    { 
      "parallel": [
        { "type": "zoom_camera", "target": "knife_sprite", "duration": 1.0 },
        { "type": "screen_flash", "color": "#FFFFFF", "duration": 0.2 }
      ]
    },
    { "type": "show_message", "text": "これは...血のついたナイフだ！" },
    { "type": "give_item", "id": "blood_knife" }
  ]
}
```

---

## 6. Copilot（実装担当）への実装ガイド

1.  **TaskBase / SequencePlayer の実装**:
    - `src/core/logic/task_base.h` および `src/core/logic/sequence_player.h` を作成せよ。
    - `SequencePlayer` は、直列実行（Queue）と並列実行（Group）を切り替えられるか、あるいは別々のクラスとして実装することを検討せよ。
2.  **イベントループとの同期**:
    - `SequencePlayer::_process` 内でタスクを回し、デルタ時間を `on_update` に渡すこと。
3.  **ファクトリパターンの採用**:
    - JSON の `type` 弦から適切なタスククラスを生成する `TaskFactory` を作成すると、拡張性が高くなる。
4.  **ScenarioRunner 連携**:
    - `ScenarioRunner` がアクション配列を実行する際、内部で `SequencePlayer` を使って演出を処理するようにリファクタリングせよ。
