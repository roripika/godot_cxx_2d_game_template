# Project Fonts

このプロジェクトでは、以下のフォントを同梱し、ゲーム内のデフォルトフォントとして使用しています。

## 採用フォント

### 1. Kosugi Maru (小杉丸ゴシック)
- **用途**: 日本語テキスト全般
- **特徴**: 読みやすく美しい丸ゴシック体。ミステリーゲームの雰囲気に適しています。
- **ライセンス**: Apache License, Version 2.0
- **入手場所**: Google Fonts (Motoya)

### 2. Montserrat (Medium)
- **用途**: 英語テキスト、UI数値、各種システム表示
- **特徴**: モダンで視認性の高いサンセリフ体。ノベルゲームのUIにおいて世界的に広く利用されています。
- **ライセンス**: SIL Open Font License (OFL) 1.1
- **入手場所**: Google Fonts (Julieta Ulanovsky)

## 設置ディレクトリ
- `assets/fonts/KosugiMaru/KosugiMaru-Regular.ttf`
- `assets/fonts/KosugiMaru/LICENSE.txt`
- `assets/fonts/Montserrat/Montserrat-Medium.ttf`
- `assets/fonts/Montserrat/OFL.txt`

## 再取得コマンド (CLI)

万が一ファイルを紛失した場合は、以下のコマンドをプロジェクトルートで実行することで再取得可能です。

```bash
mkdir -p assets/fonts/KosugiMaru assets/fonts/Montserrat
curl -L https://github.com/googlefonts/kosugi-maru/raw/main/fonts/KosugiMaru-Regular.ttf -o assets/fonts/KosugiMaru/KosugiMaru-Regular.ttf
curl -L https://raw.githubusercontent.com/googlefonts/kosugi-maru/main/LICENSE.txt -o assets/fonts/KosugiMaru/LICENSE.txt
curl -L https://github.com/googlefonts/montserrat/raw/master/fonts/ttf/Montserrat-Medium.ttf -o assets/fonts/Montserrat/Montserrat-Medium.ttf
curl -L https://raw.githubusercontent.com/googlefonts/montserrat/master/OFL.txt -o assets/fonts/Montserrat/OFL.txt
```

## Godotでの使用方法
- `project.godot` の `gui/theme/custom` に `assets/ui/default_theme.tres` を設定しています。
- 各ノードの `Theme Override > Fonts` で個別に指定することも可能です。
