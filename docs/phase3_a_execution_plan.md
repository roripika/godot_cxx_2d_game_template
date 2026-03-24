# Phase 3-A: Template Formalization 実行計画書

## 1. 目的
既存の 3 本のフィットネステスト（Mystery, Roguelike, Rhythm）から、AI が再利用可能な最小限の「ボイラープレート（型）」を抽出し、`examples/templates/` に公式テンプレートとして整備します。

## 2. 作業スコープ

### T1: テンプレート・ディレクトリの構築
- `examples/templates/` フォルダを作成。
- 各ジャンルの README と、最小構成の YAML / Task 雛形を配置するための構造を定義。

### T2: Branching Template の抽出 (`mystery_test` 由来)
- **対象**: `mystery_test` のうち、会話とフラグ分岐のみを抽出した最小 YAML。
- **成果物**: `examples/templates/branching_basic/`

### T3: Turn / Grid Template の抽出 (`roguelike_test` 由来)
- **対象**: 1対1のターン戦闘と 1x1 の移動のみを含む最小 YAML / Task 群。
- **成果物**: `examples/templates/turn_grid_basic/`

### T4: Time / Clock Template の抽出 (`rhythm_test` 由来)
- **対象**: `KernelClock` 進行と、固定時刻判定 1 ノートのみの最小 YAML。
- **成果物**: `examples/templates/time_clock_basic/`

### T5: AI 向けテンプレート選択ガイドの作成
- 各テンプレートが「どの検証軸（分岐 / 状態 / 時間）」に対応するかを、AI がプロンプト内で判別しやすくするための解説。

## 3. 実施順序
1. **T1 (Infrastructure)**: ディレクトリ作成。
2. **T2 -> T3 -> T4 (Content)**: ジャンルごとの抽出作業。
3. **T5 (Assistance)**: 選択ガイドの完成。

## 4. 完了条件
- `examples/templates/` 配下に 3 つの動作可能な（validator をパスする）最小 YAML セットが存在すること。
- AI エージェントが、特定のジャンルの要望に対して「どのテンプレートを使うべきか」を `docs/phase3_template_candidates.md` と整合して判断できること。
