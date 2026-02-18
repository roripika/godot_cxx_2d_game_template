# デモ計画書 (Universal Game Template)

## 目的
このリポジトリの「デモ」は完成ゲームではなく、**別ジャンルのゲームを最短で試作するための再利用可能な設計/Utilityの見本**として用意する。

- デモは「作れる」を示す最小実装 + 伸ばし方を示す
- 可能な限り同じ Utility / 同じ設計で横展開できる形にする
- GodotシーンはView（見せ方/操作）に寄せ、Core（データ/ルール）はC++(GDExtension)に寄せる

---

## 共通Utility（全デモ共通で持つべきもの）

### 1. Basic Game Karakuri（Autoload / Utility Framework）
「どのデモでも必ず使う」入口。ここに依存させると横展開が楽になる。

- `LocalizationService`
  - localeの永続化（`user://`）
  - `locale_changed` 通知（UI再描画のトリガ）
- `SceneFlowService`
  - シーン遷移（フェード/ロード/戻る）
  - `push/pop` 的な簡易スタック（デモ内の戻るを統一）
- `SaveDataService`
  - デモ別の進捗を保存/復元（最小: JSON）
- `DebugService`
  - FPS表示、ログ、テスト用コマンド（例: フラグを立てる、シーンを飛ぶ）

### 2. Common UI（samples/common）
デモごとに作り直さないコンポーネント群。

- 共通メニュー/戻るボタン（すでに `samples/common/common_demo_ui.gd` が存在）
- 共通トースト/通知
- 共通ダイアログ（タイトル/確認/エラー）

### 3. Dataルール
「翻訳キー」「ID」「セーブキー」など、運用で崩れやすい命名を固定する。

- 翻訳キー: `demo.<demo_id>.<category>.<name>` 形式を推奨
- Evidence/Item ID: `snake_case`（表示名は翻訳で出す）
- フラグ: `snake_case`（プレフィクスで衝突回避: `mystery.case_solved` 等）

### 4. UIルール（全デモ共通）
- UIのグループ分け/重なり順/アンカー設計は `docs/ui_layout_policy.md` を必須準拠とする。
- デモ固有の詳細は各デモの UI 拡張ポリシーに追記する（例: `docs/mystery_ui_layout_policy.md`）。

---

## デモ一覧（何を作るか）

各デモは「最小ループ」と「再利用したい要素」を明確にする。

### 1. Adventure（Mystery）
**狙い**: 逆転裁判/都市伝説スタイルの「調査→整理→対決→分岐」を、再利用可能な部品として確立する。

- 最小ループ（MVP）
  - Office（導入）→ Warehouse（調査）→ Office（推理）→ Warehouse（対決）→ Ending
- 再利用したいUtility
  - Localization（英日切替、即時反映、永続化）
  - Dialogue/Choices/Typewriter
  - Evidence/Inventory UI
  - Hotspot/Investigation の当たり判定とテキスト表示
  - SceneFlow（戻る/フェード/スキップ）
- 完了条件（MVP）
  - `./dev.sh run mystery` で最後まで通る
  - 言語切替が「全UI」に反映される（現在表示中含む）
  - 直書きテキストが残らない（原則：翻訳キー経由）

詳細は `docs/mystery_design.md` にまとめる（この計画書ではスコープ/完了条件を固定する）。
YAML運用仕様は `docs/mystery_yaml_schema_v1.md` を参照する。

### 2. Roguelike（Iso）
**狙い**: Coreのワールドデータ生成と、Viewの描画アダプタを示す（PCGのテンプレ）。

- 最小ループ
  - シード指定 → 生成 → プレイヤー配置 → 再生成
- 再利用したいUtility
  - Seed管理、生成ログ、デバッグ表示（部屋数/経路）
  - SaveData（seed保存/再現）
- 完了条件
  - シードで同一生成が再現できる

### 3. Platformer（Side）
**狙い**: Controller/Physicsのテンプレ。入力・当たり判定・カメラ等を持ち回れるようにする。

- 最小ループ
  - 移動/ジャンプ/ダメージ/リスポーン
- 再利用したいUtility
  - Input mapping（共通アクション名）
  - 画面外落下/チェックポイント

### 4. Fighting（Side VS）
**狙い**: フレーム単位の状態機械と、Hitbox/Hurtboxの基礎。

- 最小ループ
  - 2体スポーン → 基本技 → ヒットストップ → 勝敗
- 再利用したいUtility
  - InputBuffer、状態遷移ログ、当たり判定デバッグ可視化

### 5. Sandbox（Mining/Building）
**狙い**: ワールド改変 + 永続化（セーブ/ロード）の見本。

- 最小ループ
  - 採掘→アイテム入手→配置→保存→復帰
- 再利用したいUtility
  - Chunk/SaveData、インベントリ、クラフトレシピ

### 6. Rhythm
**狙い**: タイムライン/判定/スコアのテンプレ。データ駆動で譜面を差し替えられる形にする。

- 最小ループ
  - BPM設定→ノーツ生成→判定→リザルト
- 再利用したいUtility
  - 判定可視化、オーディオ同期のテスト

### 7. Gallery（Asset）
**狙い**: アセット確認/差し替えの場。デザイナーが「今ある素材」をすぐ見れる。

- 最小ループ
  - アセット一覧→プレビュー→タグ/検索

---

## Adventure（Mystery）で「実現しておきたいこと」（決め事）

### スコープ（MVP）
- Scene: `office` / `warehouse_investigation` / `office_deduction` / `warehouse_confrontation` / `ending`
- Data: Evidence 3点 + フラグ数個 + HP（ペナルティ）
- UI:
  - Dialogue（タイプライター、スキップ）
  - Choices（最低1箇所）
  - Evidence Inventory（一覧 + 詳細）
  - Language Switch（英/日、即時反映、永続化）

### “再利用できる形” の条件
- ルール/進捗（flags, evidence, hp）は **Core側（C++/Resource/Autoload）** を正とする
- View側（GDScript/tscn）は「表示と入力」だけに寄せる
- 文字列は `tr("key")`（or C++側は TranslationServer.translate）経由に統一する

### テスト観点（最低限）
- `./dev.sh run mystery` で開始からエンディングまで到達できる
- 途中で言語切替しても崩れない（表示更新/選択肢/インベントリ）
- 証拠IDが安定している（表示名と混ぜない）

---

## 次の作業（この計画書を現実にするための優先度）
1. Adventureの導線を1本化（C++主導 or GDScript主導を決めて統一）
2. Basic Game Karakuri（Autoload）を導入し、locale永続化とUI再描画を共通化
3. 直書きテキストの排除（翻訳キーの運用に寄せる）
