# Rhythm Test 差分監査レポート

## 1. 全体所見
`rhythm_test` の導入差分について監査を実施しました。

- **妥当性**: `KernelClock` を基軸とした決定論的なリズム判定ロジックが、Kernel v2.0 の既存契約に則って「Task と WorldState のみ」で完結して実装されています。
- **設計意図との一致**: 演出や音声同期を排し、ミリ秒単位の「論理判定」に特化したフィットネステストとしてのスコープを厳守しています。
- **安全性**: `rhythm_run_engine` のような巨大な Task を避け、`advance / judge / resolve / evaluate` の各ステップに責務を分離したことで、AI によるシナリオ構築が容易な構造になっています。

---

## 2. `src/core` 非変更確認
- **判定**: **非変更**
- **詳細**: `src/core/` 配下のファイルに差分はありません。`ScenarioRunner`, `WorldState`, `KernelClock` 等の public contract は一切変更・拡張されておらず、既存の Core をブラックボックスとして扱ったまま新機能が実装されています。

---

## 3. WorldState contract 監査
- **判定**: **合格 (Namespace & Type 遵守)**
- **使用キー一覧**:
    - `rhythm_test:chart:index` (int): 現在のターゲットノーツ索引
    - `rhythm_test:chart:note_count` (int): 総ノーツ数
    - `rhythm_test:clock:now_ms` (int): クロック同期値
    - `rhythm_test:tap:last_time_ms` (int): 確定したタップ時刻
    - `rhythm_test:judge:last_result` (String): 直近の判定結果 ("perfect", "miss" 等)
    - `rhythm_test:judge:perfect_count/good_count/miss_count` (int): 判定カウンター
    - `rhythm_test:round:result` (String): "clear", "fail" 等の勝敗
- **監査結果**: `rhythm_test:` ネームスペースに閉じており、全ての状態が primitive な型に分解されています。描画専用の状態も混入しておらず、クリーンな設計です。

---

## 4. `register_types.cpp` 監査
- **判定**: **合格**
- **差分内容**: `rhythm_test` 用の 6 つの Task と 1 つの Game クラスの登録のみ。
- **副作用**: 初期化順序の変更や不要なグローバル変数の追加、クラッシュ回避策の混入はなく、純粋なプラグイン的追加に留まっています。

---

## 5. YAML 構造監査
- **判定**: **合格 (既存構造遵守)**
- **構造**: `boot -> advance -> judge -> resolve -> check_round -> advance` という明確なループ構造。
- **制御フロー**: 毎ステップで `evaluate_rhythm_round` を挟むことで、Miss 制限による即時 Fail や最終 Clear 判定を標準的なシーン遷移で実現しています。
- **Corrupted Scenario**: ノーツ順序の逆転、要素数不一致、未定義アクションなどを網羅し、バリデータの有効性を証明しています。

---

## 6. Task 差分監査
- **判定**: **合格 (責務分離適切)**
- **実装 Task**:
    - `setup_rhythm_round`: 初期化と譜面展開
    - `load_fake_tap`: フェイク入力の抽出
    - `advance_rhythm_clock`: クロック進行（`KernelClock::advance`）
    - `judge_rhythm_note`: ウィンドウ判定ロジック
    - `resolve_rhythm_progress`: インデックス更新とカウンタ計上
    - `evaluate_rhythm_round`: 分岐判定
- **評価**: 1つの Task が複数のメタ状態を持たず、ステートマシンとしての Kernel の強みを活かした原子的な実装になっています。`fake_tap` 方針も遵守されており、決定論的な再現性が担保されています。

---

## 7. validator / regression 監査
- **判定**: **合格**
- **検証結果**: `validate_scenario.py` の修正により、`rhythm_test` 特有のペイロード（昇順ノーツ時刻、ウィンドウ幅の妥当性など）が静的にチェックされます。
- **回帰性**: 既存の `mystery_test` や `billiards_test` を壊すことなく、17/17 PASS を維持していることを確認しました。

---

## 8. docs 監査
- **判定**: **合格**
- **整合性**: `rhythm_test.md` 等のドキュメントは実装実態と正確に一致しており、将来的な `real input` に向けた切り分けも適切に行われています。

---

## 9. 問題点一覧
- **ブロッカー**: なし。
- **軽微なフォローアップ**: 譜面のノーツ数が現在 3〜5 個に制限（`kMaxNotes`）されていますが、これはフィットネステストとしての制限であり、実運用に向けた拡張時には動的な配列管理が必要になります。現時点では Accept 可能です。

---

## 10. 総合判定
**Accept**

**判定理由**:
Kernel v2.0 の設計思想である「Core 非変更」「Task による拡張」「論理状態の WorldState 集約」を高い精度で具現化した差分です。時間主導型のフィットネステストとして、プロジェクトにとって価値ある基盤追加であると判断します。そのまま main にマージすることを推奨します。

---

## 11. 次にやる最小ステップ
1. **GitHub Actions への統合確認**: 新規追加された 4 つの YAML が CI 上で検証対象に含まれているか確認。
2. **Phase 4 完了宣言**: `rhythm_test` のランタイム動作確認（debug overlay 経由）の実施。
