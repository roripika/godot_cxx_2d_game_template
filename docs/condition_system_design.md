# 汎用条件評価エンジン (Condition Evaluator) 設計書

## 1. 目的
ゲーム内のフラグ状態に基づき、データ（JSON）主導で複雑な論理判定（条件分岐）を行うための汎用基盤を構築します。
「特定の証拠品を持っている」「以前にこの人物と話した」「特定のステータスが一定以上」といった条件を、C++コードを修正せずにシナリオデータのみで定義・評価可能にします。

## 2. アーキテクチャ構成 (2層分離)

### A. Karakuri層 (src/core/) - 汎用フラグ・判定基盤
- **FlagService**: 
  - ゲーム内の全てのフラグ（Boolean, Integer, String）を一括管理するシングルトン・サービス。
  - `SaveService` と連携し、セーブ・ロード時にフラグ状態を永続化。
- **ConditionEvaluator**:
  - JSON 形式の条件式を再帰的に解析し、`FlagService` の現在の値を参照して `true/false` を返す評価エンジン。
  - 演算子（`==`, `!=`, `>`, `<`, `>=`, `<=`）および論理演算（`AND`, `OR`, `NOT`）をサポート。

### B. Mystery層 (src/mystery/) - ミステリー専用トリガー
- **MysteryTrigger**:
  - `Karakuri::ConditionEvaluator` を内包し、特定の条件が満たされたときに Godot のシグナルを発火するコンポーネント。
  - 例：「証拠品 A を入手済み」かつ「容疑者 B の証言を聴取済み」の場合にのみ、新しい調査ポイントを出現させる。

---

## 3. JSON 条件式構文案

### 3.1. 基本判定 (Leaf)
```json
{
  "flag": "is_cabinet_opened",
  "op": "==",
  "value": true
}
```
数値比較の例：
```json
{
  "flag": "player_level",
  "op": ">=",
  "value": 5
}
```

### 3.2. 複合条件 (Nested)
- `all`: 全ての条件が `true` であること (AND)
- `any`: いずれかの条件が `true` であること (OR)
- `not`: 条件の反転 (NOT)

例：「アイテムAを持っていて」かつ「(フラグBがON または フラグCがOFF)」
```json
{
  "all": [
    { "flag": "has_item_A", "is": true },
    {
      "any": [
        { "flag": "flag_B", "is": true },
        { "flag": "flag_C", "is": false }
      ]
    }
  ]
}
```
※ `is` は `op: "==", value: ...` のショートカット表記。

---

## 4. クラス定義案 (C++)

### [Core] FlagService
```cpp
namespace karakuri {

class FlagService : public Node {
    GDCLASS(FlagService, Node)
private:
    static FlagService *singleton;
    Dictionary flags; // { "flag_name": Variant }

public:
    static FlagService *get_singleton();
    
    // フラグ操作
    void set_flag(const String &p_name, const Variant &p_value);
    Variant get_flag(const String &p_name, const Variant &p_default = Variant());
    bool has_flag(const String &p_name) const;

    // シリアライズ (SaveService 連携)
    Dictionary serialize() const;
    void deserialize(const Dictionary &p_data);
};

}
```

### [Core] ConditionEvaluator
```cpp
namespace karakuri {

class ConditionEvaluator {
public:
    /**
     * @brief JSONデータ（Dictionary）を評価して結果を返す
     * @param p_condition 条件式の入った Dictionary
     * @param p_flags 評価対象のフラグ群 (デフォルトは FlagService::get_singleton())
     */
    static bool evaluate(const Dictionary &p_condition, FlagService *p_flags = nullptr);

private:
    // 再帰的に評価する内部関数
    static bool _eval_recursive(const Variant &p_node, FlagService *flags);
};

}
```

### [Mystery] MysteryTrigger
```cpp
namespace mystery {

class MysteryTrigger : public Node {
    GDCLASS(MysteryTrigger, Node)
private:
    Dictionary condition_json; // インスペクタから設定
    bool auto_check;           // _process 等で自動チェックするか

public:
    // 条件を満たしているか手動で確認
    bool check();

    // シグナル
    // signal condition_met()
    // signal condition_unmet()
};

}
```

---

## 5. Copilot（実装担当）への実装ガイド

1. **FlagService の実装**:
   - `src/core/services/flag_service.h / .cpp` を作成。
   - `Variant` 型を使用することで、型に縛られないフラグ管理を実現せよ。
2. **ConditionEvaluator の実装**:
   - `src/core/logic/condition_evaluator.h / .cpp` を作成。
   - `all`, `any`, `not` の各キーに対する再帰処理を正確に実装せよ。
3. **登録**:
   - `register_types.cpp` に新しいクラスを登録する。
4. **ScenarioRunner 連携**:
   - `ScenarioRunner` の `if` アクション等でこのエンジンの `evaluate` を呼び出せるように拡張を検討せよ。
