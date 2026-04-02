# Summary
- `docs/asset_contracts.md` と `docs/integration_map.md` を新規作成し、主軸 repo で保持するアセット連携契約と統合接続マップを定義した。
- 既存 `docs/asset_pipeline_overview.md` と同系統の文体で、目的→方針→表→次アクションの流れに統一した。
- `docs/README.md` に最小導線を追加し、3文書（overview/contracts/map）へ到達しやすくした。

## Files Changed
- `docs/asset_contracts.md`: asset ID / metadata / status / Human Gate / 出力・配置契約を定義
- `docs/integration_map.md`: レイヤ別接続先、機能別依存、統合優先順、初期連携モデルを定義
- `docs/README.md`: 新規2文書への導線追加

## 変更理由
- アセット生成ライン分離方針を、主軸 repo 側の契約として固定し、今後の UI/audio 連携タスクの前提を明確化するため。
- 各ラインの内部実装を持ち込まずに統合判断できる最小情報（契約と接続先）を先に揃えるため。

## 変更しなかった候補
- 候補A: `human_gate_asset_policy.md` まで同時作成する。
  - 不採用理由: 今回は contracts/map の2本追加が主目的で、同時拡張はスコープ過多になるため。
- 候補B: 各アセットラインの詳細 schema まで主軸 repo に書き込む。
  - 不採用理由: 境界分離方針（詳細は各ライン repo 管理）に反するため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- 主軸 repo に asset 連携の共通契約と統合マップが追加され、`asset_pipeline_overview` と合わせて3点セットの入口が成立した。

## Risks / Open Issues
- `asset_id` 命名規約の厳密バリデーション（正規表現・重複検知）は未定義。
- `status` 遷移の機械検証ルール（遷移禁止条件など）は今後の具体化が必要。
- UI line を最初の適用対象として具体タスク化する必要がある。

## 次に着手すべき1タスク
- `docs/human_gate_asset_policy.md` を追加し、AG-1〜AG-4 の判定基準をアセット種別ごとに明文化する。
