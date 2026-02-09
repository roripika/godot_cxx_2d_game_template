# Godot 4.x GDExtension Isometric Sandbox

## 環境構築 (Environment Setup)


### 前提条件 (Prerequisites)
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

このスクリプトは以下を行います:
- `godot-cpp` リポジトリのクローン (4.3ブランチ)
- SConsの存在確認及び `env.sh` (環境変数設定ファイル) の生成

## ビルド方法 (Build Helper)

セットアップ完了後、以下のコマンドでビルドします。

```bash
# 環境変数を読み込んでビルド (Mac M1/M2/M3)
source ./env.sh && scons platform=macos target=template_debug arch=arm64
```

## Godotエディタ設定 (Editor Setup)

### SandboxWorld ノードの設定
1. GDExtensionをビルド後、Godotエディタを開きます (または再起動)。
2. `TileMapLayer` ノードを作成する代わりに、カスタムクラス `SandboxWorld` をシーンに追加します (Type searchで検索可能になります)。
3. インスペクターで **Tile Set** リソースを新規作成します。
4. **Tile Set** の設定で、**Tile Shape** を `Isometric Diamond` (アイソメトリック・ダイヤモンド) に設定してください。
    - **注意点**: Isometric Diamondを選択した場合、**Tile Layout** は通常 `Stacked` にします。
    - `Tile Size` は使用するタイルのピクセルサイズに合わせて調整してください (例: 64x32)。
5. `y_sort_enabled` を `true` に設定すると、キャラクターやオブジェクトの描画順序が正しく処理されます。

## ディレクトリ構成
- `src/`: C++ ソースコード
- `godot-cpp/`: Godot C++ Bindings (setup.shで取得)
- `bin/`: ビルド成果物 (.framework)
