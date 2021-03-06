= オーバーロード解決

@<b>{テンプレートよりよく使うけど、テンプレートよりよくわからない！}

C++には同じ名前の関数を違う型の引数で実装できるオーバーロードという機能があります。
「オーバーロード解決」は関数の呼び出し式が与えられたときにどの実装を呼び出すのかを選択するプロセスの名称です。

//emlist[][cpp]{
void func(int);     // #1
void func(double);  // #2

int main() {
    func(1);    // calls #1
    func(0.0);  // calls #2
}
//}

この付録ではオーバーロードの詳細を解説します。

== オーバーロード解決順位

オーバーロード解決の優先付けはPartial Orderという順序で順序付けされます。
Partial Orderは@<code>{a < b}も@<code>{a > b}も@<code>{false}であっても、
@<code>{a == b}が@<code>{true}ではない、つまり比較不能という状態が存在します。

ある関数のオーバーロードの候補（overload candidates）が順序比較不能であるとき、
オーバーロード解決は曖昧となり、コンパイルに失敗します。

//emlist[][cpp]{
void func(int, double); // #1
void func(long, int);   // #2

int main() {
    func(1, 1); // ambiguous
}
//}

この例では@<b>{#1}は第一引数、@<b>{#2}は第二引数がintで完全にマッチします。
しかし、@<b>{#1}は第二引数の呼び出しがintに対してdouble、
@<b>{#2}は第一引数の呼び出しがintに対してlongとなっています。
結果、コンパイラはどちらを呼べばよいのかを判断できず、オーバーロード解決は曖昧となります。

読者の中にはdoubleよりlongのほうがintに近いのではないかと思われる方がいらっしゃるかもしれないですが、
C++はオーバーロード距離のようなものを考慮するようにはなっていないため、完全に優先順位がつかない場合はすべて曖昧となります。

さて、優先順位という言葉を定義せずに使ってしまっていました。
オーバーロードの優先順位はどのように設定されているのでしょう？
ここでは、簡略化された優先順位を紹介します。
この順位にテンプレートは含まれてません（あとで説明します）。

 * 1. 完全なマッチ: すべての実引数の型が修飾子を含めて完全に一致する場合。
 * 2. 微調整によるマッチ: 1.に加えて非constからconstへの変換、配列から配列の先頭要素へのポインタへの変換などを許可してマッチする場合。
 * 3. プロモーションによるマッチ: 2.に加えてintからlong、floatからdoubleなど、安全かつ暗黙に変換を許可してマッチする場合。
 * 4. 標準変換のみによるマッチ: 3.に加えてintからfloatやpublicな基底クラスへの変換を許可してマッチする場合。暗黙の変換演算子による変換や呼び出し可能なコンストラクタによる変換を含まない。
 * 5. ユーザー定義変換によるマッチ: 4.に加えて暗黙の変換演算子による変換やコンストラクタによる変換など、すべての変換を許可してマッチする場合。
 * 6. ellipsis (...): ほぼすべての型は省略記号にマッチする（ただし、非トリビアルなコピーコンストラクタを持つクラスはマッチしない）。

かなり簡略化されていますが、この順番で考えられない場合に出会うことはそうそうないでしょう (initializer_listが例外だが、ここでは割愛します)。

以下にいくつかの小さな例を示します:

//emlist[][cpp]{
int f1(int);    // #1
int f1(double); // #2

f1(4); // calls #1
// #1: perfect match
// #2: needs a standard conversion
//}

説明もいらないくらい簡単な例です。
パーフェクトマッチが優先されます。

//emlist[][cpp]{
int f2(int);    // #3
int f2(char);   // #4

f2(true); // calls #3
// #3: matches with promotion
// #4: requires stronger promotion
//}

整数のプロモーションにはその中でも優先順位があります。

//emlist[][cpp]{
struct X {
    X(int);
};

int f3(X);      // #5
int f3(...);    // #6

f3(1); // # calls #5
// #5: matches with user-defined conversion
// #6: requires a match with ellipsis
//}

== メンバ関数の暗黙の引数

メンバ関数は@<code>{*this}という暗黙の引数を第一引数に持つ（「暗黙の引数」というのはここでしか出てこないであろう造語です）。
@<code>{MyClass}のメンバ関数が持つ暗黙の第一引数の型は非constメンバ関数の場合@<code>{MyClass&}であり、
constメンバ関数の場合@<code>{MyClass const&}です。
C++11からは右辺値のバージョンが追加された。
右辺値の@<code>{MyClass}のメンバ関数が持つ暗黙の第一引数の型は非constメンバ関数の場合@<code>{MyClass&&}であり、
constメンバ関数の場合@<code>{MyClass const&&}である（const版を使うことはないだろうが）。

右辺値参照を引数に取ると、左辺値実引数にはマッチしなくなり、右辺値にのみマッチするようにります。

//emlist[][cpp]{
struct S {};

void func(S&);  // #1
void func(S&&); // #2

int main() {
    S s{};
    void func(s);    // calls #1
    void func(S{});  // calls #2
}
//}

メンバ関数でも同じことができます。
メンバ関数ではつぎのように@<code>{()}につづけて修飾子を書きます。
この修飾子がメンバ関数の暗黙の引数に対するオーバーロードの文法です。

//emlist[][cpp]{
struct watch_t {
    void tick() &;      // #1
    void tick() const&; // #2
    void tick() &&;     // #3
    void tack() const&; // #4
};
//}

これを呼び出すとつぎのようにオーバーロードが選択されます。

//emlist[][cpp]{
struct watch_t {
    void tick() &;      // #1
    void tick() const&; // #2
    void tick() &&;     // #3
    void tack() const&; // #4
};

int main() {
    watch_t watch{};
    const watch_t const_watch{};
    watch.tick();       // calls #1
    const_watch.tick(); // calls #2
    watch_t{}.tick();   // calls #3
    // calls #4, because const lvalue ref accepts rvalue
    watch_t{}.tack(); 
}
//}

== テンプレート VS 特殊化 VS 非テンプレート

//emlist[][cpp]{
template <class T> void f(T); // #1
void f(int); // #2
template <> void f<double>(double); // #3

int main() {
    f(1);       // calls #2
    f(1.0);     // calls #3
    f("hoge");  // calls #1
}
//}

基本的にテンプレートとテンプレートではないものがオーバーロード候補にあった場合、非テンプレートが優先されるとよく説明されます。
また、明示的特殊化されたテンプレートがあった場合、特殊化されてないテンプレートよりも優先順位が高くなります。
第4章でも説明したとおり、これには落とし穴があります。
非テンプレートがテンプレートより優先されるのは、非テンプレートがパーフェクトマッチした場合のみです。
ジェネリックな関数テンプレートを押しのけて、より特殊化されたテンプレートにマッチするためには修飾子まで含めてマッチさせる必要があります。

//emlist[][cpp]{
template<typename T> void f(T&&);

// vectorだけはこっちを呼びたいという固い意思によって書かれたオーバーロード群
template<typename T> void f(std::vector<T>&);
template<typename T> void f(const std::vector<T>&);
template<typename T> void f(std::vector<T>&&);
template<typename T> void f(const std::vector<T>&&);
//}

== initializer_list

C++11から、@<code>{std::initializer_list}が追加されました。

//emlist[][cpp]{
#include <initializer_list>

void func(std::initializer_list<int>);

int main() {
    func({1, 2, 3});
}
//}

@<code>{std::initializer_list}はbraced-init-list内の型変換が最小になるようにオーバーロード解決を行います。

//emlist[][cpp]{
#include <initializer_list>

void func(std::initializer_list<int>); // #1
void func(std::initializer_list<char>); // #2

int main() {
    func({'a','a','a'});    // calls #2
    func({'a','a','a', 1}); // calls #1
}
//}

@<code>{func({'a','a','a'})}は@<code>{std::initializer_list<char>}に完全マッチします。
@<code>{func({'a','a','a', 1}))}は型変換が必要です。
@<code>{int -> char}は標準変換が必要ですが、@<code>{char -> int}はプロモーションで可能ですのでこちらが最小の型変換です。
よって、@<code>{std::initializer_list<int>}にオーバーロードが解決します。

つぎのようなオーバーロードは、
@<code>{int -> double}、
@<code>{double -> int}は共に標準変換であるためオーバーロードが曖昧となり、解決しません。


//emlist[][cpp]{
#include <initializer_list>

void func(std::initializer_list<int>);  // #1
void func(std::initializer_list<double>); // #2

int main() {
    func({1,2,1.0}); // ambiguous
}
//}


つぎのコードを実行すると何を出力するでしょうか？

//emlist[][cpp]{
#include <string>
#include <iostream>

int main() {
    std::cout << std::string(33, 'a') << std::endl;
    std::cout << std::string{33, 'a'} << std::endl; 
}
//}


1つめは@<b>{aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa}を出力します。
2つめはそうではありません、@<b>{33}は文字として解釈されます（ASCIIを前提にすれば出力は@<b>{!a}になります）。
これは、波括弧で初期化した場合@<code>{std::initializer_list}を引数にもつコンストラクタが優先的に呼ばれるという仕様によります。
@<code>{std::string}は文字のリストを@<code>{std::initializer_list<char>}で受け取るコンストラクタを持っており、それが優先的に呼ばれたというわけです。

== テンプレートを加えたオーバーロード順位

 * 1. 完全なマッチ
 * 2. 明示的に特殊化されたテンプレートへの完全なマッチ
 * 3. より特殊化されたテンプレートへの完全なマッチ
 * 4. テンプレートの実体化による完全なマッチ
 * 5. 微調整によるマッチ
 * 6. プロモーションによるマッチ
 * 7. 標準変換のみによるマッチ
 * 8. ユーザー定義変換によるマッチ
 * 9. ellipsis (...)

もうおわかりかと思いますが、
テンプレートは完全にマッチする関数をインライン展開できるため、
ジェネリックすぎる関数テンプレートはホイホイ書いてはいけません。

ジェネリックな関数テンプレートと非テンプレートのオーバーロードを共存させようなんて絶対に考えてはいけません、地獄になるだけです。
