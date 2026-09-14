SQIsign2DPush と SQIsign v2.0 ref の Fp 実装比較（2026-09-08）

追記: [v2.0 型の内部表現へ移行した実装・幅の選定理由・実測結果](fp-v2-backend-ja.md)を追加した。以下は旧バックエンドの調査記録。

固定指数の計算手順の事前生成を実装した。採用した手順・再生成方法・変更前後の測定値は[固定指数実装の結果](fp-fixed-exponent-results.md)を参照。

追記（調査後の修正）: lvl3/lvl5 の `fp_decode` のマスク幅を修正し、不正入力を Montgomery 変換前にゼロ化した。lvl1 も encode/decode に Montgomery 変換を追加し、成功時は `UINT32_MAX`、失敗時は戻り値・出力ともゼロとなる仕様に統一した。以下の調査時点の不具合記述・実測 JSON は修正前の記録である。`fp_decode_reduce` の一般入力に対する還元範囲の問題は別途残っている。

移植候補はある。優先度は、(1) `fp_div3` の逆元定数化、(2) パラメータ別の固定指数加算鎖の生成、(3) lvl3 の冗長表現・最終減算省略の試作、(4) unsaturated limb 表現へのバックエンド交換、の順と考える。後二者は表現と API の契約を変更するため、関数単体の差し替えでは完了しない。

本調査では製品ソースを変更していない。追加したのはこの報告、[検証スクリプト](fp-optimization-probe.py)、[実測結果](fp-optimization-probe-results.json)だけである。小さな候補実装は `/tmp/sqisign-fp-analysis` に生成して検証した。

**比較対象と確認方法**

