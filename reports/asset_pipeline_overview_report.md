# Summary
- `docs/asset_pipeline_overview.md` を追加し、主軸 repo と各アセット生成ラインの責務境界を俯瞰できる叩き台を配置した。
- `docs/README.md` に導線を追加し、必要時に参照できる状態にした。

## Files Changed
- `docs/asset_pipeline_overview.md`: アセットパイプライン境界と共通契約の draft v0.1
- `docs/README.md`: 上記ドキュメントへの導線追加

## 変更理由
- 主軸 repo 側で「統合契約は持つが、各ライン実装詳細は持ち込まない」方針を固定するため。
- 次段の `asset_contracts` / `integration_map` 作成に先行する俯瞰ドキュメントが必要なため。

## 変更しなかった候補
- 候補A: 各アセットラインの詳細 schema まで本 repo に同時追加する。
  - 不採用理由: 境界定義フェーズを超えて実装詳細を持ち込み、責務分離方針に反するため。
- 候補B: README 更新を行わずドキュメント単体で追加する。
  - 不採用理由: 参照導線が弱く、運用時に発見性が下がるため。

## Tests Run
- `not run`（docs 変更のみ）

## Result
- 主軸 repo 配下に asset pipeline の俯瞰方針を配置し、参照導線まで反映した。

## Risks / Open Issues
- 現時点は overview のため、`asset_id` 命名規約や metadata schema の厳密定義は未固定。
- 各ライン repo 側の実在ツール仕様との整合は今後の `asset_contracts` で詰める必要がある。

## 次に着手すべき1タスク
- `docs/asset_contracts.md` を追加し、共通契約（ID / metadata / status / integration）を固定する。
