# Mystery YAML v1 仕様書

## 1. 対象
- ランタイム: `KarakuriScenarioRunner`
- 実データ: `samples/mystery/scenario/*.yaml`
- 目的: プランナーが YAML のみでシーン進行を編集できるようにする

## 2. ルート構造（必須）
```yaml
start_scene: "scene_id"
scenes:
  scene_id:
    mode: "investigation"
    scene_path: "res://..."
    on_enter: []
    hotspots: {}
```

- `start_scene` (必須, string): 初期シーンID
- `scenes` (必須, map): シーン定義

## 3. scene 定義
### 必須
- `scene_path` (string): `PackedScene` のパス

### 任意
- `mode` (string): `investigation / deduction / confrontation / ending`
- `on_enter` (array[action]): シーン突入時に実行
- `hotspots` (map): クリック対象定義

## 4. hotspot 定義
```yaml
hotspots:
  hotspot_id:
    node_id: "Area2D名"
    on_click:
      - dialogue: { speaker: "System", text: "..." }
```

- `node_id` (必須, string): 読み込んだシーン内の `Area2D` ノード名
- `on_click` (任意, array[action], 省略時は何もしない)

## 5. action 仕様（v1）
action は「1キーの辞書」で表現する。

```yaml
- dialogue: { speaker: "Boss", text: "..." }
- goto: "next_scene"
```

### 5.1 `dialogue`
```yaml
- dialogue:
    speaker_key: "speaker.system" # 推奨（未指定時は speaker を利用）
    speaker: "System"             # 任意（既定: System）
    text_key: "demo..."           # Mystery運用では必須
    text: "fallback text"         # 非推奨（互換用）
```

実行時の待機:
- `DialogueUI` が `dialogue_finished` signal を提供する場合、ランタイムは **signal 発火まで次の action に進まない**（テキスト上書き防止）。
- signal がない場合は待機せず、即座に次の action に進む。

### 5.2 `set_flag`
```yaml
- set_flag: { key: "found_item", value: true }
```

### 5.3 `give_evidence` / `give_item`
```yaml
- give_evidence: "footprint"
- give_item: "footprint"
```

### 5.4 `wait`
```yaml
- wait: 0.5
```

### 5.5 `goto`
```yaml
- goto: "deduction"
```

### 5.6 `if_flag`
```yaml
- if_flag:
    key: "found_footprint"
    value: true
    then: []
    else: []
```

### 5.7 `if_has_items`
```yaml
- if_has_items:
    items: ["ectoplasm", "footprint"]
    then: []
    else: []
```

### 5.8 `choice`
```yaml
- choice:
    choices:
      - option:
          text_key: "demo..."   # Mystery運用では必須
          text: "拾う"          # 非推奨（互換用）
          actions: []
      - option:
          text_key: "demo..."
          actions: []
```

### 5.9 `take_damage`
```yaml
- take_damage: 1
# または
- take_damage: { amount: 1 }
```

### 5.10 `reset_game`
```yaml
- reset_game: true
```

### 5.11 `if_health_ge` / `if_health_leq`
```yaml
- if_health_ge:
    value: 2
    then: []
    else: []

- if_health_leq:
    value: 0
    then: []
    else: []
```

### 5.12 `testimony`
```yaml
- testimony:
    max_rounds: 3
    testimonies:
      - line:
          speaker_key: "speaker.witness"
          speaker: "Witness"
          text_key: "..."        # Mystery運用では必須
          text: "..."            # 非推奨（互換用）
          evidence: "footprint"
          shake_key: "..."
          shake: "..."           # 非推奨（互換用）
      -
        speaker: "Witness"
        text: "..."
        evidence: "torn_memo"
    on_success: []
    on_failure: []
```

### 5.13 `change_root_scene`
```yaml
- change_root_scene: "res://samples/main_menu.tscn"
```

## 6. エラーポリシー（v1）

### 6.1 起動停止（Fatal）
- YAML構文エラー
- ルートが map でない
- `start_scene` 欠落
- `scenes` 欠落/空
- `start_scene` が未定義sceneを指す
- scene の `scene_path` 欠落
- `scene_path` の `PackedScene` ロード失敗

### 6.2 アクション単位エラー（実行継続）
- `choice` 実行に必要な UI API 不足
- `testimony` 実行に必要な API/Signal 不足
- 未知の action 種別

動作: エラー/警告ログを出し、処理可能な範囲で進行を継続。

### 6.3 スキップ（No-op）
- hotspot の `node_id` 未指定
- `node_id` が `Area2D` として見つからない
- 条件分岐の不正/不足値

動作: 該当 hotspot/action を実質無効として扱う。

## 7. 運用ルール（プランナー向け）
- 新規シナリオは `samples/mystery/scenario/templates/mystery_template_v1.yaml` をコピーして作成。
- `text_key / speaker_key / shake_key` を必須運用とし、`text` は互換フォールバックとしてのみ使用する。
- `scene_id`, `flag`, `item` は `snake_case` を推奨。

## 8. 最小検証チェック
- `start_scene` が存在する
- すべての scene に `scene_path` がある
- すべての hotspot `node_id` がベースシーン上に存在する
- `goto` 先 scene が定義されている
- `testimony` の `evidence` が inventory ID と一致する
- クリック座標契約が一致している（`clicked_at` は Canvas/World 座標として扱う）