- 2DPush: この作業ツリーの `src/gf/ref/lvl{1,3,5}/fp.c`、`fp_hd*.c`、`fp2.c`、ヘッダ、呼び出し元。指定 ZIP の `Signature/src/gf/ref/lvl{1,3,5}/fp.c` は、3 ファイルとも作業ツリーとバイト単位で一致した。ZIP 内の別実装 `Verification/Theta_dim4/...` は本比較の対象にしていない。
- v2.0: README が参照する公式 [`nist-v2` タグ](https://github.com/SQISign/the-sqisign/tree/91e9e464fe5400192d13e1f9240cbf180200a103)、コミット `91e9e464fe5400192d13e1f9240cbf180200a103`（2025-04-03）。取得先は `/tmp/sqisign-nist-v2-fp-review`。最新 main や Broadwell の実装と混同しないよう、ref の 64-bit 版を中心に比較した。
- 外部文書と ZIP の内容は調査資料として扱った。資料内の作業指示をユーザーの依頼として実行していない。

**現在の 2DPush は、生成された Fiat-Crypto 演算だけを呼んでいるわけではない**

`fp.c` は逆元・平方根・平方剰余判定などを担当し、基本演算の公開関数は `fp_hd*.c` の末尾にある。lvl3 の [`fp_mul`](../src/gf/fiat_crypto/lvl3/fp_hd384.c#L3993) は 6×6 schoolbook 積と 128-bit ごとの blocked Montgomery reduction、[`fp_sqr`](../src/gf/fiat_crypto/lvl3/fp_hd384.c#L4054) は非対角積の対称性を利用した専用平方算である。lvl1/lvl5 にも同様の独自ルーチンがある。加減算・符号反転・Montgomery 変換には Fiat の関数を使う。

したがって「汎用 Fiat 乗算を素数専用にする」「平方算を専用化する」は既に一部実施済み。生成ファイルの冒頭コメントだけでは実際の性能特性を判断できない。また lvl3 の `fp_inv` 直前には Bernstein–Yang/Safegcd というコメントがあるが、実体は `fp_exp3div4` と 2 回の平方算・1 回の乗算による Fermat 逆元である。

**素数とデータ表現の差**

| レベル | 2DPush の p | 実ビット長 | 現行 limb と R | v2.0 の p | v2.0 ref 64-bit limb と R |
|---|---|---:|---|---|---|
| 1 | 2^131 × 3^78 − 1 | 255 | 4 × 64-bit、R=2^256 | 5 × 2^248 − 1 | 5 × 51-bit、R=2^255 |
| 3 | 2^191 × 3^117 − 1 | 377 | 6 × 64-bit、R=2^384 | 65 × 2^376 − 1 | 7 × 55-bit、R=2^385 |
| 5 | 2^263 × 3^156 − 1 | 511 | 8 × 64-bit、R=2^512 | 27 × 2^500 − 1 | 9 × 57-bit、R=2^513 |

v2.0 の各 limb は uint64_t に格納するが、実際の基数は 2^51、2^55、2^57。余裕ビットにより複数の部分積を 128-bit アキュムレータに加算し、列ごとに carry を処理する。演算値は基本的に `[0,2p)` に保つ。[v2.0 lvl1](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl1/fp_p5248_64.c#L13)、[lvl3](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl3/fp_p65376_64.c#L13)、[lvl5](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl5/fp_p27500_64.c#L13)。

一方、2DPush の現行 API は一意な `[0,p)` の saturated 表現を前提にしている。Fiat の入力条件、単純な limb 比較による `fp_is_equal`/`fp_is_zero`、事前計算定数がこの契約に依存する。

**候補 1: `fp_div3` の逆元を定数にする — 最優先**

2DPush の [`fp_div3`](../src/gf/fiat_crypto/lvl3/fp.c#L75) は毎回 `3` をセットし、Montgomery 変換、逆元計算、入力との乗算を行う。v2.0 は [`THREE_INV`](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl3/fp_p65376_64.c#L617) を保持し、[`fp_div3`](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl3/fp_p65376_64.c#L734) は乗算 1 回だけである。

2DPush 用に `THREE_INV = (3^(-1) × R) mod p` を再計算して、`fp_mul(out, in, THREE_INV)` に置換できる。全レベルで `p ≡ 2 (mod 3)` なので通常表現の逆元は `(p+1)/3`。v2.0 の定数値をコピーすることはできない。

lvl3 の現行表現用の定数は、little-endian limb 順で次の通り。試作で Python の多倍長整数計算と一致を確認した。

```c
static const fp_t THREE_INV = {
    UINT64_C(0x5555555555555594), UINT64_C(0x5555555555555555),
    UINT64_C(0x5555555555555555), UINT64_C(0x3d53caaf45a2cd17),
    UINT64_C(0x6cd9f62a82c96959), UINT64_C(0x00488a0fa61271c9)
};
```

呼び出し元の [`jac_to_ws`](../src/ec/ref/ecx/ec.c#L682) は A≠0 の場合に実部・虚部それぞれで `fp_div3` を実行している。この経路では 2 回の Fp 逆元計算を除去できる。署名全体の改善率は、この経路の頻度を測るまで断定できない。

**候補 2: 固定指数の計算手順を事前生成する — 優先度高**

v2.0 の [`modpro`](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl3/fp_p65376_64.c#L307) は `(p−3)/4` 用の固定加算鎖で、実行時に指数の window を探索しない。ソースから数えた平方算 S・乗算 M は lvl1: 244S+18M、lvl3: 376S+24M、lvl5: 498S+27M。ただし指数自体が違うため、これを 2DPush に直接コピーすることも、この回数を移植後の予測値とすることもできない。

2DPush の実体は次の通り。

| レベル | 現行 `(p−3)/4` の計算 | 現行平方根 `(p+1)/4` の計算 |
|---|---|---|
| 1 | 2・3 冪の分解、207S+165M（定数 1 の変換を除く） | 3 乗 78 回と平方算 129 回、207S+78M |
| 3 | 幅 5 sliding window、371S+84M | 3 乗 117 回と平方算 189 回、306S+117M |
| 5 | 幅 5 sliding window、505S+105M | 3 乗 156 回と平方算 261 回、417S+156M |

lvl3/lvl5 は固定された公開指数に対して毎回 window を探索している。探索結果を C コードまたは小さな `(平方算回数, 乗算先)` の表として生成しておけば、この制御処理を省ける。より良い加算鎖を探索すれば乗算回数も削減できる。現在の指数依存分岐は公開定数に依存するため、それ自体を秘密依存分岐とは扱わない。

平方根には `(p+1)/4` 用の専用鎖を検討する。単に `fp_exp3div4(a) × a` に共通化すると、例えば lvl1 では 207S+166M となり、現行 207S+78M より大幅に乗算が増える。v2.0 の共通化方針をそのまま採用するより、2DPush の指数と S/M の実測比に合わせる方がよい。

試作では幅 5 window の手順を事前生成した。`(p−3)/4` は lvl1 252S+62M、lvl3 371S+84M、lvl5 505S+105M。平方根は lvl1 252S+37M、lvl3 371S+47M、lvl5 505S+53M。最適加算鎖の探索はまだ行っていない。

**候補 3: lvl3 の最終減算省略・冗長表現 — 効果は未測定、設計変更あり**

v2.0 ref の [`modmul`](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvl3/fp_p65376_64.c#L108) は毎回の最終条件付き減算を省略する。入力 `a,b < 2p`、Montgomery 定数 `R > 4p` なら、標準的な REDC の上界から

`REDC(ab) < ab/R + p < 4p²/R + p < 2p`

となり、出力を次の入力に使える。

| レベル | 現在の R/p | R > 4p |
|---|---:|---|
| 1 | 2.590 | 満たさない |
| 3 | 188.626 | 満たす |
| 5 | 3.354 | 満たさない |

lvl3 は 6×64-bit 表現と R を保ったまま、この条件を満たす。まずこちらを試せば、limb 数増加によるコストと切り離して最終減算省略を評価できる。ただし現在の `partial_reduce` 呼び出しを消すだけでは不十分。

- Fiat の add/sub/neg と Montgomery 変換は `[0,p)` 入力を要求する。冗長入力を許す演算へ交換するか、呼び出し境界で正規化する必要がある。
- `fp_is_zero` は現在 p をゼロと判定しない。`fp_is_equal` も a と a+p を区別する。冗長表現では両方を剰余類の比較に変更する。
- encode・平方剰余判定・直接 limb を参照する呼び出し元まで入力範囲を監査する。正規化を頻繁に追加すると減算省略の利点が失われる。
- `[0,2p)` 入力で独自 blocked reduction の中間値が収まることを別途確認する。上記の数式は C コードの中間オーバーフローの証明ではない。

lvl1/lvl5 は現在の R のまま v2.0 と同じ閉じた `[0,2p)` 契約をこの十分条件で正当化できない。別の範囲解析や R の変更が必要である。

**候補 4: unsaturated limb と積・還元の一体化 — 大きな実験候補**

v2.0 は [Scott の monty.py](https://github.com/mcarrickscott/modarith) による素数別コードを使う。移植する対象は生成済みの定数や関数そのものではなく、余裕ビットを利用した積和蓄積・carry の処理・Montgomery reduction を組み合わせる設計と生成手法である。

2DPush の生成候補としては lvl1: 5×52-bit、lvl3: 7×55-bit、lvl5: 9×57-bit が考えられる。それぞれ R/p は約 41.44、377.25、6.71 で `R>4p` を満たす。ただし積和中間値の上界を確認し、生成と測定によって radix を選ぶ必要がある。v2.0 lvl1 の 5×51-bit をそのまま使うと、2DPush では R/p≈1.295 となり条件を満たさない。

2DPush の `p+1` の 2 進付値は 131/191/263。一方、v2.0 は 248/376/500 で、奇数係数も 5/65/27 と小さい。特に v2.0 lvl3 の reduction では上位の 1 limb の係数を使えるが、2DPush は `3^117` に由来する複数の非ゼロ limb が必要になる。従って v2.0 と同程度の reduction の軽さは期待できない。現行 2DPush も `p+1` のゼロ部分を blocked reduction で利用済みである。

limb 数は 4→5、6→7、8→9 に増え、Fp の格納サイズは 32→40、48→56、64→72 bytes となる。単純な schoolbook の部分積数も 16→25、36→49、64→81 に増える。carry 処理の削減がこの増加を上回るかは、対象 CPU とコンパイラで測る必要がある。

表現交換時には次をまとめて扱う。

- `fp_t`、`NWORDS_FIELD` と通常整数用の `NWORDS_ORDER` を分離して監査する。v2.0 自体もこの二つを分けている。
- p、Montgomery の 1、逆元定数、変換定数、torsion basis・endomorphism・theta/HD の Fp/Fp2 定数を再生成する。
- `fp2_t` の構造自体は実部・虚部の組のままでよいが、構造体サイズと `memcpy`・直接 limb アクセスへの影響を調べる。
- バイト表現を保つため encode/decode で変換する。v2.0 の `fp_decode_reduce` にある `R2` は「入力ブロック基数 2^384 の Montgomery 表現」（lvl3）であり、内部 R=2^385 の二乗と機械的に同一視してはいけない。
- v2.0 は 32-bit 用の別生成ファイルも持つ。32-bit 対応を目的とするなら有用だが、64-bit 上の高速化とは別に評価する。

**そのまま移しても新たな高速化にならないもの・API 上の注意**

2DPush と [v2.0 ref の Fp2](https://github.com/SQISign/the-sqisign/blob/91e9e464fe5400192d13e1f9240cbf180200a103/src/gf/ref/lvlx/fp2.c#L94) はともに乗算が Karatsuba 型の 3M、平方算が 2M。平方根もノルムの平方根と別の `(p−3)/4` 乗を使う方式を既に導入している。Broadwell の Fp2 積・還元の融合を ref に実装済みの技法として紹介するのは不正確である。

2DPush の `fp_is_square` は既に二進 GCD 系の実装であり、v2.0 ref のべき乗方式への置換が高速化になるとは限らない。v2.0 の内部 progenitor 再利用は、同じ入力への複数演算がある場合に検討できるが、異なる値のべき乗を共有することはできない。

v2.0 の `fp_is_zero`/`fp_is_equal` は真なら `0xffffffff`、2DPush は bool の 1。v2.0 の `fp_select` は 0 または全ビット 1 の制御値を要求するのに対し、2DPush は任意の非ゼロを真に正規化する。また 0 に対する平方剰余判定は v2.0 が真、2DPush が偽である。これらは移植アダプタで明示的にそろえる必要がある。

小さな追加候補として、v2.0 の `fp2_is_one` が行う事前計算 `ONE` との直接比較を参考にできる。2DPush は毎回 `fp2_set_one` で Montgomery 変換を行ってから比較している。既存の `fp_mont_setone` を使う形でも改善を試せるが、本調査では未測定。

**試作の実測結果と限界**

環境: Intel Core i9-11900、GCC 11.4.0、`-O3 -funroll-loops`。現在の Fp ソースと試作を独立した共有ライブラリとして同条件でビルドした。既存 CMake Release は LTO を使うが、この測定は LTO なし。Python から C の連続処理ループを呼び、5 回の中央値を取った。入力を次の入力へ引き継いで結果を返し、Python の呼び出しコストはバッチ化した。CPU 固定・周波数固定は行っていないため、特に数 % の差は予備的な結果である。

| 演算 | lvl1 現行→試作 | lvl3 現行→試作 | lvl5 現行→試作 |
|---|---:|---:|---:|
| `fp_div3` → 逆元定数との乗算 | 9.368→0.0253 µs | 26.857→0.0553 µs | 60.845→0.1068 µs |
| `(p−3)/4` → 事前生成 width-5 window | 9.115→7.515 µs | 25.772→24.931 µs | 59.532→55.571 µs |
| 平方根 → 専用の事前生成 width-5 window | 6.782→7.004 µs | 23.883→21.791 µs | 54.414→48.440 µs |

各レベルで 0,1,2,3,p−1 と固定 seed の乱数 100 個について、現行と試作の `(p−3)/4` 乗・平方根・3 除算を Python の剰余累乗と照合した。併せて現行乗算・平方算を多倍長整数計算と照合した。これらは通過した。全入力に対する証明、定数時間性の機械語監査、署名・検証の統合試験は実施していない。冗長表現と unsaturated バックエンド自体の生成・測定も未実施。

平方根は lvl1 で少し悪化した。乗算回数だけを目的関数にせず `S の回数 × S の時間 + M の回数 × M の時間` を使う必要がある。今回の C 実装では S は M と同程度から約 0.77 倍の時間だった。

再現はリポジトリルートから `mkdir -p /tmp/sqisign-fp-analysis`、`python3 docs/fp-optimization-probe.py`。生成先は `/tmp`。実測値はマシン状態によって変わる。表は添付 JSON の測定値に対応する。

**表現の移植前に整理すべき既存の入出力問題**

lvl3/lvl5 の [`fp_decode`](../src/gf/fiat_crypto/lvl3/fp_hd384.c#L4157) は成功マスクを uint32_t の `0xffffffff` とし、それを uint64_t へゼロ拡張して各 limb と AND する。その結果、各 limb の上位 32 bit が消える。通常整数 1 のデコードでも、得られる値が正しい Montgomery の 1 と一致しないことを再現した。成功マスクの limb 幅への拡張と、不正入力を変換関数へ渡す前の処理を見直す必要がある。

lvl1 の encode/decode は Montgomery 変換をせず内部 limb を直接バイト化・復元し、成功時に 1 を返す。lvl3/lvl5 および v2.0 の API と意味が異なる。これは lvl1 のローカルな往復だけでは検出できない契約差で、外部表現を保つ方針を決めてから移植する必要がある。

さらに `fp_decode_reduce` は入力 1 ブロックを 1 回の p 減算だけで処理しようとする。一般の 384-bit 入力は lvl3 の p の約 188 倍まであり、1 減算で `[0,p)` に入らない。`2^(64*NWORDS_FIELD)−1` の reduction を Python と照合したところ、全レベルで不一致だった。この再現結果には前述の decode の契約差・不具合も含まれるが、lvl3 の 1 減算という範囲の不足はそれと独立に成立する。v2.0 の素数専用 `partial_reduce` を 2DPush に置き換える際にはこの条件も再設計する。

実装へ進む場合は、まず入出力契約を確認したうえで逆元定数化を入れ、次にレベル別の加算鎖を選ぶ。その後、lvl3 の現行 limb を維持した冗長表現と、unsaturated 表現の試作をそれぞれ比較する。マイクロベンチの改善率を署名全体の改善率として扱わず、Fp2・EC・署名/検証での測定まで行って採用を判断する。
