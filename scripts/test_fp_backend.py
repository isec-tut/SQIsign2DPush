#!/usr/bin/env python3
"""Independent Python-integer oracle for the unsaturated Fp backend.

Compile in a temporary directory and check redundant representatives, range,
aliasing, Montgomery conversion, Legendre, division, and serialization.
"""
import argparse
import ctypes as C
from pathlib import Path
import random
import subprocess
import tempfile
import sys
sys.dont_write_bytecode = True
from gen_fp_backend import PARAMETERS, ROOT

FLAGS = ['-O3', '-funroll-loops', '-flto', '-shared', '-fPIC', '-Wl,-Bsymbolic',
         '-DDISABLE_NAMESPACING', '-DRADIX_64', '-DTARGET_AMD64',
         '-DTARGET_OS_UNIX', '-DSQISIGN_BUILD_TYPE_REF']


def build(root, level, destination, wrapper=None):
    includes = ['include', 'src/common/generic/include',
                f'src/precomp/ref/lvl{level}/include', f'src/gf/ref/lvl{level}/include']
    base = root / f'src/gf/ref/lvl{level}'
    backend = 'fp_backend.c' if (base/'fp_backend.c').exists() else f'fp_hd{ {1:256,3:384,5:512}[level]}.c'
    command = ['cc', *FLAGS, f'-DSQISIGN_VARIANT=lvl{level}']
    for inc in includes:
        command += ['-I', str(root/inc)]
    command += [str(base/name) for name in ['fp.c', backend, 'fp_exp.c', 'fp2.c']]
    if wrapper:
        command += [str(wrapper)]
    command += ['-o', str(destination)]
    subprocess.run(command, check=True, capture_output=True)
    return C.CDLL(str(destination))


def check(lib, params, repetitions):
    level, a, b, radix, limbs, words = params
    rng = random.Random(1298 + level)
    p = (1 << a) * 3**b - 1
    R = 1 << (limbs * radix)
    Rinv = pow(R, -1, p)
    T = C.c_uint64 * limbs
    lib.fp_decode.restype = C.c_uint32
    for name in ['fp_is_square', 'fp_is_equal', 'fp_is_zero']:
        getattr(lib, name).restype = C.c_bool
    def pack(v, rd=radix):
        return T(*[(v >> (i*rd)) & ((1<<rd)-1) for i in range(limbs)])
    def val(v, rd=radix):
        return sum(int(x) << (rd*i) for i, x in enumerate(v))
    def result(v, want):
        assert all(x < 1 << radix for x in v)
        assert val(v) < 2*p and val(v) % p == want
    edges = [0, 1, p-1, p, p+1, 2*p-1]
    pairs = [(a, b) for a in edges for b in edges]
    # Exercise carry/borrow chains and the exact 2p reduction threshold.
    # Uniform random inputs almost never hit radix boundaries.
    totals = {2*p-2, 2*p-1, 2*p, 2*p+1, 2*p+2, 4*p-2}
    for i in range(1, limbs):
        for delta in (-2, -1, 0, 1, 2):
            totals.add((1 << (i*radix)) + delta)
            totals.add(2*p + (1 << (i*radix)) + delta)
    for total in sorted(totals):
        if not 0 <= total <= 4*p-2:
            continue
        low, high = max(0, total-(2*p-1)), min(2*p-1, total)
        for av in (low, high, rng.randrange(low, high+1)):
            pairs.append((av, total-av))
    pairs += [(rng.randrange(2*p), rng.randrange(2*p)) for _ in range(repetitions)]
    for av, bv in pairs:
        aa, bb, cc = pack(av), pack(bv), T()
        for op, want in [('add', (av+bv)%p), ('sub', (av-bv)%p),
                         ('mul', av*bv*Rinv%p)]:
            for alias in range(3):
                x, y = pack(av), pack(bv)
                z = T() if alias == 0 else x if alias == 1 else y
                getattr(lib, 'fp_'+op)(z, x, y)
                result(z, want)
                if op in ('add', 'sub'):
                    assert val(z) == (av+bv if op == 'add' else av-bv) % (2*p)
        for op, want in [('sqr', av*av*Rinv%p), ('neg', -av%p),
                         ('div3', av*pow(3, -1, p)%p)]:
            for alias in range(2):
                x = pack(av)
                z = x if alias else T()
                getattr(lib, 'fp_'+op)(z, x)
                result(z, want)
        ordinary = av*Rinv%p
        lib.fp_frommont(cc, aa)
        assert val(cc, 64) == ordinary
        lib.fp_tomont(cc, cc)
        result(cc, av%p)
        assert lib.fp_is_zero(aa) == (av%p == 0)
        assert lib.fp_is_equal(aa, pack(av%p))
        assert lib.fp_is_equal(aa, bb) == (av%p == bv%p)
        assert lib.fp_is_square(aa) == (pow(ordinary, (p-1)//2, p) == 1)
        for name, want in [('fp_encode', ordinary),
                           ('fp_encode_legacy_hash', ordinary*(1<<(64*words))%p)]:
            buf = C.create_string_buffer(words*8)
            getattr(lib, name)(buf, aa)
            assert int.from_bytes(buf.raw, 'little') == want
    for v in [0, 1, p-1, p, p+1, (1<<(words*64))-1] + [rng.getrandbits(words*64) for _ in range(repetitions)]:
        buf = C.create_string_buffer(v.to_bytes(words*8, 'little'))
        cc = T()
        status = lib.fp_decode(cc, buf)
        assert status == (2**32-1 if v < p else 0)
        result(cc, v*R%p if v < p else 0)
    for length in [0, 1, 31, 32, 33, 47, 48, 49, 63, 64, 65, 128, 200]:
        for data in [bytes(length), bytes([255])*length, rng.randbytes(length)]:
            cc = T()
            lib.fp_decode_reduce(cc, data, len(data))
            result(cc, int.from_bytes(data, 'little')*R%p)
    for v in [0, 1, 3, (1<<64)-1]:
        cc = T()
        lib.fp_set_small(cc, C.c_uint64(v))
        result(cc, v*R%p)
    print(f'lvl{level}: integer oracle passed ({repetitions} random cases + boundaries)', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repetitions', type=int, default=1000)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='sqisign-fp-oracle-') as tmp:
        for params in PARAMETERS:
            lib = build(ROOT, params[0], Path(tmp)/f'fp{params[0]}.so')
            check(lib, params, args.repetitions)

if __name__ == '__main__':
    main()
