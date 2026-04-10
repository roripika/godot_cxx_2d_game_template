# Summary
- `docs/audio_pipeline_rollout_note.md` を新規作成し、IB-06（audio line rollout note）の初期方針を定義した。
- SFX / BGM / voice を切り分け、最初の rollout 範囲を SFX/BGM に限定し、voice を初期除外する理由を明文化した。
- 主軸 repo と audio line repo の責務境界、および UI line 文書型の転用方針を整理し、次段タスク起票の前提を揃えた。

## Files Changed
- `docs/audio_pipeline_rollout_note.md`: IB-06 本体文書を追加（切り分け、初期対象、責務境界、転用方針、保留条件）
- `docs/integration_backlog.md`: IB-06 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `audio_pipeline_rollout_note.md` への最小導線を追加
- `reports/audio_pipeline_rollout_note_report.md`: 本タスクの実施記録を追加

## 変更理由
- UI line の次段として audio line を接続するため、SFX/BGM/voice を同時に扱わず段階導入の境界を固定する必要があるため。
- 主軸 repo に生成実装の詳細を持ち込まず、契約・Gate・統合記録を管理する責務分離を維持するため。

## 変更しなかった候補
- 候補A: 初期 rollout に voice を含める。
  - 不採用理由: 台本・話者・字幕同期など依存が多く、最小成功例としての切り分けが難しいため。
- 候補B: audio line の内部エンジン設定や詳細ログ規約まで主軸 repo で定義する。
  - 不採用理由: 主軸 repo の責務境界を越えるため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- IB-06 の完了条件（audio line 次段導入の責務表と順序整理）を満たした。
- 次段として `audio_integration_contract.md` を起票し、SFX/BGM の入出力契約へ進める状態になった。

## Risks / Open Issues
- 最初に扱う audio 具体対象（`sfx/ui/click_001` か `bgm/title/theme_a` か）は未確定のため、次タスクで 1 件に固定が必要。
- audio line 側の実際の出力フォーマット差異がある場合、契約項目の微調整が必要。

## 次に着手すべき1タスク
- `audio_integration_contract.md` を起票し、SFX / BGM の入力・出力・status・失敗時契約を固定する。
