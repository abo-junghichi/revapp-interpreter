# ラムダ計算プログラミング言語 "revapp"の、最低限のインタプリタ実装
式の結合法則を右結合に揃えたラムダ計算の表記法[De Bruijn notation](https://en.wikipedia.org/wiki/De_Bruijn_notation)に基づき、評価戦略に必要呼びを用いることで、簡潔さを図ったプログラミング言語。
そしてその言語で書かれたソースコードを、中間表現に変換することなくそのままメモリ上にコピーして解釈実行する、最低限の言語処理系。

## 動作テスト
GNU C コンパイラがセットアップ済みのUnix環境での場合

	$ make all

でインタプリタのバイナリrevappi.outが生成されます。

	$ ./revappi.out example/fizzbuzz.revapp

でFizzBuzz問題の回答が延々と標準出力に書き出され続けます。
端末からCtrl+Cを入力するなどして強制終了して下さい。
シェルスクリプトにも使えるよう、このインタプリタは下記の文法に加えてshebang行も認識します。

	$ ./example/cat.revapp < Makefile

### 静的リンクのLinux-i386用極小バイナリ
バイナリ構築の前に、
組み込みrevappソースコードをC言語処理系から扱える形に変換します。

	$ make romsrc.c

nostdlibディレクトリでバイナリを構築します。

	$ cd nostdlib
	$ sh build.sh i386
	$ cd ..
	$ du -b nostdlib/revappi.out.trunc
	8865 nostdlib/revappi.out.trunc

使い方は通常版と同じです。

	$ ln -s nostdlib/revappi.out.trunc revappi.out
	$ ./revappi.out example/fizzbuzz100.revapp
	$ ./example/cat.revapp < Makefile

組み込みrevappソースコードを削除することで、バイナリサイズを減らせます。

	$ make img2c.out
	$ printf "" | ./img2c.out > romsrc.c
	$ cd nostdlib
	$ sh build.sh i386
	$ cd ..
	$ do -b nostdlib/revappi.out.trunc
	4025 nostdlib/revappi.out.trunc

使うときは、組み込みrevappソースコードを先に読み込ませる必要があります。

	$ make romsrc/romsrc.revapp
	$ cat romsrc/romsrc.revapp example/fizzbuzz100.revapp | ./nostdlib/revappi.out.trunc /dev/stdin

## 文法
revappの文法をBNF記法で表すと、De Bruijn notationとよく似たものになります。

	<expr> ::=
	  "=<0文字以上のidentifier>" <expr>
	| "(" <expr> ")" <expr>
	| "<1文字以上のidentifier>" <expr>
	| 空文字列

すなわち、`<expr>`は必ず空文字列で終了する、と構文解析されます。
このようにDe Bruijn notationを解釈することで、
`<expr>`の終了処理を空文字列のみに集中でき、インタプリタ実装を単純化できます。
副産物として、中身のない括弧のペア「()」は恒等関数を表します。

また、空文字列の変数参照は表記できないので、
`<identifier>`が空文字列のラムダ抽象「=」により束縛された引数は、
そのラムダ抽象が有効な名前空間からは参照されません。

## "revapp"の理由
或いは、「ラムダ式」の何処が間違っていたのかについて。

ラムダ式の表現する計算体系・ラムダ計算は、チューリング完全な最小の計算体系です。一つの変換規則と一つの関数定義規則しか持たず、それでいて、制御構造の表現からデータ領域確保まで、プログラミングに十分な機能を提供しています。よってラムダ計算を素直に表現すれば、チューリング完全な最小のプログラミング言語が生まれるはずでした。しかし代わりに、ラムダ計算の生みの親であるアロンゾ・チャーチとスティーヴン・コール・クリーネは、それ以前の数学体系の表記法を使ってラムダ計算を表現しました。こうして生まれたのが「ラムダ式」です。

二つの引数x,yを取る関数f(x,y)=x+yを考えてみましょう。チャーチ達が作ったラムダ式では、先の式の左辺に現れた変数の順番を踏襲して「f=λxy.x+y」と表現します。また、関数適用の順番も、数学の表記に近い「f 3 2 = f(3,2)」となるように左結合に定め、「f 3 2」と「(f 3) 2」は同じ式になるように表現します。ということは、「(λxy.x+y) 3」の様に、引数を二つ取る関数に一つずつ値を適用する場合、λ記号の直後の変数から代入が行われる事になります。従って、「λxy.x+y」は「λx.(λy.x+y)」と同値となり、変数への代入は右結合となります。結果、関数適用とラムダ抽象では結合の順番が逆になっています。また、「λx.g x 3」と「(λx.g x) 3」の様に、ラムダ抽象に束縛されている変数と、ラムダ抽象の外側の引数を区別するため、括弧を多用しなければなりません。そして何より、「f=λxy.x+y」の様に、関数fを定義するのに等号「=」が必要となります。

この様に冗長な表記法をチャーチ達が選ばざるを得なかったのは、生みの親の宿命です。ラムダ計算発見当初は、チャーチ達以外の人々はラムダ計算を知らないからです。ラムダ計算の素直な表現は、ラムダ計算を知らない人に取っては暗号文に過ぎません。しかし、チャーチ達の偉大な発見が常識となった今、当時の冗長な表記法に拘る必要はありません。

その名が示すようにrevapp(reverse apply)では、関数適用の順番を逆にして、関数適用もラムダ抽象も右結合で統一しています。

このため、左から右へソースコードの語順通りに読み進める、簡潔な方法での解釈実行が可能になります。多くの言語処理系のインタプリタ実装では、ソースコードを解析して抽象構文木などの中間表現に変換してから、その中間表現を対象に逐次実行する方法が一般的です。勿論、変数などのアクセスの度に文字列比較をするマシンサイクルの浪費を省くなど、高速化が理由の一つではあるでしょう。しかしもう一つの理由は単に、ソースコードそのままでの逐次実行が絶望的になるほどの、複雑な文法のため、と私は疑っています。当インタプリタ実装では、ソースコードをそのままメモリに格納し、そのメモリイメージを対象に逐次実行する方法を採っています。

表記も簡潔になります。「3 λx g x」の様にラムダ抽象の束縛部分と引数部分がλ記号で分断され、括弧の使用を節約できます。また、変数を示すラムダ記号と、その変数に適用される引数を隣接させる事ができ、ラムダ記号を等号「=」の様に用いることが出来ます。revappでは、ラムダ記号ではなく等号でラムダ抽象を表現します。

## 最低限の処理系
当処理系では、二つの意味での最低限を目指しました。

ひとつは、言語の実行効率を上げる、最適化の強度です。

[lazy K](https://tromp.github.io/cl/lazy-k.html)の著者が述べているように、純粋な関数型言語では副作用の概念がありません。正確には、隠蔽されて最適化の対象となっており、副作用を伴わない演算処理と同様に、メモ化などにより省かれます。例えば、ウェブページをキャッシュするプロクシサーバであれば、サーバプログラムのコンパイル時に世界中のウェブページを読み込むことで、実行時には全くウェブアクセスをしないものが生成されるかもしれません。そして、コルモゴロフ複雑性を計算しようとするデータ圧縮プログラムと同様に、演算量を圧縮しようとする最適化プログラムが目指す最小演算量は、将来に渡って計算不可能でしょう。しかし最大演算量については、最適化をしなければいいので計算可能です。特に副作用演算については、プログラムの動作に直結します。データ圧縮プログラムであれば、「無圧縮」の意味は自明ですが。プログラミング言語処理系での「無最適化」ならば言語仕様を明示しなければなりません。必要呼びを評価戦略とするラムダ計算では、演算量の最大値はどのようなものか? プログラミング言語revappの言語仕様を確立するために、この問いへの答えが私には必要でした。

そして、言語の実行効率の最適化をしないということは、言語の実行の他には必須の機能がない言語処理系に於いては、処理系の実装をできるだけ単純にすることと同義であり。これが、当処理系の目指すもう一つの最低限です。

当処理系はC言語で実装していますが、ライブラリを殆ど用いていません。このため、僅かな書き換えだけで、静的リンク形式なのにファイルサイズが10キロバイトに満たないLinux-i386用実行ファイルをコンパイルできます。また、C言語の機能である、関数の再帰呼び出しも使っていないので、組み込みOSなどスタックサイズの制限された環境でもスタック・オーバーフローの心配がありません。
