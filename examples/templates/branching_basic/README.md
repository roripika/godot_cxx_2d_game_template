# Branching Basic Template

**カテゴリ**: Evidence / Branching / Multi-Ending  
**参照実装**: `src/games/mystery_test/`  
**完了メモ**: `docs/mystery_test_completion.md`

---

## このテンプレートが解決すること

プレイヤーが「事実（証拠）」を収集し、集まった証拠の組み合わせを  
`all_of` / `any_of` 条件で評価して、複数のエンディングに分岐するゲームロジックを  
**`src/core` を一切変更せずに** Kernel 上に載せるための最小構成を示します。

---

## いつ使うか

| 条件 | 判断 |
| :--- | :--- |
| プレイヤーが「証拠・事実・フラグ」を収集していく | ✅ 使う |
| 収集した情報の組み合わせで結末が変わる | ✅ 使う |
| 「全条件を満たしたら true」「どれか 1 つでも true」の評価が必要 | ✅ 使う |
| 複数のエンディング（solved / wrong / neutral など）が存在する | ✅ 使う |
| ビジュアルノベル・推理ゲーム・ADV・選択肢ゲーム | ✅ 使う |
| ターン進行・HP 管理・グリッド移動が主軸 | ❌ 使わない（Turn/Grid テンプレートを使う） |
| リアルタイム物理・タイミング判定が主軸 | ❌ 使わない（Time/Clock テンプレートを使う） |

---

## このテンプレートがカバーしない領域

- 3 つ以上の証拠グループを持つ複雑な論理式（AND/OR のネスト）
- HP ストライク制・ペナルティ付き選択肢
- 証拠インベントリの UI 表示
- セーブ／ロードの本番運用
- リアルタイム入力・クリック判定との統合
- 国際化・翻訳対応

---

## テンプレートの構成

```
examples/templates/branching_basic/
├── README.md          ← このファイル（用途・境界説明）
├── contract.md        ← WorldState キー仕様 + Task 契約 + スコープ混用禁止ルール
├── scenario/
│   └── branching_smoke.yaml  ← solved/wrong の 2 経路を網羅する最小 YAML
└── verification.md    ← 動作確認手順 + 実測値リファレンス
```

---

## 参照実装からの学び

`mystery_test` は以下の設計判断を検証済みです。

1. **証拠フラグは「SESSION か GLOBAL か」で Task を使い分ける**  
   - `discover_evidence` → SCOPE_SESSION（現セッション内の発見）  
   - `add_evidence` → SCOPE_GLOBAL（セッションをまたぐ動機・事実）  
   - 読み取り側も対応する: `check_condition`（SESSION）/ `check_evidence`（GLOBAL）  
   この 2 ペアを混用すると判定が常に false になる（詳細: `contract.md`）。

2. **条件評価は `check_condition` 1 点に集約する**  
   `all_of` または `any_of` の配列で複数証拠を評価し、`if_true` / `if_false` で  
   分岐先シーン名を渡す。Task が直接シーンを呼び出してはいけない。

3. **証拠配置は YAML で宣言的に記述する**  
   `discover_evidence` を `on_enter` に並べるだけで証拠が積まれる。  
   ロジックを Task に書かず、YAML が「何を発見するか」を制御する。

4. **FakeCommand は不要（mystery パターンの特徴）**  
   mystery_test はプレイヤーコマンドがなく、YAML 自体が証拠配置を決定する  
   決定論的モデルです。smoke シナリオは FakeCommand なしで再現可能です。

5. **ループなし・1 回で終結するシナリオ**  
   Turn/Grid と異なり、証拠収集フェーズが完了すると必ずエンディングに到達します。  
   「continue してboot に戻る」ループはこのパターンには存在しません。

6. **pos-0 sacrifice パターン**  
   シーン遷移後 pos-0 がスキップされる既知バグを回避するため、  
   非 start シーン（accuse / ending 等）の pos 0 にダミーを置く。  
   詳細: `docs/infrastructure_backlog.md`

---

## 新しいゲームに適用する手順（概要）

1. `src/games/<your_game>/tasks/` に必要な Task クラスを実装する（最小: `discover_evidence`, `check_condition`, `end_game`）。
2. `scenario/` 以下に `branching_smoke.yaml` を参考とした YAML を配置する。
3. `contract.md` の WorldState キー設計を自分のゲーム用に埋める。
4. `verification.md` の確認手順で全エンディング経路を runtime 検証する。

実装の出発点としては `src/games/mystery_test/` をリファレンスとして読む。  
コードはコピーせず、Task の**責務定義**と**WorldState 読み書きスコープ**を参照すること。
