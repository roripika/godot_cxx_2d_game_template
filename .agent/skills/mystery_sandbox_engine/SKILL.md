---
name: mystery_sandbox_engine
description: Architecture and implementation guidelines for Mystery-Sandbox game development.
---

# 🛠️ Antigravity AgentSkills: "Mystery-Sandbox Engine" 憲法

## 1. アーキテクチャ原則（層の定義）

* **【脳 (Logic Layer)】**: すべての「判定」「フラグ保持」「データ計算」は `src/` 内の C++ (GDExtension) で完結させる。
* **【体 (View Layer)】**: Godotのシーン（.tscn）とGDScriptは、「入力の検知」と「アニメーション/音の再生」のみを担当する。
* **【架け橋 (Bridge)】**: シングルトン（AutoLoad）のC++クラス `MysteryManager` を唯一の通信窓口とし、GDScriptから直接フラグ変数を書き換えることを禁止する。

## 2. デグレ防止とトレーサビリティ（追跡可能性）

* **型安全の徹底**: 新しいフラグや証拠品を追加する際は、必ず `enums.hpp`（または相当するヘッダー）に `enum class` を追加し、文字列での指定を避ける。
* **自動ロギング**: 状態（フラグ等）を変更する関数には、必ず以下の情報を `UtilityFunctions::print` で出力するコードを含めること。
  - `[CHANGE] Flag: {Name} | Old: {Value} | New: {Value} | Caller: {Function}`
* **スナップショット**: `MysteryManager` は現在の全状態をシリアライズ（Dictionary化）できるメソッドを常に保持し、いつでも「その瞬間の再現」ができるようにする。

## 3. 実装の進め方ルール

* **思考の明文化**: 実装を開始する前に、必ず「どのクラスを修正し、どのフラグに影響が出るか」を1行で報告してから着手すること。
* **高凝集・低結合**: 「1つの機能（例：ドアの解錠）」に関するロジックが複数のファイルに散らばらないよう、C++の1つのクラス内で完結させる設計を優先する。職場のクリーンアーキテクチャのような「過剰なレイヤー分割」は禁止する。
