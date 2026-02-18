# Mystery Demo 設計ドキュメント
## 目標
逆転裁判/都市伝説解体センタースタイルの完全なアドベンチャーゲームデモを作成する。

---

## デモストーリー構成

### プロローグ：オフィス（導入）
- **場所**: 探偵事務所
- **目的**: 事件の概要を聞く
- **内容**:
  - 所長から依頼を受ける（「古い倉庫で怪奇現象が発生」）
  - 調査に必要な道具を入手（初期証拠品）
  - 倉庫への移動

### 第1章：調査パート
- **場所**: 廃倉庫
- **目的**: 証拠を集める
- **内容**:
  - ホットスポットをクリックして調査
  - 証拠品を3つ収集（エクトプラズム、足跡、破れたメモ）
  - 目撃者と会話（選択肢あり）
  - 証拠が揃ったら次へ

### 第2章：推理パート
- **場所**: オフィス（復帰）
- **目的**: 証拠を整理して仮説を立てる
- **内容**:
  - 収集した証拠を確認
  - 所長との会話で選択肢
  - 正しい推理をすると次へ進む
  - 間違えるとペナルティ（HP減少）

### 第3章：対決パート（法廷スタイル）
- **場所**: 倉庫（再訪）
- **目的**: 犯人を追い詰める
- **内容**:
  - 犯人（？）の証言を聞く
  - 矛盾点を見つけて証拠品をつきつける
  - 「ゆさぶる」で追加情報を引き出す
  - 3回の証言ラウンド
  - 正解でクリア、3回ミスでゲームオーバー

### エピローグ：結末
- **場所**: オフィス
- **内容**:
  - フラグに応じて3種類のエンディング
    1. **完全勝利**: 全証拠収集 + ノーミス
    2. **通常クリア**: クリアしたが証拠漏れやミスあり
    3. **バッドエンド**: HP0でゲームオーバー

---

## システム設計

### 1. 状態管理（GlobalState）
```gdscript
# シングルトン: AdventureGameState
- flags: Dictionary<String, bool>  # フラグ管理
- variables: Dictionary<String, Variant>  # 変数（HPなど）
- evidence: Array<String>  # 所持証拠品ID
- current_scene: String  # 現在のシーン
- health: int = 3  # HP（ミス回数）
```

**機能**:
- `set_flag(name, value)`: フラグ設定
- `get_flag(name)`: フラグ取得
- `add_evidence(id)`: 証拠品追加
- `has_evidence(id)`: 証拠品所持確認
- `change_scene(path)`: シーン遷移
- `take_damage()`: HP減少
- `reset_game()`: ゲームリセット

### 2. 証拠品システム

#### EvidenceItem（Resource）
```gdscript
class_name EvidenceItem extends Resource

@export var id: String  # 一意ID
@export var display_name: String  # 表示名
@export var description: String  # 説明
@export var icon: Texture2D  # アイコン
@export var category: String  # カテゴリ（物証/証言/その他）
```

#### InventoryUI（Control）
- 証拠品リストを表示（グリッド or リスト形式）
- クリックで詳細表示
- 「つきつける」モード時は選択可能に

**配置**: CanvasLayerとして常駐、必要時に表示

### 3. 会話・選択システム

#### DialogueUI（拡張版）
**現在の機能**:
- `show_message(name, text)`: 基本表示

**追加機能**:
- `show_choices(choices: Array<String>)`: 選択肢表示
- `set_portrait(texture)`: 立ち絵表示
- `set_expression(expr)`: 表情変更
- `type_text(text, speed)`: タイプライター効果

**レイアウト**:
```
┌─────────────────────────────┐
│   [立ち絵]                  │
│                             │
│  ┌─────────────────────┐   │
│  │ [名前]              │   │
│  │ テキスト内容...     │   │
│  │                    │   │
│  └─────────────────────┘   │
│  [選択肢1]                  │
│  [選択肢2]                  │
└─────────────────────────────┘
```

#### ChoiceManager（Node）
- 選択肢を管理
- 選択結果をシグナルで返す
- フラグに応じて選択肢の表示/非表示を制御

### 4. 調査システム

#### InvestigationScene（Node2D）
- 背景画像
- ホットスポット定義（Area2D or Rect2の配列）
- クリック処理

#### HotspotManager（Node）
```gdscript
class Hotspot:
    var id: String
    var rect: Rect2  # クリック領域
    var description: String  # 初回調査時の説明
    var evidence_id: String  # 取得可能な証拠品ID（あれば）
    var requires_flag: String  # 必要なフラグ
    var examined: bool = false  # 調査済みか
```

**動作**:
1. クリック位置をチェック
2. ホットスポットと一致したら説明を表示
3. 証拠品があれば入手
4. フラグを設定

### 5. 対決システム（法廷モード）

#### TestimonySystem（Control）
**構成要素**:
- 証言データ（配列）
- 現在の証言インデックス
- コマンドボタン（「次へ」「ゆさぶる」「つきつける」）

#### Testimony（データ構造）
```gdscript
class Testimony:
    var speaker: String  # 話者
    var text: String  # 証言内容
    var contradiction_evidence: String  # この証言に矛盾する証拠品ID
    var shake_result: String  # ゆさぶった時の反応
    var correct_evidence: bool = false  # 正解の証拠品が提示されたか
```

