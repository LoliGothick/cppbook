= 無駄なコードを減らす

コードは書けば書くほどバグが混入します。
無駄なコードの重複や複雑性を回避し、シンプルで美しいコードを維持することが設計の成功の鍵です。

== コピペ乱舞

無駄なコードを減らすための最初の方針は共通の処理を関数として共通化することです。
悲しいかな、コピペによって増殖してしまったコードはたくさんあります。
そのようなコードは一部を変更すると他の変更が漏れ、容易にバグの原因となります。

そもそも、自分でコードを書かずに信頼できるライブラリを使うのが最適です。
この章では、C++の標準ライブラリのカスタマイゼーションポイントの活用方法を紹介します。

== カスタマイゼーションポイント

多くの言語では文字列化やオブジェクトの比較など、よくある操作をカスタマイズする方法は確立されています。
カスタマイズを覚えておくことで共通の資源でコードを書くことが可能になります。

=== Stringifying

C++20から入る文字列フォーマットを待ちましょう（本音）。

実際問題、C++17までは以下のように@<code>{std::ostream}に出力する@<code>{operator<<}をカスタマイゼーションポイントに使うことが多いです。
@<code>{std::stringstream}に出力して@<code>{str()}で文字列を取り出します。

//emlist[][cpp]{
class Widget {
    int a, b;
public:
    friend std::ostream&
    operator<<(std::ostream& os, const Widget& w) const {
        return os << a << " " << b;
    }
};
//}

=== 比較

C++20から入る三方比較演算子 @<code>{operator<=>}を待ちましょう（本音）。C++17では、比較演算CPOは何種類かあります。

@<b>{方法1: 演算子のオーバーロード}

@<code>{==, !=, <, >, <=, >=}の6種類の演算子を適切にオーバーロードしましょう。

@<b>{方法２: CPO（カスタマイゼーションポイントオブジェクト）}

@<code>{std::map}や@<code>{std::set}は、第3テンプレート引数がカスタマイゼーションポイントオブジェクトになっています。
std::lessを特殊化するか、自分で作ったクラスを渡してあげることで@<code>{std::map}のキー比較をカスタマイズできます。

//emlist[std::mapのを降順にカスタマイズ][cpp]{
#include <map>
#include <functional>

int main() {
    std::map<std::string, int, std::greater<>> dict{};
}
//}

このように、カスタマイゼーションポイントをクラスから分離しています。
何も書かなければデフォルト実装が選択されます。
ユーザーが明示的にクラスを指定することで実装の詳細をユーザーが静的にインジェクションすることができるのです。
このカスタマイゼーションポイントの設計はポリシーと呼ばれています（次章で解説します）。

比較を伴う関数は比較のための関数オブジェクトを受け取るようになっています。
std::sort,std::min,std::maxの第3引数などがそれです。何も指定しなければ演算子が使われます。
関数ではよくあるカスタマイゼーションポイントのもたせ方です。

//emlist[std::sortのカスタマイズ][cpp]{
#include <algorithm>
#include <vector>

int main() {
    std::vector<std::pair<int, int>> vec{ {1 ,2}, {2, 3}, {3, 4} };
    std::sort(vec.begin(), vec.end(), [](auto&& p1, auto&& p2){
        return p1.second < p2.second;
    });
}
//}

== この章のまとめ

 * コードをしっかり共通化する
 * そもそも自分でコードを書かずにライブラリを探す
 * 標準ライブラリのカスタマイゼーションポイントを利用する
