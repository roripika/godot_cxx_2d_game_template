# Godot 4 + GDExtension(C++)で作る！2Dアドベンチャーゲーム開発記 (AI共作メトリクス)

Qiita記事向けの、技術的詳細を盛り込んだ開発レポートです。ミステリーデモの完成に至るまでの「AIとのペアプロの実態」を詳細に記載します。

## 1. プロジェクト概要
- **目的**: Godot 4のGDExtension (C++) をベースに、データ駆動型のゲームSDK（Karakuri SDK）を構築し、それを用いた「ミステリーADV」のデモを完成させる。
- **AIエージェント**: Antigravity (Google DeepMind), Codex (OpenAI), Copilot (GitHub)

## 2. 開発メトリクス (2月20日〜2月24日)
- **総ステップ数**: **4,682 ステップ**
- **AIによる自律ツール実行**:
    - コード編集: 約580回
    - ビルド実行 (SConstruct/scons): 40回
    - テスト・デバッグ（ゲーム起動・キャプチャ）: 48回
- **修正・追加されたコード行数 (LOC)**: 
    - C++: +3,300 / -850
    - GDScript: +2,100 / -600
    - Shaders: +800
    - Assets/UI Config: +1,800 (フォント・キャラ画像・YAML含む)

## 3. 技術的なハイライト

### A. C++ `ActionHandler` アーキテクチャ
`KarakuriScenarioRunner` は、コアロジックを肥大化させずに機能を拡張できるよう、`std::function` と `HashMap` を組み合わせたアクションディスパッチャを採用しました。
- **設計のこだわり**: `register_action("kind", callback)` メソッドにより、YAMLに新しいアクション（例: `testimony`, `take_damage`）をSDKのコアコードを弄らずに追加可能。
- **成果**: ミステリーデモ固有の「ゆさぶり」「つきつける」といったロジックを、完全にモジュール化された C++ コードとして分離。

### B. 高度なシェーダー遷移 (Cocos2d-x風)
Godot 4の `CanvasItemShader` を駆使し、レトロかつリッチな画面遷移を再現しました。
- **PageTurn**: テクスチャ座標を二次関数で歪ませ、裏面の影（Shadow）を動的に計算することで、2Dでありながら「紙がめくれる」立体感を表現。
- **RadialWipe**: 極座標 (`atan2`) を利用し、中心から扇状に画面が切り替わるエフェクトを実装。
- **SplitCols/Rows**: 画面をグリッド状に分割し、インターレース方式でフェードアウトさせる高速な遷移を実現。

### C. 苦労したデバッグ: 「ヘッドレスモードの謎のフリーズ」
自動テスト（UI Capture）実行中、Linux/Macのヘッドレス環境で `Tween` の終了を待つ `await` が無限ループに陥る問題に遭遇。
- **原因**: シェーダーのコンパイルや画面描画がスキップされるヘッドレスモードにおいて、特定のレンダリング駆動型 Tween が `finished` を発火しないことが判明。
- **AIによる解決**: C++ 側に `transition_timeout_sec` というウォッチドッグタイマーを実装。一定時間信号が来ない場合は強制的に次のシーンへ遷移させるガードロジックを導入。

### D. 立ち絵のレイアウト自動調整
1152x648 などの解像度において、キャラクターの立ち絵（バストアップ）が不自然に切れないよう、UIレイヤーのアンカーとオフセットを引き直すロジックを GDScript で実装。
- **動的スケーリング**: キャラクターの迫力を出すため、画面高の 80% (4/5) を基準に動的スケーリングを計算し、下端接地（ボトムアラインメント）を維持。

### E. タイポグラフィとフォント管理の自動化
ミステリーゲームの雰囲気を高めるため、カスタムフォントの自動導入パイプラインを構築。
- **CLI経由の資産取得**: `curl` を使用し、GitHub の Google Fonts リポジトリから直接フォント（Kosugi Maru / Montserrat）とライセンスを取得。
- **ライセンスの厳密な管理**: `assets/fonts/<FontName>/` のディレクトリ構造を採用し、フォントファイルとライセンス文書を一対一でパッケージング。
- **グローバルテーマ適用**: Godot の `default_theme.tres` を定義し、プロジェクト設定から一括適用。日本語は小杉丸ゴシック、英語・数値は Montserrat に自動的に切り替わる仕組み。

