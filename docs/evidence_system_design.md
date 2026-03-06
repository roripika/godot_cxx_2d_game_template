# 証拠品システム設計（探索・対話共存型）

## 1. 目的
「探索で拾う（Investigation）」ことと「対話で突きつける（Deduction/Present）」ことの両方に対応した、柔軟で再利用性の高い証拠品システムを構築します。
既存の `GameItem` と `EvidenceManager` を整理・統合し、Karakuri（基盤）と Mystery（ゲーム固有）の2層分離を厳守した設計とします。

## 2. アーキテクチャ構成

### A. Karakuri層 (src/core/)
基盤としてのアイテム管理を担当します。
- **ItemData**: アイテムの基本属性（名前、説明、アイコン、ID）を保持するデータ構造体。
- **ItemService**: 
  - 全アイテムデータのマスターリスト（JSON等からロード）の管理。
  - プレイヤーの現在の所持状況の管理。
  - `SaveService` と連携し、所持状態を保存・復元。

### B. Mystery層 (src/mystery/)
ミステリーゲーム特有のロジックを担当します。
- **Evidence**: `ItemData` を拡張（継承または包含）した、証拠品固有のデータ。
  - `contradicts_with`: 矛盾を指摘できる証言や状況のID。
  - `is_key_item`: 重要アイテムフラグ（移動制限や演出用）。
- **MysteryManager**: `ItemService` を利用して、「証拠品」としてのフィルタリングや、突きつけ時の矛盾判定ロジックを提供。

---

## 3. クラス定義案 (C++)

### [Core] ItemData & ItemService
```cpp
namespace karakuri {

// 基本アイテムデータ
struct ItemData {
    String id;
    String name;
    String description;
    String icon_path;
    Dictionary metadata; // 拡張用の汎用データ
};

// アイテム管理サービス (Singleton)
class ItemService : public Node {
    GDCLASS(ItemService, Node)
private:
    static ItemService *singleton;
    Dictionary item_master;    // 全アイテムカタログ (Loaded from JSON)
    Array inventory;           // 現在所持しているアイテムIDのリスト

public:
    static ItemService *get_singleton();
    
    // アイテムのロードと操作
    void load_master_data(const String &p_path);
    void add_item(const String &p_id);
    void remove_item(const String &p_id);
    bool has_item(const String &p_id) const;
    
    Array get_inventory_data() const; // UI表示用に全所持アイテムのDataを返す
};

}
```

### [Mystery] Evidence
```cpp
namespace mystery {

// ItemDataをミステリー向けに拡張
class Evidence : public RefCounted {
    GDCLASS(Evidence, RefCounted)
private:
    String item_id;
    String contradicts_with; // 矛盾する「証言ID」または「シーンID」
    bool is_key_item;

public:
    // ItemServiceからデータを取得して初期化
    void setup(const String &p_id, const Dictionary &p_mystery_data);
    
    // 判定ロジック
    bool can_contradict(const String &p_statement_id) const;
};

}
```

---

## 4. ScenarioRunner との連携

`ScenarioRunner` に以下のカスタムアクションを追加します。

### a. `give_item`
探索中にアイテムを入手した際に使用。
```json
{ "give_item": "blood_stained_knife" }
```

### b. `present_evidence`
対話中に証拠品選択UIを開き、結果によって分岐。
```json
{
  "present_evidence": {
    "on_correct": [ { "dialogue": { "text": "これが証拠だ！" } }, { "goto": "truth_path" } ],
    "on_wrong": [ { "dialogue": { "text": "それは関係ないな..." } } ]
  }
}
```

---

## 5. UI（Godot側）へのデータ提供
`ItemService` は、UI（GDScript）に対して以下の形式の `Dictionary` を提供します。

```gdscript
# ItemService.get_inventory_data() の返却例
[
    {
        "id": "knife",
        "name": "血のついたナイフ",
        "description": "被害者の部屋で見つかった凶器と思われるナイフ。",
        "icon": "res://assets/items/knife.png"
    },
    ...
]
```
これにより、Godot 側で `for` ループを回すだけでインベントリ画面が構築可能です。

---

## 6. 保存(Save)の仕組み
`SaveService` がシリアライズする際、`ItemService` の `inventory`（IDの配列）を保存対象に含めます。
ロード時に ID リストを復元し、`item_master` と照合することで状態を完全に復元します。
