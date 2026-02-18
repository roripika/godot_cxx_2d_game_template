# Mystery UI レイアウト方針（デモ固有拡張）

> 共通ルールは `docs/ui_layout_policy.md` を参照。
> 本ドキュメントは Mystery デモの固有設計のみを扱う。

## 1. 目的
- 共通UIポリシーに対して、Mysteryデモ固有のノード割り当てと運用ルールを定義する。
- デザイナー/プランナー/実装者の誰が触っても、同じ優先順位ルールで UI を保守できる状態にする。

## 2. 基本ルール（再掲）
- UI は同じ画面内で内容物ごとにグループ分けする。
- 同一グループ内で UI を重ねない（同一グループ内の重なりは設計違反）。
- アクション/タップにより、別グループ UI が一時的に上へ重なることは許可する。
- グループごとの優先順位は固定する。
- 子グループ（サブ階層）を定義して運用してよい。
- エフェクト UI は「発生文脈」によって優先順位を変更できる。
- グループ/子グループ/各パーツの階層設計はドキュメント台帳で管理する。

## 3. グループ優先順位（上が前面）
1. システムUIグループ
2. インスタントサブ情報UIグループ
3. メイン情報UIグループ
4. サブ情報UIグループ

Godot 実装では、原則として `CanvasLayer.layer` または `z_index` で表現する。

## 4. グループ定義（Mystery固有の解釈）
### 4.1 システムUIグループ（最前面）
- 役割: ゲーム進行に依存しない常設操作/状態表示。
- 例: 言語切替、戻る、HP、全体ガイド。

### 4.2 インスタントサブ情報UIグループ
- 役割: 短時間で消える補助情報。
- 例: トースト、ヒント、即時警告、短時間オーバーレイ。

### 4.3 メイン情報UIグループ
- 役割: 進行の主文脈を担う UI。
- 例: 会話パネル、証言パネル、選択肢、進行操作。

### 4.4 サブ情報UIグループ（最背面）
- 役割: 参照系の補助情報。
- 例: 証拠リスト、詳細パネル、用語ヘルプ。

## 5. 子グループ運用
- 各グループに子グループを置いてよい。
- 子グループ内でも「重なり禁止」を守る。
- 子グループは「領域分割（アンカー）」で干渉を避ける。

例:
- システムUI
  - `system.top_left`（HP/ガイド）
  - `system.top_right`（言語切替/メニュー）
- メイン情報UI
  - `main.bottom`（会話）
  - `main.right`（証言）

## 6. エフェクトUIの優先順位ルール
- 通常の視覚強調（点滅/ハイライト）:
  - 親 UI と同一グループ内で一段上に表示する。
- 注意喚起（警告/失敗通知）:
  - インスタントサブ情報UIへ昇格表示する。
- 画面遷移/フェードなど全画面制御:
  - 一時的にシステムUIより上に表示してよい。
  - 終了後は必ず破棄し、常設しない。

## 7. アンカー設計ルール
- 主要 UI は固定座標ではなくアンカー比率で配置する。
- グループごとに担当矩形を決め、同一グループ内で矩形を共有しない。
- `project.godot` のストレッチ設定とセットで管理する。
- 最小確認解像度:
  - 1152x648（基準）
  - 1280x720（16:9）
  - 1170x2532 相当の縦長比率（モバイル想定）

## 8. 階層設計台帳（管理フォーマット）
以下を UI 変更PRごとに更新する。

| group | child_group | node_path | scene | layer/z | anchor_rect | overlap_in_group | owner |
|---|---|---|---|---|---|---|---|
| system | system.top_left | `CanvasLayer/HealthBar` | `samples/mystery/karakuri_mystery_shell.tscn` | system | 例: x0.02-0.28, y0.02-0.10 | no | designer |
| main | main.bottom | `CanvasLayer/DialogueUI` | `samples/mystery/karakuri_mystery_shell.tscn` | main | 例: x0.30-0.98, y0.64-0.98 | no | designer |

## 9. レビュー観点（UI専用）
- 同一グループ内で重なりが発生していない。
- グループ優先順位が崩れていない。
- 一時表示 UI が表示後に適切に閉じる。
- 新規 UI パーツが台帳に追記されている。

## 10. 運用
- 本方針が Mystery の UI レイアウト基準。
- 例外を入れる場合は、PR本文に理由と影響範囲を明記する。

## 11. Mystery UI グループ設計（現行 v1）
以下は `samples/mystery/karakuri_mystery_shell.tscn` を対象にした、現行の正式グループ割り当て。

### 11.1 グループ優先度ID（運用値）
- `system`: 300
- `instant_subinfo`: 200
- `main`: 100
- `sub`: 0

### 11.2 ノード割り当て台帳
| group | child_group | node_path | 役割 | anchor_rect（概略） | overlap_in_group |
|---|---|---|---|---|---|
| system | system.top_left | `SystemUiLayer/UiGuideLabel` | 全体ガイド文言 | x0.28-0.74, y0.01-0.06 | no |
| system | system.top_left | `SystemUiLayer/HealthLabel` | HPラベル | x0.02-0.24, y0.02-0.06 | no |
| system | system.top_left | `SystemUiLayer/HealthBar` | HP本体 | x0.02-0.24, y0.06-0.11 | no |
| system | system.top_right | `SystemUiLayer/LanguageSwitcher` | 言語切替 | 右上固定 | no |
| instant_subinfo | instant.left_modal | `InstantSubInfoUiLayer/InventoryUI` | 証拠一覧/詳細（モーダル表示） | x0.08-0.52, y0.12-0.88 | no |
| main | main.bottom | `MainInfoUiLayer/DialogueUI` | 会話表示/選択肢 | x0.24-0.98, y0.63-0.98 | no |
| main | main.right | `MainInfoUiLayer/TestimonySystem` | 証言/対決操作 | x0.66-0.98, y0.12-0.58 | no |

### 11.3 一時的な重なり許可（イベント起点）
- `InventoryUI` は「証拠確認/提示アクション」で表示し、`MainInfoUiLayer` の上に一時表示してよい。
- `TestimonySystem` は対決開始時に表示し、非対決時は非表示に戻す。
- `LanguageSwitcher` と `HealthBar` は常時最前面維持。

### 11.5 レイヤー実装値（現行）
- `SystemUiLayer.layer = 30`
- `InstantSubInfoUiLayer.layer = 20`
- `MainInfoUiLayer.layer = 10`
- `SubInfoUiLayer.layer = 0`

### 11.4 エフェクトUI割り当て
- 会話強調エフェクト: `main.bottom` 内で `DialogueUI` 上に表示。
- 証拠提示成功/失敗の短時間通知: `instant_subinfo` に昇格して表示。
- フェード/暗転: 一時的に `system` より前面で表示し、遷移終了後に破棄。