### F. シナリオロジックとNPC構成の拡張
テンプレートとしての汎用性を実証するため、ミステリーシナリオをプロ級の構成へと拡張。
- **7名のキャラクター実装**: 主人公、ボス、助手に加え、倉庫関係者等のNPCを追加。AI画像生成ツールにより、世界観の統一されたバストアップ画像を即座に用意。
- **複雑な論理構造**: [master_plan.md](file:///Users/ooharayukio/godot_cxx_2d_game_template/docs/mystery_scenario_master_plan.md) を起草し、Mermaidによるフロー管理、フラグ依存の証言変化、複数アイテムによる矛盾抽出ロジックを実装。
- **データ駆動開発**: 全ての会話と分岐を `mystery.yaml` 上で定義。バイナリ（C++）を触ることなく物語を完結させられるワークフローを確立。

### G. 「Node2Dの罠」: Control同士のアンカー崩壊
`warehouse_base.tscn` において、`Node2D` の直下に `Button` や `Label` を配置していたため、画面解像度を変更した際にアンカーが効かず、UIが画面外に消失する問題が発生しました。
- **原因**: `Node2D` は「サイズ」を持たない（0x0）ため、その子ノードである `Control` が「親の右端」にアンカーしようとすると、常に左端（座標0）基準になってしまう。
- **解決**: UI要素を `CanvasLayer` (Layer 5) にまとめ、シーン全体で「スクリーン空間への絶対座標アンカー」を強制することで解決しました。

### H. 翻訳データの「キー表示」ゴースト現象
翻訳ファイルを更新した直後、特定のボタンが翻訳値ではなく `mystery.ui.inventory_button` のようなキー文字列のまま表示される問題。

> [!NOTE]
> **用語解説: `tr()` 関数**
> Godot Engineには多言語対応のための `tr()` (translate) という組み込み関数があります。これは「翻訳キー（例: `mystery.title`）」を引数として渡すと、エンジンの仕組みによって現在の言語設定（日本語や英語など）に基づいた文字列を自動的に返してくれるものです。一般的なゲーム開発におけるI18n（国際化）の仕組みを簡略化したものと言えます。

- **原因**: `.csv` の更新だけでは Godot の `.translation` バイナリ（エンジンが実行時に参照する最適化データ）の再インポートが走らず、キャッシュされた古いデータ（またはデータなし）を参照していた。
- **解決**: `.translation` ファイルの強制削除と、起動引数によるエディタ経由の再インポートを促す運用で安定化。

### I. UI Audit（自動検診）スクリプトの開発
「UIがなぜか見えない」というユーザー報告に対し、ヘッドレス環境でもUIの「はみ出し」を検証できる `ui_audit.gd` を開発。
- **仕組み**: `get_global_rect()` で全コントロールの絶対座標を算出し、`ProjectSettings` のターゲット解像度 (1152x648) と比較。はみ出しているノードを `[OFF-SCREEN]` 警告として出力し、デバッグの属人性を排した。

## 4. 開発プロセス
1. **PLANNING**: AIが `implementation_plan.md` で設計案を提示。ユーザーが承認。
2. **EXECUTION**: AIが C++ をビルドし、GDScript を合わせ、シーンファイルをバイナリではなく `.tscn` テキストとして直接編集。
3. **VERIFICATION**: AI自身が `capture_scene.gd` をドライバとして起動し、撮影された成果物画像をユーザーに提示。

## 5. Codex担当の改修記録（実装補強・回帰防止）

### A. YAMLパーサの拡張（C++）
`KarakuriYamlLite` にインライン構文の解釈を追加。
- 対応した値形式: `{ key: value }` / `[a, b, c]`
- 目的: `dialogue` などの payload が文字列化される不具合を防止し、YAMLの記述意図をそのままC++へ渡す。
- 関連コミット: `1391fe9`

### B. 会話再生の信頼性向上（C++ + GDScript）
会話が表示されないケースに対し、フォールバックと待機制御を強化。
- `text_key` / `speaker_key` 未解決時でも可視テキストを維持するフォールバックを追加
- `DialogueUI` 側で表示更新の安全策を追加
- 関連コミット: `1391fe9`

### C. 立ち絵をYAMLで制御する機能追加
シナリオ側から立ち絵位置と演出を指定可能にした。
- `portrait_side`: `left | center | right | auto`
- `portrait_enter`: `none | fade_in`
- `portrait_exit`: `none | fade_out`
- 反映先: `KarakuriScenarioRunner`（C++）と `dialogue_ui_advanced.gd`
- 関連コミット: `1391fe9`

### D. Antigravity改修後の回帰レビューと修正
レビューで検出した回帰を順次修正。
- `evidence_ui_path` の誤配線を修正（インベントリUIがRunnerに再接続）
- Dialogueパネル本体を横移動させる実装を除去し、立ち絵のみ位置調整するよう修正
- `office_base_ui.gd` のノード参照を null 安全化
- デバッグ用の未追跡成果物（テストPNG/一時スクリプト）を整理
- 関連コミット: `b9bfcd4`

### E. 自動テストの強化（会話再生チェックを明示）
ヘッドレス＋トランジション環境でも安定して検証できるようスモークテストを改修。
- 会話は「表示テキスト」だけでなく「`_message_text_key`」でも検証
- トランジション中の待機条件を改善し、分岐（成功/失敗）双方を最後まで確認
- 実行結果: `res://samples/mystery/scripts/karakuri_scenario_smoke.gd` が `passed`
- 関連コミット: `1391fe9`, `b9bfcd4`

### F. Codex作業の定量メモ（該当コミットベース）
- コミット数: 2
- 変更ファイル数: 19（重複含む）
- テキスト差分: **+500 / -65**
- 対象レイヤー: C++, GDScript, `.tscn`, YAML, 設計/引き継ぎドキュメント

## 6. Copilot担当の改修記録（初期実装〜SDK基盤整備）

担当フェーズ: **2/14（初期構築）〜 2/20 前半（M8完了）**  
関連コミット: `4ed1d4c`, `81fc913`, `aebf96a`, `20dd4c7`, `542efda`, `0403f7f`, `2253f58`

### A. ミステリーデモ初期システムの構築（Phase 1）

`4ed1d4c` / `542efda` にて、ミステリーデモの GDScript 側コアを一気に立ち上げた。

- **HP管理**: `AdventureGameStateBase` に HP フィールドとダメージ処理を追加
- **証拠物品**: `EvidenceItem` リソースクラスと `InventoryUI` の実装
- **会話UI**: `DialogueUIAdvanced`（選択肢分岐対応）の実装
- **ホットスポット**: `HotspotManager` でクリック判定とシグナル発火
- **証言システム**: `TestimonySystem` の GDScript 実装（ゆさぶり選択 UI）
- **対決システム**: `warehouse_confrontation_scene.gd` と `warehouse_investigation_scene.gd` を新規作成し、倉庫シーンを構築
- **シーン生成スクリプト群**: `scripts/create_evidence_data.gd`, `create_evidence_ui_scene.gd`, `create_testimony_ui.gd` など、再現性の高いシーン自動生成ツールを整備

> **変更規模 (4ed1d4c + 542efda 合計)**: ファイル数 約 36、+2,170 / -260

---

### B. 英語/日本語ロカライゼーション基盤の導入

`81fc913` → `aebf96a` → `20dd4c7` の 3 コミットで、UI の多言語化を段階的に実装した。

#### B-1. 翻訳 CSV とランタイム切替  
- `samples/mystery/translations/mystery_translations.csv` に UI テキスト・メッセージ・証拠名の翻訳エントリを 70 行以上定義  
- 全シーンスクリプトに `tr()` 呼び出しを適用し、翻訳漏れを防ぐためのリスト（`LANGUAGE_README.md`）を同梱

#### B-2. `LanguageSwitcher` コンポーネント  
- `language_switcher_scene.gd` でランタイムに `TranslationServer.set_locale()` を発行する UI コンポーネントを実装  
- `office_scene.tscn` に配置し、ゲームを再起動せずに `English` ↔ `日本語` を切替可能に  
- ボタン位置を `position` でなく `offset` で指定する修正（`20dd4c7`）で、異解像度での配置ずれを解消

> **変更規模 (81fc913 + aebf96a + 20dd4c7 合計)**: ファイル数 約 42（UID/キャッシュ含む）、+1,344 / -92  
> **実コード対象レイヤー**: GDScript, 翻訳 CSV, `.tscn`, `project.godot`

---

### C. C++ 責務寄せと言語切替基盤の整備（M3-M5-M7）

`0403f7f` にてGDScript側に分散していたロジックを C++ 側へ集約し、SDK としての堅牢性を高めた。

#### C-1. `KarakuriLocalizationService`（C++ 新規実装）
- `src/karakuri/karakuri_localization_service.cpp` (+154) を新規作成
- Godot の `TranslationServer` を C++ から呼び出す薄いラッパーとして設計し、YAML アクション `set_locale` で呼び出せるよう `KarakuriScenarioRunner` に登録

#### C-2. `KarakuriScenarioRunner` の責務拡張
- 言語切替・インベントリ表示制御・証言進捗を Runner 側に引き込み、GDScript は「受け取って表示する」役割のみに限定
- `dialogue_ui_advanced.gd` (202 行以上更新)、`inventory_ui.gd` (181 行以上更新)、`testimony_system.gd` (344 行削減リファクタ) を一貫した責務構造に整理
- `mystery_shell_ui.gd`, `office_base_ui.gd`, `warehouse_base_ui.gd` を新設し、各シーンの「UI 初期化」コードを一箇所に集約

#### C-3. 翻訳キー整合性チェックスクリプト
- `scripts/check_mystery_translation_keys.sh` (+51) を追加。CSV に定義されたキーと GDScript で使われる `tr(key)` の差分を CI的に検出可能に。

> **変更規模 (0403f7f)**: 36 ファイル、+1,560 / -606  
> **対象レイヤー**: C++, GDScript, YAML, `.tscn`, 翻訳 CSV, 設計ドキュメント

---

### D. セーブ機能と TestimonySession 分離（M8）

`2253f58` にて、SDK として欠けていた「セーブ/ロード」と「状態の適切な分離」を実装した。

#### D-1. `KarakuriSaveService`（C++ 新規実装）
- `src/karakuri/karakuri_save_service.cpp` (+216) / `.h` (+84) を新規作成
- フラグ・インベントリ・HP を `user://karakuri/<demo_id>/save.json` に JSON 永続化
- YAML アクション `save` / `load` を `init_builtin_actions` に追加し、シナリオ記述から直接呼び出し可能に
- `AdventureGameStateBase` にスナップショット API (`get_flags_snapshot` / `restore_flags_snapshot` 等) を追加

#### D-2. `KarakuriTestimonySession` への状態分離
- `karakuri_testimony_session.h` を新規作成し、Runner 側にあったフラットなフィールド 9 個を `testimony_` 構造体 1 つに集約
- Runner.cpp 内の53箇所の参照を `testimony_.xxx` に置換し、Runner 本体のコード見通しを向上
- `complete_testimony()` 内のリセット順序を整理（`chosen` 取り出し後に `reset()` を呼ぶよう修正）

> **変更規模 (2253f58)**: C++ +395 / GDScript +3 / YAML +267 / ドキュメント +430

---

### E. 実運用で見えた課題と後フェーズへの影響

Copilot フェーズで生まれた試作は速度優先だったため、次のような課題が顕在化した：
- 言語切替の UI は実装されたが、`tr()` 呼び出しの漏れや反映ラグが一部のシーンで発生
- GDScript 直書きのロジックが残り、YAML 駆動との責務境界が曖昧なファイルが生じた
- UI の重なり順・アンカーポリシーが各シーンで不統一になった

これらが Antigravity フェーズ（設計の明文化・C++ 集約）と Codex フェーズ（回帰修正・自動テスト強化）の方針を決定づける直接の契機となった。

---

### F. Copilot 作業の定量サマリ（コミットベース）

| 指標                                     | 値                                                                                                                         |
| ---------------------------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| コミット数                               | **7**                                                                                                                      |
| 主要変更ファイル数（UID/キャッシュ除く） | 約 **70**                                                                                                                  |
| GDScript 差分                            | **+2,800 / -400**                                                                                                          |
| C++ 差分                                 | **+900 / -130**                                                                                                            |
| YAML / 翻訳 CSV 差分                     | **+700 / -300**                                                                                                            |
| 新規実装クラス (C++)                     | `KarakuriLocalizationService`, `KarakuriSaveService`, `KarakuriTestimonySession`                                           |
| 新規実装スクリプト (GDScript)            | `language_switcher_scene.gd`, `mystery_shell_ui.gd`, `office_base_ui.gd`, `warehouse_base_ui.gd`, `testimony_system.gd` 他 |

---
*このレポートは、SDK開発の全行程を記録した AI (Antigravity / Codex / Copilot) のログを基に構成されました。*

---

## 7. Copilot担当の改修記録（バグ修正・テスト戦略強化）

担当フェーズ: **2025年2月25日**  

### A. インベントリ開中のホットスポット誤発火バグ修正（C++）

**問題:**  
インベントリ（証拠品 UI）を開いた状態で背景や NPC をクリックすると、インベントリの裏側でホットスポットが反応してしまうバグが存在した。

**原因:**  
`KarakuriScenarioRunner::on_clicked_at()` に「インベントリ表示中は処理をスキップ」するガードがなかった。

**修正内容 (`src/karakuri/scenario/karakuri_scenario_runner.cpp`):**
```cpp
void KarakuriScenarioRunner::on_clicked_at(const Vector2 &pos) {
  if (is_executing_actions_ || !mode_input_enabled_ || waiting_for_transition_) {
    return;
  }
  // Block hotspot interactions while the inventory/evidence UI is open.
  if (evidence_ui_ != nullptr && bool(evidence_ui_->get("visible"))) {
    return;
  }
  // ... hotspot loop
}
```

**補足:**  
`give_evidence` アクションは証拠取得後に自動でインベントリを開く設計のため、この修正によって「証拠品を取った直後の余分なクリックが次のホットスポットを誤発火する」問題も同時に解消された。

---

### B. 3段階テスト戦略の設計と実装

AI 実装を品質保証するため、性質の異なる 3 種のテストを整備した。詳細は [`docs/testing_strategy.md`](docs/testing_strategy.md) を参照。

#### テスト構成

```
./dev.sh smoke    ~5-10秒   起動確認・必須UIノード・プロローグ表示・ロケール切替
./dev.sh e2e      ~60秒     全シナリオ通し（グッドエンド + バッドエンド）
./dev.sh monkey   ~30-60秒  ランダム操作でクラッシュ・デッドロックを検出
```

#### なぜ分けたのか

| 種類       | 問う問い           | 実行タイミング                    |
| ---------- | ------------------ | --------------------------------- |
| **smoke**  | 「動くか？」       | 全ての変更後に必須                |
| **e2e**    | 「正しく動くか？」 | C++ / YAML 変更後に必須           |
| **monkey** | 「壊れないか？」   | 入力処理・UI 状態管理変更後に必須 |

e2e だけでは「想定した順序で操作した場合」しか検証できない。AI が生成するコードは「正しい操作手順では動く」が「想定外の順序では壊れる」実装になりやすいため、monkey テストをセットで必須化した。

#### Tier 2: e2e に追加した検証項目

前フェーズからの既存 e2e テスト（`karakuri_scenario_smoke.gd`）に以下を追加:

| 追加テスト                  | 内容                                                       |
| --------------------------- | ---------------------------------------------------------- |
| インベントリ開閉            | `show_inventory()` → CloseButton 押下 → `visible == false` |
| インベントリ中 NPC ブロック | インベントリ表示中のクリックがランナーを起動しないこと     |
| モード切替ラベル変化        | ボタンテキストが Investigate ↔ Talk 切替で変わること       |

#### Tier 3: monkey テストの設計

`karakuri_monkey.gd` では以下のアクションをランダムな比重で実行する:

| アクション             | 比重 | 内容                                          |
| ---------------------- | ---- | --------------------------------------------- |
| 画面ランダムクリック   | 4/20 | 1280×720 内の任意座標                         |
| ホットスポットクリック | 3/20 | 倉庫の既知 HS 座標をランダム選択              |
| モード切替ボタン       | 2/20 | Investigate ↔ Talk                            |
| インベントリを開く     | 3/20 | `show_inventory()`                            |
| インベントリを閉じる   | 3/20 | CloseButton を押す                            |
| 会話スキップ           | 5/20 | タイピングスキップ / `dialogue_finished` 発火 |

実行後に「インベントリが開閉できる」「モードボタンが機能する」「SceneContainer が生きている」の 3 点を不変条件として検証する。

**再現性の確保:**  出力される `seed=XXXXXX` を `MONKEY_SEED=XXXXXX ./dev.sh monkey` で指定することで、同一の操作列を再実行できる。

#### 定量サマリ

| 指標           | 値                                                                                                   |
| -------------- | ---------------------------------------------------------------------------------------------------- |
| 新規ファイル   | `karakuri_monkey.gd`, `docs/testing_strategy.md`                                                     |
| 変更ファイル   | `karakuri_scenario_smoke.gd`（テスト追加）, `dev.sh`（コマンド追加）, `.agent/AGENTS.md`（方針更新） |
| C++ 修正行数   | +5 / -0（`on_clicked_at` ガード追加）                                                                |
| テスト実行結果 | smoke ✅ / e2e ✅ / monkey ✅（seed=1771995524, actions=400）                                           |