**フロー**:
1. 証言を1つずつ表示
2. プレイヤーは「次へ」「ゆさぶる」「つきつける」を選択
3. **つきつける**:
   - 証拠品選択UI表示
   - 正解なら次フェーズへ進む
   - 不正解ならHP-1、証言を最初から
4. **ゆさぶる**:
   - 追加情報を表示（ヒント）
5. 全ラウンドクリアで勝利

#### ContradictionDetector（ロジック）
```gdscript
func check_evidence(testimony: Testimony, evidence_id: String) -> bool:
    return testimony.contradiction_evidence == evidence_id
```

### 6. UI/UXコンポーネント

#### HealthBar（HBoxContainer）
- ハート or ゲージ表示
- HP減少でアニメーション

#### TextAnimator（Label拡張）
- 1文字ずつ表示
- 速度調整可能
- クリックでスキップ

#### PortraitManager（TextureRect）
- 立ち絵の表示/非表示
- 表情差分の切り替え
- フェードイン/アウト

---

## データ構造

### 証拠品定義（JSON or Resource）
```json
{
  "evidence_list": [
    {
      "id": "ectoplasm",
      "display_name": "エクトプラズム",
      "description": "幽霊が出た証拠となる物質。倉庫の床に落ちていた。",
      "icon": "res://assets/evidence/ectoplasm.png",
      "category": "物証"
    },
    {
      "id": "footprint",
      "display_name": "血の足跡",
      "description": "倉庫の奥に続く足跡。人間のものではない。",
      "icon": "res://assets/evidence/footprint.png",
      "category": "物証"
    },
    {
      "id": "torn_memo",
      "display_name": "破れたメモ",
      "description": "「午前3時に...」と書かれている。",
      "icon": "res://assets/evidence/memo.png",
      "category": "物証"
    }
  ]
}
```

### 証言データ
```gdscript
var testimonies = [
    {
        "speaker": "倉庫管理人",
        "text": "あの夜は何も見ていません。",
        "contradiction": "footprint",
        "shake": "いや...何か音は聞こえたかも..."
    },
    {
        "speaker": "倉庫管理人",
        "text": "私は午後10時には帰宅しました。",
        "contradiction": "torn_memo",
        "shake": "本当です！信じてください！"
    },
    {
        "speaker": "倉庫管理人",
        "text": "幽霊なんているわけがない。",
        "contradiction": "ectoplasm",
        "shake": "...科学的にありえないでしょ？"
    }
]
```

---

## ディレクトリ構成

```
samples/mystery/
├── karakuri_mystery_shell.tscn    # エントリーポイント（固定）
├── office_base.tscn               # オフィス基底シーン
├── warehouse_base.tscn            # 倉庫基底シーン
├── ending_base.tscn               # エンディング基底シーン
├── scenario/
│   └── mystery.yaml               # シーン遷移/会話/分岐データ
├── scripts/
│   ├── adventure_game_state.gd    # 状態管理（既存）
│   ├── evidence_item.gd           # 証拠品リソース
│   ├── inventory_ui.gd            # 証拠品表示UI
│   ├── dialogue_ui_advanced.gd    # 拡張会話UI
│   ├── choice_manager.gd          # 選択肢管理
│   ├── hotspot_manager.gd         # ホットスポット管理
│   ├── testimony_system.gd        # 証言システム
│   ├── contradiction_detector.gd  # 矛盾検出
│   ├── health_bar.gd              # HP表示
│   └── portrait_manager.gd        # 立ち絵管理
├── data/
│   └── evidence/                  # 証拠品リソース（.tres）
└── ui/
    ├── evidence_inventory_ui.tscn # 証拠品UI（補助）
    └── testimony_ui.tscn          # 証言UI（補助）
```

---

## 実装順序

### フェーズ1: 基盤システム（1-2時間）
1. ✅ AdventureGameState の機能拡張（HP、証拠品管理）
2. EvidenceItem リソース作成
3. InventoryUI 実装（シンプル版）
4. DialogueUI 拡張（選択肢表示）

### フェーズ2: 調査システム（1時間）
5. HotspotManager 実装
6. 調査シーン作成（warehouse_investigation.tscn）
7. 証拠品3つのデータ作成

### フェーズ3: 対決システム（1-2時間）
8. TestimonySystem 実装
9. ContradictionDetector 実装
10. 対決シーン作成（warehouse_confrontation.tscn）
11. HealthBar UI

### フェーズ4: ストーリー統合（1時間）
12. 5つのシーンを繋げる
13. フラグ分岐の実装
14. エンディング3種類

### フェーズ5: 演出・UI（1時間）
15. 立ち絵表示
16. タイプライター効果
17. BGM/SE統合
18. トランジション演出

---

## 成功基準
- [x] プロローグから対決まで一通りプレイ可能
- [x] 証拠品の収集と提示が動作
- [x] 選択肢による分岐が動作
- [x] 3種類のエンディングが表示される
- [x] HP管理とゲームオーバーが機能
- [x] 証言・矛盾検出システムが動作

---

## 次のステップ
この設計を元に、フェーズ1から順に実装を開始します。
