#!/usr/bin/env python3
"""Matched Fp benchmark. --baseline is a complete pre-migration source checkout.
Uses each checkout's own headers, Montgomery R and limb layout. Inputs encode
the same ordinary values. Reports median wall ns/op over shuffled rounds.
"""
import argparse
import ctypes as C
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time
import sys
sys.dont_write_bytecode = True
from test_fp_backend import build, FLAGS
from gen_fp_backend import PARAMETERS, ROOT

OPS = ['add', 'sub', 'mul', 'sqr', 'inv', 'sqrt', 'exp3div4', 'is_square', 'div3', 'fp2_mul', 'fp2_sqr']
WRAPPER = '''
#include <fp2.h>
uint64_t run_batch(int op, int count, const fp2_t *inputs) {
    fp_t r;
    fp2_t r2;
    uint64_t checksum = 0;
    for (int i = 0; i < count; i++) {
        const fp2_t *a = &inputs[2*(i%64)], *b = a+1;
        switch(op) {
        case 0: fp_add(r, a->re, b->re); break;
        case 1: fp_sub(r, a->re, b->re); break;
        case 2: fp_mul(r, a->re, b->re); break;
        case 3: fp_sqr(r, a->re); break;
        case 4: fp_copy(r, a->re); fp_inv(r); break;
        case 5: fp_copy(r, a->re); fp_sqrt(r); break;
        case 6: fp_exp3div4(r, a->re); break;
        case 7: checksum ^= fp_is_square(a->re); continue;
        case 8: fp_div3(r, a->re); break;
        case 9: fp2_mul(&r2, a, b); checksum ^= r2.re[0]; continue;
        default: fp2_sqr(&r2, a); checksum ^= r2.re[0]; continue;
        }
        checksum ^= r[0];
    }
    return checksum;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    rng = random.Random(731)
    report = {'compiler': subprocess.check_output(['cc', '--version'], text=True).splitlines()[0],
              'flags': FLAGS, 'cpu': cpu, 'rounds': 9, 'levels': []}
    with tempfile.TemporaryDirectory(prefix='sqisign-fp-bench-') as tmp:
        tmp = Path(tmp)
        wrapper = tmp/'wrapper.c'
        wrapper.write_text(WRAPPER)
        for level, a, b, radix, limbs, words in PARAMETERS:
            p = (1 << a) * 3**b - 1
            libs = [build(root, level, tmp/f'{label}{level}.so', wrapper)
                    for root, label in [(args.baseline.resolve(), 'base'), (ROOT, 'new')]]
            values = [rng.randrange(1, p) for _ in range(256)]
            inputs = []
            for n, rd in [(words, 64), (limbs, radix)]:
                raw = []
                for v in values:
                    v = v*(1<<(n*rd))%p
                    raw += [(v>>(i*rd)) & ((1<<rd)-1) for i in range(n)]
                inputs.append((C.c_uint64*len(raw))(*raw))
            times = {(j, op): [] for j in range(2) for op in range(len(OPS))}
            for lib in libs:
                lib.run_batch.restype = C.c_uint64
            for trial in range(report['rounds']):
                keys = list(times)
                rng.shuffle(keys)
                for j, op in keys:
                    count = 1000 if 4 <= op <= 8 else 100000
                    start = time.perf_counter_ns()
                    libs[j].run_batch(op, count, inputs[j])
                    times[j, op].append((time.perf_counter_ns()-start)/count)
            row = {'level': level, 'ns_per_op': {}}
            for op, name in enumerate(OPS):
                old, new = (statistics.median(times[j, op]) for j in range(2))
                row['ns_per_op'][name] = {'before': round(old, 2), 'after': round(new, 2), 'speedup': round(old/new, 3)}
            report['levels'].append(row)
            print(json.dumps(row), flush=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')

if __name__ == '__main__':
    main()
