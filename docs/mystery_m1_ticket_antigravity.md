# Mystery M1 チケット（antigravity）

## タイトル
`[Karakuri][Mystery][M1] 入口統一と旧導線の整理`

## 背景
- Mystery デモは `karakuri_mystery_shell.tscn` 導線と旧導線が混在している。
- 今後の実装効率と不具合調査性を上げるため、実行入口を一本化する。
- 進捗管理は `TASK.md` の `Priority: Mystery Demo Execution (Owner: antigravity)` を正とする。

## 目的
- Mystery の起動導線を 1 本に統一する。
- 旧導線の扱い（削除 or 互換）を明示し、ドキュメントと実装の不整合をなくす。

## Scope
- 実行入口の統一（`karakuri_mystery_shell.tscn` を正式導線に固定）
- 旧導線の棚卸し（例: `samples/mystery/main_mystery.tscn` など）
- `README.md` と `docs/mystery_design.md` の起動手順同期
- `TASK.md` M1 項目の更新

## Out of Scope
- M2 以降（YAML スキーマ拡張、C++責務寄せ、多言語再実装など）
- 新機能追加（演出/シナリオ分岐増加/新UI）

## 実装タスク
- [ ] `project.godot` / 起動導線の実態を確認
- [ ] `samples/mystery/` 内の旧導線シーン・スクリプト参照を洗い出す
- [ ] 旧導線の扱いを決定して反映（削除または互換注記）
- [ ] 起動手順を `README.md` と `docs/mystery_design.md` に反映
- [ ] `TASK.md` の M1 項目を更新

## 受け入れ条件（DoD）
- [ ] `./dev.sh run mystery` で正式導線から起動できる
- [ ] 旧導線を辿らなくても開始から進行できる
- [ ] ドキュメント記載の起動手順が実装と一致する
- [ ] PR 本文に `Scope / Out of Scope / Test / Risks` がある

## テスト項目（最低限）
- [ ] 起動直後に `karakuri_mystery_shell.tscn` が使われることを確認
- [ ] メイン導線でゲーム進行が止まらないことを確認
- [ ] 旧導線参照が残る場合、意図と理由が PR に記載されている

## PR テンプレート（このチケット用）
```md
## Scope
- M1 の範囲で実施した内容

## Out of Scope
- M2以降の未対応事項

## Test
- `./dev.sh run mystery`
- 導線確認（正式導線）
- 旧導線の扱い確認

## Risks / Follow-up
- 残課題、次PRへ持ち越す項目
```

## 参照
- `TASK.md`
- `docs/mystery_antigravity_handover.md`
- `docs/mystery_design.md`
