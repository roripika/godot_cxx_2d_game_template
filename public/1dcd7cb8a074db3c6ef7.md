---
title: cocos2d-x 3.x Xcodeでソースコードをreferenced(参照)で管理したい時
tags:
  - Xcode
  - cocos2d-x
  - 3.x
  - referenced
private: false
updated_at: '2016-10-23T16:42:50+09:00'
id: 1dcd7cb8a074db3c6ef7
organization_url_name: null
slide: false
ignorePublish: false
---
Windowsとソースコードを共有したい時にはまったので、対処方法をメモ

cocos2d-xのコマンドでxcodeのプロジェクトファイルを作った時を前提に記載します。

###以下の設定を行う。以上！！！

{自分のプロジェクト名}.xcodeproj > Target の　{自分のプロジェクト名}を選択 > Build Settings >　header search paths

``` text:memo
 $(SRCROOT)/../Classes/

```

