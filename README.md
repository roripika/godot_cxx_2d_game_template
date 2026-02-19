# Godot 4.x GDExtension Isometric Sandbox

## 目的

このプロジェクトは、Godot + C++ (GDExtension) でゲーム開発するための再利用可能な環境を構築するものです。
このリポジトリを、AIとアイディアを考えながら多様なゲームを作るための基盤として利用します。

## 環境構築


### 前提条件
- **Godot 4.3+** (必須: TileMapLayerを使用するため)
- SCons
- Python 3

### 1. 必須ツールのインストール (macOS)

まず、システムレベルで必要なツールをインストールします。ターミナルで以下を実行してください。

```bash
# 1. Xcode Command Line Tools (Git, C++ Compiler)
xcode-select --install

# 2. Homebrew (パッケージマネージャー) - まだの場合
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 3. Python & SCons (ビルドシステム)
brew install python scons
```

### 2. プロジェクトの依存関係セットアップ

プロジェクトディレクトリで以下のスクリプトを実行し、`godot-cpp` (Godot C++ バインディング) を取得します。

```bash
chmod +x setup.sh
./setup.sh
```

## クイックスタート (CLI/エディタ両対応)

このテンプレートは「デモ切替 → GDExtensionビルド → 実行/エディタ起動」をワンコマンドで回せるように `dev.sh` を用意しています。

```bash
# 依存セットアップ (godot-cpp 取得など)
./dev.sh setup

# デザイナー/プランナー向け: メインメニューをエディタで開く
./dev.sh edit menu

# CLIで即実行 (例: mystery)
./dev.sh run mystery
```

`mystery` の正式エントリーポイントは `samples/mystery/karakuri_mystery_shell.tscn` です。

ミステリーデモの受け入れスモーク:
```bash
godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd
./scripts/check_mystery_translation_keys.sh
```

## 環境構築 (macOS)

### 1. 前提ツールのインストール

HomebrewでGodotとビルドシステム (SCons) をインストールします。

```bash
# Godot Engineをインストール
brew install --cask godot

# SConsをインストール (GDExtensionのビルドシステム)
brew install scons
```

### 2. C++拡張のビルド

このプロジェクトはGDExtension (C++) を利用します。エディタで開く前にソースをビルドしてください。

```bash
# デバッグ向けビルド (開発用)
./dev.sh build

# (手動) デバッグ向けビルド (開発用)
# scons platform=macos target=template_debug arch=arm64
```

### 3. プロジェクトの実行

#### CLI (推奨)
エディタを開かずに直接ゲームを起動できます。

```bash
# ヘルパースクリプトを使用
./scripts/launch_game.sh

# もしくはgodotコマンドで直接実行
godot --path .
```

#### Godotエディタ
1. Godotを開きます。
2. **Import** をクリックします。
3. このフォルダを選択し、`project.godot` を開きます。
4. **Import & Edit** をクリックします。
5. `F5` または再生ボタンで実行します。

## デモ切り替え

このテンプレートには複数ジャンルのデモが含まれています。セットアップスクリプトで切り替えます。

```bash
# 利用可能なデモ一覧
ls samples/

# アクティブなデモを切り替え
./scripts/setup_demo.sh menu        # メインメニュー (閲覧に推奨)
./scripts/setup_demo.sh gallery     # アセットギャラリー
./scripts/setup_demo.sh mystery     # アドベンチャー / ビジュアルノベル
./scripts/setup_demo.sh roguelike   # アイソメトリック・ダンジョンクローラー
./scripts/setup_demo.sh platformer  # 横スクロールアクション
./scripts/setup_demo.sh sandbox     # 採掘 & クラフト
./scripts/setup_demo.sh fighting    # 1v1格闘
./scripts/setup_demo.sh rhythm      # リズムゲーム
```

## ドキュメント
- `docs/demo_plan.md`: 各デモで「何を作るか」「共通Utilityをどう再利用するか」の計画書
- `docs/mystery_design.md`: Adventure(Mystery) デモの詳細設計

## Godotエディタ設定

### SandboxWorld ノードの設定
1. GDExtensionをビルド後、Godotエディタを開きます (または再起動)。
2. `TileMapLayer` ノードを作成する代わりに、カスタムクラス `SandboxWorld` をシーンに追加します (Type searchで検索可能になります)。
3. インスペクターで **Tile Set** リソースを新規作成します。
4. **Tile Set** の設定で、**Tile Shape** を `Isometric Diamond` (アイソメトリック・ダイヤモンド) に設定してください。
    - **注意点**: Isometric Diamondを選択した場合、**Tile Layout** は通常 `Stacked` にします。
    - `Tile Size` は使用するタイルのピクセルサイズに合わせて調整してください (例: 64x32)。
5. `y_sort_enabled` を `true` に設定すると、キャラクターやオブジェクトの描画順序が正しく処理されます。

- `bin/`: ビルド成果物 (.framework)
