import os
import re
import datetime

def convert_to_blog_style(report1_path, report2_path, github_link):
    # 今回はユーザーの強い要望に基づき、レポートのデータを使いつつ、
    # 開発の実体験（壁打ち、トークン制限、エージェント分散など）を大幅に盛り込んだ構成にします。
    
    blog_content = f"""---
title: "Godot 4 + GDExtension(C++)で作る！2Dアドベンチャーゲーム開発記 (AI共作メトリクス)"
tags:
  - Godot
  - Cpp
  - GDExtension
  - AI
  - ペアプログラミング
private: true
updated_at: '{datetime.datetime.now().isoformat()}'
id: null
organization_url_name: null
slide: false
---

# 激闘の4,600ステップ：AIエージェントと完走したGodot 4 + C++ ADV開発の全記録

## はじめに：なぜGodotとAIなのか
「GodotとAIの親和性が高い」という噂を耳にし、その真偽を確かめるべく、私はこのプロジェクトをスタートさせました。
目的は、Godot 4のGDExtension (C++) をベースにした2DアドベンチャーゲームSDK「Karakuri SDK」の構築。
結果から言えば、**4,682ステップ**という膨大な対話を通じて、SDKの完成まで漕ぎ着けることができました。

しかし、巷で言われるような「ワンプロンプトで魔法のようにゲームが完成する」といった夢のような話ではありませんでした。
そこにあったのは、AIとの泥臭い「壁打ち」の連続でした。

---

## 🚀 1. 幻想と現実：「ワンプロンプト」は存在しない
AIを使えば、一言でゲームができると思われがちですが、現実は過酷です。
新規の仕組みを作る際、AIは何度も的外れな提案をしたり、既存の構造を壊したりします。

- **「壁打ち」の重要性**: 仕様を一つずつ確認し、実装してはエラーを出し、ログを読ませて修正する。この反復（壁打ち）こそが開発の本体でした。
- **構造化の転換**: 途中で「このやり方ではスケーリングしない」と判断し、ワークフローやデータ構造をドラスティックに変更することもありました。AIに「今の構造は捨てて、C++側に責務を寄せよう」と提案し、再構築を促す判断力は依然として人間側に求められます。

---

## 🎨 2. ツールセットの妙：Antigravity + Nanobanana
今回、AIエージェントとして中心的に活動したのが **Antigravity** です。
特筆すべきは、**Nanobanana** との連携による素材作成。
- **自律的なアセット生成**: Antigravityがシナリオに必要な素材（背景、立ち絵）を特定し、Nanobananaを通じて生成。これを自動的にプロジェクトのディレクトリに配置し、`.tscn`から参照させる。
- この流れが確立されてからは、開発スピードが飛躍的に向上しました。

---

## 📐 3. UI開発の地獄：明確な基準の不在
最もAIが苦戦し、開発が停滞したのは「UIの配置」でした。
「いい感じに配置して」という指示では、AIは全く先に進めません。

- **配置基準の重要性**: アンカー（Anchors）やプリセットをどう使うか、デザインの基準を明確に伝えないと、UIが重なったり画面外に消えたりします。
- **UI方針の確立**: 「UI被り」を防ぐために、レイヤー構造やアンカーの共通ルールを定義。これをAIに徹底させるまでが最大のハードルでした。逆にいえば、基準さえ決まればAIの正確性は凄まじいものがありました。

---

## 🔋 4. メタ戦略：トークン制限とエージェントの分散
開発を進める上で避けられないのが、AIモデルの「利用制限」です。
安価なアカウントで開発を続けるためには、リソースの管理も「開発の一部」でした。

- **モデルの使い分け**: 
    - 高度な設計が必要な時は **Gemini 3.1 Pro**。
    - 制限に引っかかりそうになったり、定型的な修正の時は **Gemini 3 Flash** へスイッチ。
    - こうして「節約」をしながら、ピーク時の出力を維持しました。
- **エージェントの負荷分散**: 
    - **Antigravity** に全体設計を任せ、
    - **Codex** にリファクタリングやバグ修正を振る。
    - **Copilot** には基盤コードのボイラープレートを書かせる。
    - 単一のAIに頼らず、複数の「脳」にトークンを分散させることで、クォータ制限を回避しつつ24時間体制の開発を維持しました。

---

## 💡 最後に：4,600ステップが見せた景色
AIとのペアプロは、魔法ではありません。「高度な思考の自動化」と「泥臭い管理」のハイブリッドです。
それでも、このスピード感でC++のSDKを組み上げられたのは、AIというパートナーがいたからに他なりません。

単なるレポートではなく、この「戦略的な苦労」が、AI開発の最前線に挑む皆さんの参考になれば幸いです。

---

## 🔗 Repository & Metrics
- **GitHub Branch**: [{github_link}]({github_link})
- **総LOC**: C++ +3,300, GDScript +2,100
- **自律実行回数**: 80回以上

これからも、最適なモデル選択と複数のエージェントを駆使し、新たなゲーム体験を構築していきます！
"""
    return blog_content

if __name__ == "__main__":
    report1_path = "/Users/ooharayukio/godot_cxx_2d_game_template/qiita_development_report.md"
    report2_path = "/Users/ooharayukio/godot_cxx_2d_game_template/qiita_development_report_v2_metrics.md"
    github_link = "https://github.com/roripika/godot_cxx_2d_game_template/tree/main"
    
    blog_md = convert_to_blog_style(report1_path, report2_path, github_link)
    
    article_path = "/Users/ooharayukio/godot_cxx_2d_game_template/public/blog_post.md"
    os.makedirs(os.path.dirname(article_path), exist_ok=True)
    with open(article_path, 'w', encoding='utf-8') as f:
        f.write(blog_md)
    print(f"Final deep-dive blog post generated at: {article_path}")
