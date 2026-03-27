# Turn/Grid Basic Template

**カテゴリ**: Turn / Entity / Grid  
**参照実装**: `src/games/roguelike_test/`  
**完了メモ**: `docs/roguelike_test_completion.md`

---

## このテンプレートが解決すること

ターン制・グリッド上でエンティティが動き HP / 位置が変化し、1 ターンの結果として
`clear` / `fail` / `continue` の 3 分岐を持つゲームロジックを  
**`src/core` を一切変更せずに** Kernel 上に载せるための最小構成を示します。

---

## いつ使うか

| 条件 | 判断 |
| :--- | :--- |
| プレイヤー・敵が「ターン」単位で交互に行動する | ✅ 使う |
| グリッド座標 (x, y) で位置を管理する | ✅ 使う |
| HP / 体力のような数値消耗リソースがある | ✅ 使う |
| 1 ターンの終わりに勝敗・継続を判定する | ✅ 使う |
| リアルタイム入力や物理シミュレーションが主軸 | ❌ 使わない |
| 盤面が 50×50 を超え手続き生成が必要 | ❌ 使わない（このテンプレートの外） |

---

## このテンプレートがカバーしない領域

- 複数エネミー汎化（enemy_N の動的生成）
- FOV / 視野計算
- インベントリ・アイテムシステム
- 手続き的マップ生成
- リアル入力統合（現状は FakeCommand ベース）
- 戦闘エフェクト・プロダクション UI

---

## テンプレートの構成

```
examples/templates/turn_grid_basic/
├── README.md          ← このファイル（用途・境界説明）
├── contract.md        ← WorldState キー仕様 + Task 契約
├── scenario/
│   └── turn_grid_smoke.yaml  ← clue/fail/continue を網羅する最小 YAML
└── verification.md    ← 動作確認手順 + 実測値リファレンス
```

---

## 参照実装からの学び

`roguelike_test` は以下の設計判断を検証済みです。

1. **Task は 6 種の単一責務に分割する**  
   `setup_round` → `load_command` → `apply_player_move` → `apply_player_attack`  
   → `apply_enemy_turn` → `resolve_turn` → `evaluate_round`  
   各 Task は WorldState への読み書きのみで副作用を持たない。

2. **シーン分岐は `evaluate_round` 1 点に集約する**  
   Kernel の ScenarioRunner がシーン遷移を管理するため、Task が直接シーンを  
   呼び出してはいけない。`if_clear:` / `if_fail:` / `if_continue:` payloadで  
   分岐先シーン名を渡す。

3. **FakeCommand で deterministic smoke を取る**  
   `load_fake_player_command` Task が YAML の `command: move_down` 等を  
   WorldState に書き込むことで、入力なしで再現可能なシナリオになる。

4. **pos-0 sacrifice パターン**  
   シーン遷移直後 pos-0 がスキップされる既知バグを回避するため、  
   boot 以外のシーンの pos 0 には同一 Task の複製ダミーを置く。  
   詳細: `docs/infrastructure_backlog.md`

---

## 新しいゲームに適用する手順（概要）

1. `src/games/<your_game>/tasks/` に 6 種前後の Task クラスを実装する。  
2. `scenario/` 以下に `turn_grid_smoke.yaml` を参考とした YAML を配置する。  
3. `contract.md` の WorldState キー設計を自分のゲームで埋める。  
4. `verification.md` の確認手順で 3 経路を runtime 検証する。

実装の出発点としては `src/games/roguelike_test/` をリファレンスとして読む。  
コードはコピーせず、Task の**責務定義**と**WorldState 読み書き境界**を参照すること。
