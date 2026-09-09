#!/usr/bin/env python3
"""Migrate precomputed Fp constants from a saturated-backend source checkout.

Usage: python3 scripts/convert_fp_precomp.py /path/to/pre-migration/checkout
The input must use 4/6/8 saturated 64-bit Montgomery words. Output goes to the
current checkout. Integers, quaternion matrices and inactive 16/32-bit branches
are left intact. The represented field value (including projective scales) is
preserved; this is not ordinary-integer-to-Montgomery conversion.
"""
from pathlib import Path
import argparse
import re
import sys
sys.dont_write_bytecode = True
from gen_fp_backend import PARAMETERS, ROOT


def convert(source, level, p, radix, limbs, words, name):
    ratio = pow(1 << (64 * words), -1, p) * (1 << (radix * limbs)) % p
    count = 0
    def leaf(match):
        nonlocal count
        vals = [int(x, 0) for x in re.findall(r'0x[0-9a-fA-F]+|\b\d+\b', match[0])]
        if not vals or len(vals) > words:
            return match[0]
        v = sum(x << (64*i) for i, x in enumerate(vals))
        assert v < p, (level, name, v)
        v = v * ratio % p
        count += 1
        return '{' + ', '.join(f'0x{(v>>(radix*i)) & ((1<<radix)-1):x}' for i in range(limbs)) + '}'
    if name == 'endomorphism_action.c':
        marker = '#elif 8 * DIGIT_LEN == 64'
        before, active = source.split(marker, 1) if marker in source else ('', source)
        marker = marker if before else ''
        def decl(m):
            return re.sub(r'\{[^{}]*\}', leaf, m[0])
        active = re.sub(r'const ec_(?:basis|curve|point)_t\s+\w+\s*=.*?;', decl, active, flags=re.S)
        source = before + marker + active
    elif name == 'hd_splitting_transforms.c':
        # Only field-valued 64-bit branches; tensor index tables remain integers.
        source = re.sub(r'(#elif RADIX == 64\n)(.*?)(?=\n#endif\n#endif)',
                        lambda m: m[1] + re.sub(r'\{[^{}]*\}', leaf, m[2]), source, flags=re.S)
    else:
        source = re.sub(r'(const fp2_t\s+.*?;)',
                        lambda m: re.sub(r'\{[^{}]*\}', leaf, m[0]), source, flags=re.S)
    assert count, (level, name)
    print(f'lvl{level}/{name}: {count} Fp constants')
    return source


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == ROOT:
        parser.error('source must be a separate pre-migration checkout')
    for level, a, b, radix, limbs, words in PARAMETERS:
        p = (1 << a) * 3**b - 1
        constants = args.source / f'src/precomp/ref/lvl{level}/include/fp_constants.h'
        assert re.search(rf'#define NWORDS_FIELD {words}\b', constants.read_text())
        for name in ('endomorphism_action.c', 'hd_splitting_transforms.c', 'gf_constants.c'):
            path = Path(f'src/precomp/ref/lvl{level}/{name}')
            if (args.source / path).exists():
                (ROOT / path).write_text(convert((args.source/path).read_text(), level, p, radix, limbs, words, name))

if __name__ == '__main__':
    main()
