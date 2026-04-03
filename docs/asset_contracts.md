# Asset Contracts

**ステータス**: draft v0.1  
**作成日**: 2026-04-03  
**目的**: 各アセット生成ラインが `godot_cxx_2d_game_template` と協調できるよう、共通の asset contract を定義する。

---

## 1. 目的

本ドキュメントは、UI / 音 / ボイス / キャラ画像 / モーション / エフェクト / 動画など、
複数のアセット生成ラインが主軸 repo と連携するための共通契約を定義する。

目的は以下。

1. アセット種別が異なっても、同じ考え方で管理できるようにする
2. 各ラインの内部実装を主軸 repo に持ち込まずに、統合可能にする
3. Human Gate / review / integration の判断を統一しやすくする

---

## 2. 適用範囲

本 contract は以下に適用する。

- UI asset pipeline
- audio asset pipeline（SFX / BGM / voice）
- character image pipeline
- motion pipeline
- effect pipeline
- video pipeline

scenario generator 本体は別系統だが、asset metadata / status / integration の観点では同じ思想を参照する。

---

## 3. 共通 Asset ID

全アセットは一意な `asset_id` を持つ。

### 3-1. 推奨形式

```text
<asset_type>/<group>/<name_or_variant>
```

例:

- `ui/button/primary_001`
- `sfx/ui/click_001`
- `bgm/battle/theme_a`
- `voice/hero/line_012`
- `char/hero/base_pose_a`
- `motion/hero/attack_slash_01`
- `fx/slash/light_01`
- `movie/op/opening_v1`

### 3-2. ルール

- asset type を先頭に置く
- group はゲーム内カテゴリまたはキャラクタ単位で分ける
- rename は極力避ける
- バリエーション差分は suffix で表現する
- 採番規則は各ラインで持ってよいが、重複しないこと

---

## 4. 共通 Metadata

全アセットは最低限以下の metadata を持つ。

| キー | 必須 | 内容 |
| :--- | :---: | :--- |
| `asset_id` | 必須 | 一意な ID |
| `asset_type` | 必須 | `ui` / `sfx` / `bgm` / `voice` / `char` / `motion` / `fx` / `movie` |
| `source_spec` | 推奨 | 入力 spec / prompt / script への参照 |
| `generator_name` | 推奨 | 生成ライン名 |
| `generator_version` | 推奨 | generator の版 |
| `model_or_backend` | 推奨 | 生成モデルまたは backend |
| `created_at` | 必須 | 生成日時 |
| `status` | 必須 | 状態遷移値 |
| `reviewed_by` | 任意 | review 実施者 |
| `approved_by` | 任意 | 採用承認者 |
| `usage_scope` | 推奨 | 使用画面 / 使用機能 / 使用シーン |
| `notes` | 任意 | 補足情報 |

---

## 5. Status 契約

すべてのアセットは以下の status を持つ。

| status | 意味 |
| :--- | :--- |
| `draft` | 仕様段階。生成未実行または草案 |
| `generated` | 生成済み。未レビュー |
| `reviewed` | レビュー済み。採否未確定 |
| `approved` | 採用可 |
| `integrated` | ゲーム本体へ統合済み |
| `deprecated` | 廃止。新規利用しない |

### 5-1. 原則

- `generated` → `approved` の直行は避ける
- `approved` 前に少なくとも 1 回は Human Gate を通す
- `deprecated` は削除ではなく状態で表現する

---

## 6. Human Gate 契約

アセット種別が違っても、Gate の考え方は共通化する。

| Gate | 目的 | 共通観点 |
| :--- | :--- | :--- |
| AG-1 | 生成前確認 | 入力仕様 / 世界観 / 用途の妥当性 |
| AG-2 | 生成直後レビュー | 明らかな破綻、形式不備、用途逸脱の有無 |
| AG-3 | 品質確認 | ループ、ノイズ、破綻、視認性、整合性 |
| AG-4 | 統合前確認 | ゲーム内利用に耐えるか、他資産と整合するか |

### 6-1. 判定値

各 Gate の結果は以下で表現する。

- `PASS`
- `REWORK`
- `STOP`

### 6-2. 原則

- `PASS`: 次工程へ進む
- `REWORK`: 修正して同じ Gate へ戻る
- `STOP`: 実装・統合を止めて設計判断へ戻す

---

## 7. 出力契約

各ラインは、少なくとも以下を返せることが望ましい。

1. 生成物本体
2. metadata
3. 生成ログまたは report
4. 失敗時の理由

### 7-1. 例

- UI: `svg/png/pdf + metadata`
- audio: `wav/ogg + metadata`
- voice: `wav/ogg + line mapping`
- motion: `animation clip + metadata`
- video: `mp4/webm + metadata`

---

## 8. 配置契約

主軸 repo への最終統合時は、生成ライン側の内部出力先ではなく、
主軸側が定める integration ルールに従う。

原則:

- 生成途中成果物は各ライン repo 側に保持
- 採用済み成果物のみ主軸 repo または最終配布先へ統合
- タイムスタンプ付き作業出力は原則 git 管理対象外
- 長期採用物は stable な配置へ移す

---

## 9. 失敗時契約

生成失敗時は、少なくとも以下を残す。

- どの入力で失敗したか
- どの工程で失敗したか
- 再現可能か
- `REWORK` か `STOP` か
- 次に人間が見るべき論点

---

## 10. 主軸 repo に置くもの / 置かないもの

### 主軸 repo に置く

- asset type 一覧
- ID 契約
- metadata 契約
- status 契約
- Human Gate 契約
- integration ルール

### 主軸 repo に置かない

- 各ラインの内部 prompt 詳細
- 各ラインの compiler / renderer 実装
- 各ライン固有の細かいテスト仕様
- モデル依存の運用メモ

---

## 11. 次アクション

1. `asset_pipeline_overview.md` と本 contract を相互参照にする
2. `integration_map.md` を追加し、各 asset line の接続先を明示する
3. `human_gate_asset_policy.md` を追加し、asset type ごとの AG 観点差を固定する
4. UI line を最初の具体例として contract 適用する
5. audio / character / motion / fx / movie を同型で展開する
