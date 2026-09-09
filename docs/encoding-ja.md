# SQIsign2DPush のバイナリ形式

`encoding.c` の codec は、SQIsign-v2.0 Section 4.6 の little-endian 規約を基礎に、2DPush の署名構造をエンコードする。

## 公開鍵

公開鍵は次の順序で格納する。

1. Montgomery 曲線係数 `A` の `Fp2` wire 表現
2. `Epk[3^e2]` の basis hint (`uint16_t`, little-endian)

サイズは NIST-I/III/V でそれぞれ `66/98/130` byte。

## 秘密鍵

秘密鍵は公開鍵の後に次を格納する。

1. transporter から復元した quaternion `gamma` の分母 (`1` または `2`)
2. `gamma` の4係数（符号付き two's complement）
3. `2 x 2` の `Z/3^e2 Z` 行列（各係数は unsigned little-endian）
4. `2^e1` torsion basis の affine x 座標3個（`P`, `Q`, `P-Q`）

サイズは NIST-I/III/V で `455/679/899` byte。

## 署名

署名は次の順序で格納する。

1. `Eaux` の Montgomery 曲線係数 `A`
2. `n1`
3. `mat_rsp` の4係数（`Z/2^e1 Z`）
4. challenge（`Z/3^e2 Z`）
5. `hint_aux`
6. `hint_chall`

実装でのサイズは NIST-I/III/V で `151/219/295` byte。NIST-V の `295` byte は、`128 + 2 + 4*33 + 31 + 2` の合計である。論文 Table 2 にある `297` byte とは2 byteの差があるため、仕様値との扱いを決める場合は注意する。

## 検証

デコーダは入力長、有限体要素の範囲、曲線の非特異性、各整数の範囲、行列の可逆性、torsion basis の形式を検証する。失敗時は出力オブジェクトを変更せず `0` を返す。
