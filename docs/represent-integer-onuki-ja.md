# Onuki 実装に合わせた represent_integer

参照元は [hiroshi-onuki/SQIsign2D-Push](https://github.com/hiroshi-onuki/SQIsign2D-Push) の
commit `19e2df792f1ecbf3ee33209dbdc0c1a34886955f`。
Julia の [`FullRepresentInteger`](https://github.com/hiroshi-onuki/SQIsign2D-Push/blob/19e2df792f1ecbf3ee33209dbdc0c1a34886955f/src/quaternion/klpt.jl)
と [`sum_of_two_squares`](https://github.com/hiroshi-onuki/SQIsign2D-Push/blob/19e2df792f1ecbf3ee33209dbdc0c1a34886955f/src/quaternion/cornacchia.jl) を C に取り込んだ。
参照元の MIT ライセンスと著作権表示は、新しい二つの実装ファイルに記載した。

## 変更点

| 項目 | 変更前の C | 今回の C |
|---|---|---|
| 座標の探索 | 正の座標、ノルム楕円内 | 参照元と同じ符号付き区間（0 を含む） |
| 二平方和 | `ibz_cornacchia_extended`、小素数リスト `{5}` | 2～97 の全小素数を除去し、残りが 1 または適切な素数なら Cornacchia |
| 原始化 | 共通因子を除去し、入力ノルムも縮小し得る | 非原始元を棄却して再探索し、指定ノルムを維持 |
| 入力ノルム | 書き換え可能 | `const ibz_t *`、正の奇数・偶数を受け付ける |
| 整環への変換 | `order_elem_create` 経由 | 参照元の整環基底の座標から直接構成 |

実装は `src/klpt/ref/klptx/represent_integer.c` に分離した。
`equiv.c`、DoublePath、署名処理などの呼び出しは引き続き `represent_integer` を使う。
二平方和の処理は `src/quaternion/ref/generic/sum_of_two_squares.c` に置き、
既存の `ibz_cornacchia_extended` は他の利用箇所のために維持した。

## 探索とノルム

指定ノルムを $N$ とすると、参照元の式をそのまま用いて

$$
m=\left\lfloor\sqrt{\lfloor4N/p\rfloor}\right\rfloor,\quad
z\in[-m,m],\quad
m_d=\left\lfloor\sqrt{\lfloor(4N-z^2)/p\rfloor}\right\rfloor,\quad
w\in[-m_d,m_d]
$$

から標本を選び、$x^2+y^2=4N-p(z^2+w^2)$ を解く。
参照元の $m_d$ は $p z^2$ ではなく $z^2$ を引いている。
このため楕円外の点も選ぶが、残差が非正なら棄却する。
探索分布を寄せるため、この式も維持している。

$x\equiv w\pmod2$、$y\equiv z\pmod2$ を満たす場合、
基底 $(1,i,(i+j)/2,(1+ij)/2)$ における座標を

$$
((x-w)/2,\ (y-z)/2,\ z,\ w)
$$

とする。通常の $1,i,j,ij$ 表示では $(x+yi+zj+w\,ij)/2$ であり、ノルムは正確に $N$。
負の座標の偶奇判定にも対応している。

## 参照元と残る違い

Julia の `FullRepresentInteger` 単体は非原始元も返す。
[`DoublePath`](https://github.com/hiroshi-onuki/SQIsign2D-Push/blob/19e2df792f1ecbf3ee33209dbdc0c1a34886955f/src/double_path/double_path.jl)
と [`RII`](https://github.com/hiroshi-onuki/SQIsign2D-Push/blob/19e2df792f1ecbf3ee33209dbdc0c1a34886955f/src/rii/rii.jl)
の呼び出し側が、整環座標の gcd が 1 になるまで再実行する。
C では既存の「成功時は原始元」という契約を保つため、この再探索を関数内にまとめた。
ただし全候補に既存の試行上限（lvl1: 16384、lvl3・5: 32768）を適用する。
Julia の呼び出し側の無制限の再実行とは、この失敗時の挙動が異なる。
失敗すると 0 を返し、出力元を零元にする。入力ノルムは失敗時も変更しない。

素数の二平方和には C の既存 `ibz_cornacchia_prime` を再利用する。
素数判定の反復回数は C 側の 32 回を維持する。
乱数生成器や平方根の選び方が異なるので、同じシードで Julia と同一の元を返すことは保証しない。

二平方和の関数は完全な因数分解を行わない。
例えば $101^2$ は二平方和で表せるが、小素数除去後も合成数なので参照元同様に失敗する。
一方、$3\bmod4$ の小素数でも指数が偶数なら、その平方因子を取り込める。

## 検証

専用テスト `sqisign_test_represent_integer_lvl1`・`lvl3`・`lvl5` を追加した。

- 二平方和は -1～1000 を独立した整数の全探索と比較。大きな平方因子、残余合成数、失敗時の出力も検証。
- 各レベルで小さなノルム、DoublePath のノルム、大きな奇数ノルムの計 37 ケースについて、ノルムの一致・入力不変・整環への所属・原始性を検証。
- 非正入力と、原始元を得られない小さなノルム 9 の失敗も検証。
- 全レベルで専用テストおよび AddressSanitizer / LeakSanitizer を通過。
- 全レベルで既存の鍵生成・署名・検証を各 100 回通過。既存 quaternion テストも通過。
- 既存 `sqisign_test_klpt_LVL1` の Eichler norm / keygen KLPT / signing KLPT の失敗は、変更直前のソースでも同じ項目で再現した。
- 既存 `sqisign_test_id2iso_lvl1` は変更前・変更後とも 60 秒でタイムアウトしたため、通過は確認できていない。

専用テストの実行例（ビルド済みの場合）：

```sh
ctest --test-dir build -R '^sqisign_test_represent_integer_' --output-on-failure
```

この変更では参照元への仕様・探索方法の整合を確認した。速度の改善率は測定していない。
