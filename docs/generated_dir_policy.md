# scenarios/generated/ 運用方針 — Generator 生成物管理

**バージョン**: v1.0  
**作成日**: 2026-04-01  
**前提**: `docs/phase3_c_generator_entry.md` セクション 6・7 に基づく

---

## 1. ディレクトリ構成と追跡ルール

```
scenarios/generated/
│
├── .gitkeep                               # ディレクトリ確保。追跡対象 ✅
│
├── branching_basic_expected_output.yaml   # 期待出力サンプル。追跡対象 ✅
│                                          # validate 通過確認済み（Phase 3-B T10）
│
├── <name>_<YYYYMMDD_HHMMSS>.yaml          # Generator 生成物。追跡対象外 ❌
│                                          # HG-4 通過後に手動で git add する
│
└── <name>_<YYYYMMDD_HHMMSS>_review.md     # 作業メモ。追跡対象外 ❌（常に）
```

### .gitignore の設定内容

```gitignore
# タイムスタンプ付き YAML（HG-4通過後に手動登録）
scenarios/generated/*_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_[0-9][0-9][0-9][0-9][0-9][0-9].yaml
# review.md（バージョン管理しない）
scenarios/generated/*_review.md
```

---

## 2. review.md の位置づけ

### 何か

Generator（`tools/gen_scenario_branching.py` 等）が YAML と同時に生成する **HG-2 作業メモ**。

### 何でないか

- 常時参照するドキュメントではない
- `docs/` に配置するものではない
- バージョン管理する必要はない

### 記載内容

| 項目 | 内容 |
|:---|:---|
| 生成サマリ | template / start_scene / check_type / terminal シーン名 / 証拠フラグ一覧 |
| HG-2 チェックリスト | scene 名・check_type・location・result の目視確認項目 |
| よくある間違い | `few_shot_prompts.md` の Shot N から引用 |
| HG-3 コマンド | `python3 tools/validate_scenario.py <yaml>` の実行コマンド |

### ライフサイクル

```
Generator 実行
    │ <name>_<ts>_review.md 生成
    ▼
HG-2: 確認（review.md を開いてチェックリストを消化）
    │ review.md の役割終了
    ▼
コンフリクト防止のため削除推奨（gitignore 済みなので残しても無害）
```

---

## 3. 生成 YAML のライフサイクル

```
[Generator 実行]
    │ <name>_<ts>.yaml 生成（gitignore 対象）
    ▼
[HG-2] review.md でシーン構成・Task 列を目視確認
    ▼
[HG-3] validate_scenario.py → exit 0 確認
    │ exit 1 → SUGGEST を読んで手動修正 → 再 validate
    ▼
[HG-4] Godot runtime 煙テスト（3 経路）
    ▼
[正式採用] 人間が手動で git add → commit
    │  git add -f scenarios/generated/<name>_<ts>.yaml
    │  または scenarios/<game_name>.yaml に改名してから add
    ▼
[長期保管] scenarios/ (non-generated) に配置を推奨
```

### HG-4 通過後の git add 手順

```bash
# 方法 1: generated/ のまま登録
git add -f scenarios/generated/<name>_<ts>.yaml
git commit -m "scenarios: <game_name> — HG-4 通過済み"

# 方法 2（推奨）: 改名して scenarios/ に移動
mv scenarios/generated/<name>_<ts>.yaml scenarios/<game_name>.yaml
git add scenarios/<game_name>.yaml
git commit -m "scenarios: <game_name> — HG-4 通過済み"
```

---

## 4. 古い生成物の削除方針

`scenarios/generated/` に HG-4 未通過のファイルが蓄積した場合:

```bash
# review.md と未通過 YAML を一括削除（gitignore 済みなので git 管理外）
# 期待出力サンプルと .gitkeep は削除しない
ls scenarios/generated/ | grep -v "expected_output\|\.gitkeep"
# 問題なければ:
rm scenarios/generated/*_[0-9]*_[0-9]*.yaml
rm scenarios/generated/*_review.md
```

削除は **人間が判断して手動実行**する。Generator が自動削除することはない。

---

## 5. 期待出力サンプルの管理

| ファイル | 管理方針 |
|:---|:---|
| `branching_basic_expected_output.yaml` | 追跡済みの品質基準。削除・上書き禁止 |
| 追加テンプレートの期待出力サンプル | 各 Generator 実装時に人間が手動で commit する |

期待出力サンプルは **Generator の内部仕様変更時のみ更新し、必ず `validate_scenario.py` exit 0 確認後に commit する**。

---

## 参照ドキュメント

| ファイル | 役割 |
|:---|:---|
| `docs/phase3_c_generator_entry.md` | Generator 全体方針・HG 定義 |
| `docs/t11_gen_branching_completion.md` | gen_scenario_branching.py の出力仕様詳細 |
| `.gitignore` | タイムスタンプ付き生成物の除外パターン |
