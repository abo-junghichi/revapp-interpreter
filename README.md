# ラムダ計算プログラミング言語 "revapp"の、最低限のインタプリタ実装
式の結合法則を右結合に揃えたラムダ式に基づき、評価戦略に必要呼びを用いることで、簡潔さを図ったプログラミング言語。
そしてその言語で書かれたソースコードを、中間表現に変換することなくそのままメモリ上にコピーして解釈実行する、言語処理系。

## 動作テスト
GNU C コンパイラがセットアップ済みのUnix環境での場合

	$ make all

でインタプリタのバイナリrevappi.outが生成されます。

	$ ./revappi.out example/fizzbuzz.revapp

でFizzBuzz問題の回答が延々と標準出力に書き出され続けます。
端末からCtrl+Cを入力するなどして強制終了して下さい。
シェルスクリプトにも使えるよう、このインタプリタは下記の文法に加えてshebang行も認識します。

	$ ./example/cat.revapp < Makefile

## 文法
revappの文法をBNF記法で表すと以下の様になります。

	<expr> ::= "="<identifier> | <identifier> | <expr> <expr>

ただし、<identifier>は文字数が0以上の文字列とします。 
結合順位を明示するために括弧を使うことが出来ます。カッコがない場合、全ての構文は右結合優先の原則に従います。例えば"a (=b c)"では、"=b c"を囲むカッコが不要で"a =b c"と書いても同じ意味になります。括弧まで含めた文法は

	1. <term> ::= <identifier> | "="<identifier> |"(" <expr> <expr> ")"
	2. <expr> ::= "(" <expr> ")" <expr> | <term>

となります。

## "revapp"の理由
或いは、「ラムダ式」の何処が間違っていたのかについて。

ラムダ式の表現する計算体系・ラムダ計算は、チューリング完全な最小の計算体系です。一つの変換規則と一つの関数定義規則しか持たず、それでいて、制御構造の表現からデータ領域確保まで、プログラミングに十分な機能を提供しています。よってラムダ計算を素直に表現すれば、チューリング完全な最小のプログラミング言語が生まれるはずでした。しかし代わりに、ラムダ計算の生みの親であるアロンゾ・チャーチとスティーヴン・コール・クリーネは、それ以前の数学体系の表記法を使ってラムダ計算を表現しました。こうして生まれたのが「ラムダ式」です。

二つの引数x,yを取る関数f(x,y)=x+yを考えてみましょう。チャーチ達が作ったラムダ式では、先の式の左辺に現れた変数の順番を踏襲して「f=λxy.x+y」と表現します。また、関数適用の順番も、数学の表記に近い「f 3 2 = f(3,2)」となるように左結合に定め、「f 3 2」と「(f 3) 2」は同じ式になるように表現します。ということは、「(λxy.x+y) 3」の様に、引数を二つ取る関数に一つずつ値を適用する場合、λ記号の直後の変数から代入が行われる事になります。従って、「λxy.x+y」は「λx.(λy.x+y)」と同値となり、変数への代入は右結合となります。結果、関数適用とラムダ抽象では結合の順番が逆になっています。また、「λx.g x 3」と「(λx.g x) 3」の様に、ラムダ抽象に束縛されている変数と、ラムダ抽象の外側の引数を区別するため、括弧を多用しなければなりません。そして何より、「f=λxy.x+y」の様に、関数fを定義するのに等号「=」が必要となります。

この様に冗長な表記法をチャーチ達が選ばざるを得なかったのは、生みの親の宿命です。ラムダ計算発見当初は、チャーチ達以外の人々はラムダ計算を知らないからです。ラムダ計算の素直な表現は、ラムダ計算を知らない人に取っては暗号文に過ぎません。しかし、チャーチ達の偉大な発見が常識となった今、当時の冗長な表記法に拘る必要はありません。

その名が示すようにrevappでは、関数適用の順番を逆にして、関数適用もラムダ抽象も右結合で統一しています。

このため、左から右へソースコードの語順通りに読み進める、簡潔な方法での解釈実行が可能になります。多くの言語処理系のインタプリタ実装では、ソースコードを解析して抽象構文木などの中間表現に変換してから、その中間表現を対象に逐次実行する方法が一般的です。勿論、変数などのアクセスの度に文字列比較をするマシンサイクルの浪費を省くなど、高速化が理由の一つではあるでしょう。しかしもう一つの理由は単に、ソースコードそのままでの逐次実行が絶望的になるほどの、複雑な文法のため、と私は疑っています。当インタプリタ実装では、ソースコードをそのままメモリに格納し、そのメモリイメージを対象に逐次実行する方法を採っています。

表記も簡潔になります。「3 λx g x」の様にラムダ抽象の束縛部分と引数部分がλ記号で分断され、括弧の使用を節約できます。また、変数を示すラムダ記号と、その変数に適用される引数を隣接させる事ができ、ラムダ記号を等号「=」の様に用いることが出来ます。revappでは、ラムダ記号ではなく等号でラムダ抽象を表現します。
