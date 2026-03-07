# 汎用シーケンス管理システム (TaskRunner) 設計書

## 1. 目的
演出、音、ロジック、UI操作を「数珠つなぎ（直列）」または「一斉実行（並列）」で実行するための、柔軟な非同期タスク管理システムを構築します。
本システムでは、商業レベルのUXを実現するため、**「演出のスキップ（即時完了）」**および**「状態の巻き戻し（ロールバック）」**を基盤レベルでサポートします。

## 2. アーキテクチャ構成 (2層分離)

### A. Karakuri層 (src/core/) - 汎用タスク基盤
- **TaskBase**: 全てのタスクの基底クラス。
  - `on_start()` / `on_update(delta)` / `is_finished()`.
  - **`complete_instantly()`**: 演出をスキップし、タスクがもたらすべき最終状態（フラグ更新等）を即座に適用して終了させる。
- **SequencePlayer**: タスクの実行エンジン。
  - `skip_all()`: 実行中の全タスクに対し `complete_instantly()` を呼び出し、シーケンスを最速で完了させる。
  - **`StateSnapshot`**: フラグやアイテム等の「ゲームの状態」をキャプチャし、後で復元するためのスナップショット機能。
- **WaitTask**: 指定秒数待機、または特定のシグナル入力を待機する汎用タスク。

### B. Mystery層 (src/mystery/) - ミステリー専用アクション
- **MysteryActions**:
  - `ZoomCameraTask` / `ShowEvidenceUITask` / `PlayMysterySoundTask`.
  - 各タスクは `complete_instantly()` をオーバーライドし、「カメラを目標位置へワープさせる」「アイテムを即座に所持状態にする」等のスキップ処理を実装する。

---

## 3. クラス設計 (C++)

### [Core] TaskBase, SequencePlayer & StateSnapshot
```cpp
namespace karakuri {

// タスクの基底
class TaskBase : public RefCounted {
    GDCLASS(TaskBase, RefCounted)
public:
    virtual void on_start() {}
    virtual void on_update(double delta) {}
    virtual bool is_finished() const = 0;
    
    // スキップ用：即座に最終状態を適用して完了フラグを立てる
    virtual void complete_instantly() { /* デフォルトは何もしない */ }
};

// ゲーム状態のスナップショット
struct StateSnapshot {
    Dictionary flags;     // FlagService のコピー
    Array inventory;      // ItemService の所持リストのコピー
    String current_scene; // 現在のシーンID
};

// シーケンス実行エンジン
class SequencePlayer : public Node {
    GDCLASS(SequencePlayer, Node)
private:
    Array task_queue;
    std::vector<StateSnapshot> rollback_stack; // 巻き戻し用のスタック

public:
    void add_task(const Ref<TaskBase> &p_task);
    void process_tasks(double delta);
    
    // スキップ：全タスクを即時完了させる
    void skip_all();
    
    // ロールバック：シーケンス開始前の状態を保存・復元
    void create_snapshot();
    void rollback_to_last_snapshot();
};

}
```

---

## 4. スキップとロールバックのフロー

### 4.1. インスタンススキップ (Skip)
プレイヤーがボタンを連打、またはスキップボタンを押した際の挙動：
1. `SequencePlayer::skip_all()` を呼び出す。
2. 実行中の全てのタスクの `complete_instantly()` が呼ばれる。
3. 視覚的な演出（ズーム等）はスキップされるが、データ的な更新（フラグセット、アイテム取得）は確実に実行される。

### 4.2. ステートロールバック (Rollback)
「1つ前のセリフや選択肢に戻る」際の挙動：
1. 各重要シーケンス（会話の塊など）の開始時に `create_snapshot()` を実行。
2. `FlagService` や `ItemService` の現在のデータをコピーしてスタックに積む。
3. ロールバック要求時、スタックから最新の `StateSnapshot` を取り出し、各 Service にデータを書き戻す。

---

## 5. JSON シナリオ仕様案

```json
{
  "scene_id": "discovery_knife",
  "can_rollback": true, // このシーケンスの開始時にスナップショットを作成するか
  "sequence": [
    { "type": "play_effect", "id": "highlight" },
    { "type": "zoom_camera", "target": "knife" },
    { "type": "set_flag", "name": "found_knife", "value": true },
    { "type": "give_item", "id": "blood_knife" }
  ]
}
```

---

## 6. Copilot（実装担当）への実装ガイド

1.  **副作用の分離**:
    - タスクの `on_update` で少しずつデータを変えるのではなく、`on_start` または `complete_instantly` でデータ更新を一気に行う設計を徹底せよ。これがないとスキップ時にデータの整合性が崩れる。
2.  **Snapshot のディープコピー**:
    - `FlagService` 等の `Dictionary` を保存する際は、参照渡しではなく必ず `duplicate(true)` でディープコピーを作成せよ。
3.  **Undo 可能な設計**:
    - `ActionRunner` (演出) や `SoundService` (音) は、ロールバック時に「停止」または「巻き戻し」が必要になる場合がある。`SequencePlayer` の `rollback` 命令と各サービスをどう連携させるか検討せよ。
4.  **ScenarioRunner との統合**:
    - シナリオの各ノード実行前に自動で `create_snapshot()` を呼ぶオプションを `ScenarioRunner` に追加せよ。
