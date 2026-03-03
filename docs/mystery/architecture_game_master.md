# MysteryGameMaster と EvidenceManager の設計思想およびテスト戦略

## 1. 導入の目的
これまで各シーン・UI側（Godot / GDScript）に散らばっていた「犯人は誰か」「証拠品は何を持っているか」「どこを調べたか」といったゲームの進行状態（フラグ・アイテム所持状態）を、C++ (GDExtension) 側の単一の『GameMaster』クラスに集約しました。

これにより、**「ロジック（C++）と演出（Godot）の完全分離」**が実現し、UIのリッチ化やシーン変更作業を行っても、フラグ管理が壊れる（デグレが発生する）リスクが激減します。

## 2. フォルダ構成のベストプラクティス
この基盤をより強固にするため、以下のような明確な責任分界点に基づくディレクトリ構造を推奨します。

```text
godot_cxx_2d_game_template/
├── src/features/mystery/               # [ロジック層・C++] 
│   ├── mystery_game_master.cpp/h       #   - 全体のフラグ進行管理 (脳)
│   ├── evidence_manager.cpp/h          #   - 証拠品・プロファイルの所持状態管理
│   └── (その他のゲーム固有C++ロジック)
│
├── samples/mystery/data/               # [データ層・YAML/JSON]
│   ├── items.yaml                      #   - 証拠品のマスターデータ定義
│   └── scenario/mystery.yaml           #   - シナリオの会話・分岐・フラグ条件データ
│
└── samples/mystery/                    # [演出層・Godot Scene]
    ├── mystery_title.tscn              #   - タイトル画面演出
    ├── office_base.tscn                #   - 事務所の見た目とUI配置
    └── warehouse_base.tscn             #   - 倉庫の見た目とUI配置
```

## 3. デグレを防ぐための「単体テスト（UnitTest）」の考え方
`MysteryGameMaster` や `EvidenceManager` のような純粋なC++クラスは、Godotの画面描画（UI）や物理エンジンから完全に独立しています。そのため、「ボタンを押して確認する」ような手動テストではなく、**プログラムによる自動テスト（Unit Test）**が非常に容易かつ効果的になります。

### A. テストすべき観点
1. **フラグの初期値テスト**: 未設定のフラグを取得した際に正しく `false` が返るか。
2. **状態変化テスト**: `set_flag("has_met_boss", true)` したあと、`get_flag` が確実に `true` を返すか。
3. **重複取得の防止テスト**: `add_evidence("ectoplasm")` を2回呼んだ際、インベントリに2つ追加されず、戻り値が正しくエラー（false）を返すか。
4. **セーブ＆ロードの復元テスト**: `serialize_flags()` で出力した辞書データを `deserialize_flags()` に食わせた際、すべての進行状況が完璧に復元されるか。

### B. 自動化の方法 (C++ / GDScript)
* **C++側の手軽なテスト（Catch2 / GoogleTest など）**:
  UIが存在しないため、純粋なC++のテストフレームワークを導入してCI（GitHub Actions）で毎コミットごとに回すことが理想的です。
* **GDScript側のテスト（GUT など）**:
  Godotエディタ上で `GUT (Godot Unit Test)` などのプラグインを使い、`MysteryGameMaster.set_flag()` などを呼び出してアサーション（検証）する自動スクリプトを書くことで、開発者がエディタ上で一瞬でデグレを検知できます。
