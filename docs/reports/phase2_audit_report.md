# Phase 2 Implementation Audit Report

## 1. 全体所見

現在、Phase 2-A/B の実装過程で、**GDExtension の起動クラッシュ（Abort trap: 6）**を回避するための緊急の修正が含まれています。
この修正により、本来変更禁止である `src/core` 配下のライブラリ（`ActionRegistry`）に、内部実装とライフサイクルの変更が加えられています。

本レポートでは、これらの変更を「受け入れ可能（カテゴリA）」「再審査（カテゴリB）」「要修正/非推奨（カテゴリC）」に分類し、安全な PR 分割案を提示します。

---

## 2. カテゴリ別棚卸し

### カテゴリA: 受け入れ候補
**条件**: Core 非変更、Phase 2 の目的に合致、スコープ逸脱なし。

| ファイル名 | 役割 | 変更の必要性 | 影響範囲 |
| :--- | :--- | :--- | :--- |
| `src/games/mystery_test/utils/scenario_validator.cpp` | YAML バリデータ | Phase 2-A の主目的。AI生成物の Fail-Fast 保証。 | 最小（モジュール内） |
| `tests/smoke_test.gd` | 自動検証スクリプト | Phase 2-B の主目的。再デプロイ不要の検証。 | 外部ツール |
| `tests/run_all.sh` | ビルド・テスト統合 | 検証サイクルの自動化。 | 外部ツール |
| `src/games/mystery_test/tasks/*.cpp` | 実証用タスク群 | Kernel v2.0 の基本機能（リエントリ、フラグ等）の Regression 検証。 | 最小（モジュール内） |

### カテゴリB: 要再審査
**条件**: 初期化順序や registry 内部など影響範囲が大きい、またはスコープ超過。

| ファイル名 | 役割 | 懸念点 | 影響 |
| :--- | :--- | :--- | :--- |
| `src/register_types.cpp` | モジュール登録 | シングルトンの初期化を前倒しし、タスク登録を最後方に遅延させている。 | 全体 |
| `src/games/billiards_test/...` | Billiards 最小構成 | Phase 2-C の中身だが、起動確認のため先行導入されている。 | 最小 |

### カテゴリC: 未完成または取り込み非推奨
**条件**: 一時しのぎ、方針不整合、Core 制約違反。

| ファイル名 | 役割 | 問題点 | 判断 |
| :--- | :--- | :--- | :--- |
| `src/core/action_registry.cpp` | タスク登録基盤 | **Core 変更禁止に違反**。メモリ管理をポインタに変更、`initialize()` を外出し。 | **要再設計** |
| `src/games/billiards_test/tasks/*.cpp` | 静的定数修正 | `static godot::String` を `const char*` に変更。GDExtension 固有の回避策。 | 暫定 |

---

## 3. 危険差分の特記事項

### ActionRegistry のライフサイクル変更
- **リスク**: `ActionRegistry` が「インスタンス生成時」ではなく「明示的な `initialize()` 呼び出し」を要求するようになった。これは Core API の破綻を招く恐れがある。
- **背景**: GDExtension の動的ロード時に `godot::String` や `godot::HashMap` が初期化されると、エンジン本体とのリンクタイミングによりクラッシュするため。

---

## 4. PR 分割・取り込み案

現在の差分を以下の順序で分割・適用することを推奨します。

### **Step 1: PR-Stability-Fix (Category C 改修版)**
- **内容**: Core を汚染しない形での GDExtension クラッシュ対策。
- **方針**: `ActionRegistry` の API 変更を minimal に抑え、`register_types.cpp` の順序調整で解決できるか再検証する。

### **Step 2: PR-Guard-Rail (Category A)**
- **内容**: `ScenarioValidator`、`smoke_test` スイート一式。
- **方針**: 問題なくそのままマージ可能。

### **Step 3: PR-Billiards-Skeleton (Category B)**
- **内容**: `billiards_test` 最小モジュール。
- **方針**: Phase 2-C の正式なタスクとしてレビューを行う。

---

## 5. ブロッカー（main 反映前に解決すべき点）

1. **`src/core` の差し戻し検討**:
   - `ActionRegistry` 内部の `std::map` ポインタ化を解除し、標準的な `godot::HashMap` でクラッシュしない構成を模索する。
2. **テストコードの分離**:
   - `register_types.cpp` にハードコードされたテスト用 include の整理。
3. **静的リソースのガイドライン化**:
   - `godot::String` のグローバル静的利用を禁止し、`const char*` 利用を徹底するルール化。
