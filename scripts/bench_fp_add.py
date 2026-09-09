#!/usr/bin/env python3
"""Compare addition before/after carry fusion, using the same limb layout.

--baseline is a source checkout with the unsaturated backend before the change.
Independent-input throughput and dependent chains are measured separately.
Subtraction is an unchanged control; Fp2 multiplication checks downstream cost.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import sys
import tempfile
import time
sys.dont_write_bytecode = True
from test_fp_backend import build, FLAGS
from gen_fp_backend import PARAMETERS, ROOT

NAMES = ('add', 'sub', 'add_chain', 'sub_chain', 'fp2mul')


def wrapper_code():
    code = '#include <fp2.h>\n'
    for name in ('add', 'sub'):
        for chain in (False, True):
            function = f'run_{name}' + ('_chain' if chain else '')
            left = 'r' if chain else 'a->re'
            code += f'''
uint64_t {function}(int count, const fp2_t *inputs) {{
    fp_t r;
    fp_copy(r, inputs[0].re);
    uint64_t checksum = 0;
    for (int i = 0; i < count; i++) {{
        const fp2_t *a = &inputs[i % 64];
        fp_{name}(r, {left}, a->im);
        checksum ^= r[0];
    }}
    return checksum;
}}
'''
    code += '''
uint64_t run_fp2mul(int count, const fp2_t *inputs) {
    fp2_t r;
    uint64_t checksum = 0;
    for (int i = 0; i < count; i++) {
        fp2_mul(&r, &inputs[i % 64], &inputs[(i+1) % 64]);
        checksum ^= r.re[0];
    }
    return checksum;
}
'''
    return code


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    rng = random.Random(9362)
    report = {'compiler': subprocess.check_output(['cc', '--version'], text=True).splitlines()[0],
              'flags': FLAGS, 'cpu': cpu, 'rounds': 31, 'iterations': 300000,
              'units': 'median ns/op', 'levels': []}
    with tempfile.TemporaryDirectory(prefix='sqisign-add-bench-') as tmp:
        tmp = Path(tmp)
        wrapper = tmp/'wrapper.c'
        wrapper.write_text(wrapper_code())
        for level, a, b, radix, limbs, words in PARAMETERS:
            p = (1 << a)*3**b-1
            mask = (1 << radix)-1
            roots = (args.baseline.resolve(), ROOT)
            for root in roots:
                header = (root/f'src/precomp/ref/lvl{level}/include/fp_constants.h').read_text()
                if f'#define FP_RADIX {radix}' not in header or f'#define NWORDS_FIELD {limbs}' not in header:
                    parser.error('Both checkouts must use the same unsaturated field layout')
            libs = [build(root, level, tmp/f'{j}-{level}.so', wrapper) for j, root in enumerate(roots)]
            raw = [(v >> (i*radix)) & mask for v in [rng.randrange(2*p) for _ in range(128)] for i in range(limbs)]
            inputs = (C.c_uint64*len(raw))(*raw)
            keys = [(j, name) for j in range(2) for name in NAMES]
            times = {key: [] for key in keys}
            for j, name in keys:
                getattr(libs[j], 'run_'+name).restype = C.c_uint64
            for name in NAMES:
                # Warm up and check that the measured computations agree.
                assert getattr(libs[0], 'run_'+name)(1025, inputs) == getattr(libs[1], 'run_'+name)(1025, inputs)
            for _ in range(report['rounds']):
                rng.shuffle(keys)
                for j, name in keys:
                    start = time.perf_counter_ns()
                    getattr(libs[j], 'run_'+name)(report['iterations'], inputs)
                    times[j, name].append((time.perf_counter_ns()-start)/report['iterations'])
            row = {'level': level, 'backend_sha256': [hashlib.sha256((root/f'src/gf/ref/lvl{level}/fp_backend.c').read_bytes()).hexdigest() for root in roots],
                   'ns_per_op': {name: {'before': round(statistics.median(times[0, name]), 3),
                                        'after': round(statistics.median(times[1, name]), 3)} for name in NAMES}}
            report['levels'].append(row)
            print(json.dumps(row), flush=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')

if __name__ == '__main__':
    main()
