# ミステリー演出・論理・音響の統合システム設計 (Mystery Experience System)

## 1. 目的
証拠品入手から突きつけ（論破）までのユーザー体験を、「視覚効果」「ゲームロジック」「音響・ハプティクス」が完全に同期した状態で実現するための統合システムを設計します。

## 2. アーキテクチャ構成 (2層分離)

### A. Karakuri層 (src/core/) - 汎用演出・音響基盤
特定のゲームジャンルに依存しない、演出と音の再生の仕組みを提供します。

- **ActionRunner (演出ランナー)**:
  - 画面シェイク、フェード、ズーム、スローモーション等の演出命令をキューで管理し、順次（または並列で）実行します。
  - Godotの `Tween` や `AnimationPlayer` と連携するインターフェースを持ちます。
- **SoundService (音響・ハプティクスサービス)**:
  - BGM/SEの再生管理（フェードイン/アウト、チャンネル管理）。
  - モバイルデバイス等のハプティクス（振動）制御。

### B. Mystery層 (src/mystery/) - ミステリー専用ロジック
Karakuri層の機能を組み合わせて、ミステリーゲーム特有の体験を構築します。

- **EvidencePresenter (証拠品選択ロジック)**:
  - 証拠品選択UIの表示制御。
  - プレイヤーの選択が「正解（矛盾の指摘）」か「不正解」かを `Evidence` クラスのロジックを用いて判定します。
- **MysteryEffectMap (イベント演出定義)**:
  - 「証拠品入手」「逆転」「ダメージ」など、特定のイベントに紐づく「演出のプリセット（音＋振動＋エフェクト）」を定義します。

---

## 3. C++ クラス定義案

### [Core] ActionRunner / SoundService
```cpp
namespace karakuri {

// 演出の種類
enum class EffectType { Shake, Zoom, Fade, SlowMotion, Flash };

// 演出基盤
class ActionRunner : public Node {
    GDCLASS(ActionRunner, Node)
public:
    // 演出をキューに追加
    void play_effect(EffectType type, Dictionary params = {});
    void stop_all();
};

// 音響・振動基盤
class SoundService : public Node {
    GDCLASS(SoundService, Node)
public:
    static SoundService *get_singleton();
    
    void play_bgm(const String &p_stream_path, float fade_sec = 0.0);
    void play_se(const String &p_stream_path);
    void vibrate(int intensity, float duration_sec); // ハプティクス
};

}
```

### [Mystery] EvidencePresenter
```cpp
namespace mystery {

class EvidencePresenter : public Node {
    GDCLASS(EvidencePresenter, Node)
public:
    // ScenarioRunner から呼ばれる
    // 展示UIを開き、プレイヤーの選択を待つ
    void start_selection(const String &target_statement_id, const Array &candidate_ids);
    
    // 判定結果をシグナルで返す
    // signal selection_finished(is_correct, selected_id)
};

}
```

---

## 4. シーケンス設計

### a. 証拠品入手時 (Item Collected)
`MysteryObject` が `item_collected` を発火した際の自動シーケンス：
1. `ScenarioRunner` が入力をロック。
2. `ActionRunner` がターゲットへズーム。
3. `SoundService` が「証拠品ゲット」のジングルと軽い振動を再生。
4. UIにアイテム詳細が表示され、プレイヤーの確認（クリック）を待つ。

### b. 突きつけ時 (Present Evidence)
`ScenarioRunner` のカスタムアクション `present_evidence` の流れ：
1. 会話を一時停止し、証拠品選択画面（`EvidencePresenter`）を表示。
2. プレイヤーが証拠を選択。
3. `Evidence::can_contradict()` で判定。
4. **成功時**: `SoundService` が「異議あり！」SEと強振動を再生。`ActionRunner` が画面をフラッシュ。
5. `ScenarioRunner` が `on_correct` のステップを `inject_steps` で実行。

---

## 5. JSON シナリオ仕様案 (演出の記述)

演出を伴うシナリオの記述例：
```json
{
  "scenes": {
    "interrogation_01": {
      "on_enter": [
        { "dialogue": { "speaker": "犯人", "text": "アリバイならある！あの夜は家で寝ていた！" } },
        {
          "present_evidence": {
            "target_id": "statement_alibi",
            "on_correct": [
              { "play_effect": "contradict_impact" }, 
              { "dialogue": { "speaker": "主役", "text": "それは嘘だ！この領収書を見ろ！" } }
            ],
            "on_wrong": [
              { "take_damage": 1 },
              { "dialogue": { "speaker": "主役", "text": "（しまわった、勘違いだったか...）" } }
            ]
          }
        }
      ]
    }
  }
}
```

`play_effect` アクションは `MysteryEffectMap` を参照し、内部で `ActionRunner` と `SoundService` を同時に呼び出します。
