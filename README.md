# CaslII

基本情報技術者試験で出題されるアセンブラ言語（ＣＡＳＬII）のシミュレータです。</br>
ターミナルで実行します。

実行方法
```shell
$ casl ソースパス1 [ソースパス2] ...
```
ソースパスにCASLIIのソースフィルを指定してください。

実行すると、デバッグコマンド入力待ち画面になります。

```text
Casl Debugger 1.0
Debugger Starting...
Memory Word Size: 4096
Used Word Size: 37

EC = 0
PR = 0000, SR = 1000, OF = 0, ZF = 0, SF = 0
GR0 = 0000, GR1 = 0000, GR2 = 0000, GR3 = 0000
GR4 = 0000, GR5 = 0000, GR6 = 0000, GR7 = 0000
$
```

入力画面からデバッグコマンドを入力し、実行結果を確認することができます。


デバッグコマンド
-

概要 | コマンド | 備考
---- | ---- | -----
全レジスタ表示 | R | 全レジスタ表示で表示される`EC`は最初から実行されたステップ数を表示します
ソースリスト表示 | L [start offset] [end offset] | `start offset`、`end offset`を指定しない場合はPRレジスタが指す位置から最後まで表示します
シングルステップ | S 
現在状態からの実行 | C
ブレークポイント設定 | BP [offset1] [offset2] ... | `offset`を指定しない場合は先頭から最後までのソースリストを表示します
全ブレークポイントまたは、指定ブレークポイントクリア | BC * \| [offset1] [offset2] ...
レジスタをリセットして先頭から実行 | GO
レジスタをリセット | RESET
GR0表示 | GR0
GR1表示 | GR1
GR2表示 | GR2
GR3表示 | GR3
GR4表示 | GR4
GR5表示 | GR5
GR6表示 | GR6
GR7表示 | GR7
ヘルプ表示 | H
終了 | Q

その他、全般
-

* offsetは10進数、16進数またはLABELで指定することができます。
<br/> BP 123 <br/> BC #12AB  <br/> L LABEL
* コマンドは大文字小文字の区別をしません。
<br/> go
<br/>

実行モジュール
-


## Windows版

binフォルダ下にある`casl-win.exe`をWindowsの適当なフォルダに置いて、コマンドプロンプトより実行してください。<br/>
文字コードはUTF-8を使っています。<br/>
表示が文字化けしないように、caslコマンドを実行するときに、内部でコードページを`65001`に変更しています。


## Linux版

ソース環境を取得し、ビルドしてください。
cmakeツールを使っています。また、テスト環境としてgoogle testを使っています。<br/>
テスト環境が必要ない場合は、`CMakeList.txt`の下記の部分をコメントアウトしてください。
```text
if(NOT WIN32)
    add_subdirectory(test)
endif()
```

### Linux版ビルド方法
リポジトリをクーロン後、以下のコマンドを実行してください。<br/>`CaslII`はクーロン先のフォルダ名です。
```bash
$ cd CaslII
$ mkdir build
$ cd build
$ cmake ../
$ make
```
make実行後、実行モジュールの`casl`が`build`フォルダ下に作成されますのでそれを使用してください。





