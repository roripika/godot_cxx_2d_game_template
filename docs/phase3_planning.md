# Phase 3 (Template & Assistance) 計画書

## 1. 概要
Phase 2 で確立された「決定論的 Kernel 基盤」の上に、AI エージェントがより効率的かつ正確にゲームを構築するための **「型（Template）」** と **「支援ツール（Assistance）」** を整備します。

## 2. 三つの柱

### 2.1 Genre Templates (ボイラープレートの整備)
AI がゼロから YAML を書くのではなく、検証済みの構造からスタートできるようにします。
- **Roguelike Template**: グリッド移動、ターン順、基本戦闘タスクのセット。
- **Rhythm Template**: 譜面データの構造、判定ウィンドウ、クロック進行タスクのセット。
- **整備内容**: `examples/templates/` 配下への最小構成 YAML と関連 Task クラスの整理。

### 2.2 AI Assist Tooling (検証・修正支援)
`validate_scenario.py` を単なる「成否判定」から「修正案提示」へと進化させます。
- **Schema-driven Guidance**: 何が足りないか、どのペイロードが不正かを NL (Natural Language) で解説する機能。
- **Task Catalog Auto-gen**: 現存する ActionRegistry の全タスクを、AI が読みやすい Markdown 形式で自動出力するスクリプト。

### 2.3 Generator Design (生成プロンプトの高度化)
自然言語の要求から、Kernel 契約を遵守した YAML を生成するためのコンテキスト設計。
- **Few-shot Prompts**: 成功したスモークテストのペアをコンテキストとして提供する仕組み。
- **Iterative Refinement Flow**: 生成 -> バリデーション -> エラーフィードバック -> 再生成の自動化サイクル設計。

## 3. 最初のステップ
1. **Task Catalog Generator の作成**: AI が現在の全機能を把握できるようにする。
2. **Template の物理的配置**: `examples/templates/` ディレクトリの構築。
3. **Phase 3 A 計画の合意**: どのテンプレートから着手するか（Roguelike or Rhythm or New?）の決定。

## 4. 完了条件
- AI がテンプレートを用いて、新しい「バリエーション」のゲームを 15 分以内に構築できること。
- バリデーションエラーが起きた際、人間を介さずに AI 自身でエラー箇所を修正できるコンテキストが整っていること。
