# 言語切り替え機能 / Language Switching Feature

## 概要 / Overview

Mystery Demoに英語と日本語の切り替え機能を追加しました。
Language switching (English/Japanese) has been added to the Mystery Demo.

## 実装内容 / Implementation

### 1. 翻訳ファイル / Translation File
- **ファイル**: `samples/mystery/translations/mystery_translations.csv`
- すべてのUI テキスト、メッセージ、証拠品名が英語と日本語で定義されています
- All UI texts, messages, and evidence names are defined in both English and Japanese

### 2. 翻訳キーの使用 / Using Translation Keys
各スクリプトで `tr("key_name")` 関数を使用して翻訳テキストを取得します：
Each script uses the `tr("key_name")` function to get translated text:

```gdscript
# 例 / Example
dialogue_ui.show_message("System", tr("warehouse_intro"))
detail_name.text = tr("evidence_" + evidence.id)
```

### 3. 言語切り替えUI / Language Switcher UI
- **スクリプト**: `samples/mystery/scripts/language_switcher_scene.gd`
- 任意のシーンに追加可能な言語切り替えボタン
- Language switching buttons that can be added to any scene

使用方法 / Usage:
```gdscript
# シーンのルートノードにスクリプトをアタッチ
# Attach the script to the root node of a scene
extends Control

var language_switcher = preload("res://samples/mystery/scripts/language_switcher_scene.gd")

func _ready():
    var switcher = Control.new()
    switcher.set_script(language_switcher)
    add_child(switcher)
```

### 4. 更新されたファイル / Updated Files
以下のファイルが翻訳対応になりました：
The following files have been updated for translation:

- `warehouse_investigation_scene.gd` - 調査シーン / Investigation scene
- `office_deduction_scene.gd` - 推理シーン / Deduction scene
- `warehouse_confrontation_scene.gd` - 対決シーン / Confrontation scene
- `ending_scene.gd` - エンディング / Ending
- `testimony_system.gd` - 証言システム / Testimony system
- `inventory_ui.gd` - インベントリUI / Inventory UI
- `create_evidence_resources.gd` - 証拠品リソース生成 / Evidence resource creation

## テスト / Testing

### テストシーン / Test Scene
`samples/mystery/language_test.tscn` で言語切り替えをテストできます。
You can test language switching with `samples/mystery/language_test.tscn`.

### 言語の切り替え方法 / How to Switch Languages

**方法1: UIボタン / Method 1: UI Buttons**
画面右上の「English」または「日本語」ボタンをクリック
Click the "English" or "日本語" button in the top right corner

**方法2: コード / Method 2: Code**
```gdscript
TranslationServer.set_locale("en")  # 英語 / English
TranslationServer.set_locale("ja")  # 日本語 / Japanese
```

## 翻訳の追加 / Adding Translations

新しいテキストを追加する場合：
To add new text:

1. `mystery_translations.csv` に新しい行を追加
   Add a new line to `mystery_translations.csv`
   
2. フォーマット：`key_name,English text,日本語テキスト`
   Format: `key_name,English text,Japanese text`
   
3. スクリプトで `tr("key_name")` を使用
   Use `tr("key_name")` in your script

### 例 / Example
```csv
new_message,This is a new message,これは新しいメッセージです
```

```gdscript
dialogue_ui.show_message("System", tr("new_message"))
```

## 注意事項 / Notes

- CSVファイルの変更後は、Godotエディタで「Project > Reload Current Project」が必要な場合があります
- After changing the CSV file, you may need to "Project > Reload Current Project" in Godot Editor

- 証拠品の名前と説明は `evidence_{id}` と `evidence_{id}_desc` という形式のキーを使用します
- Evidence names and descriptions use keys in the format `evidence_{id}` and `evidence_{id}_desc`

- デフォルトの言語は日本語です（`ja`）
- The default language is Japanese (`ja`)
