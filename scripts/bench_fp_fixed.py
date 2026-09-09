#!/usr/bin/env python3
"""Compare a saved reference Fp backend with the current one on x86-64 Linux.

--baseline contains lvl1/, lvl3/, lvl5/ copies of fp.c, fp2.c, fp_hd*.c
(and fp_exp.c if present). Current headers are used for both builds.
Builds live in a temporary directory; the repository build/ is not modified.
"""

import argparse
import ctypes
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
from gen_fp_fixed import PARAMETERS, ROOT


FLAGS = ["-O3", "-funroll-loops", "-flto", "-shared", "-fPIC", "-Wl,-Bsymbolic",
         "-DDISABLE_NAMESPACING", "-DRADIX_64", "-DTARGET_AMD64",
         "-DTARGET_OS_UNIX", "-DSQISIGN_BUILD_TYPE_REF"]
WRAPPER = """
#include <fp2.h>
uint64_t run_batch(int op, int count, const fp2_t *inputs) {
    fp_t result;
    fp2_t result2;
    uint64_t checksum = 0;
    for (int i = 0; i < count; i++) {
        const fp2_t *input = &inputs[i % 64];
        switch (op) {
        case 0: fp_exp3div4(result, input->re); break;
        case 1: fp_copy(result, input->re); fp_sqrt(result); break;
        case 2: fp_copy(result, input->re); fp_inv(result); break;
        default:
            fp2_copy(&result2, input);
            fp2_sqrt(&result2);
            fp_copy(result, result2.re);
            checksum ^= result2.im[0];
            break;
        }
        checksum ^= result[0];
    }
    return checksum;
}
"""


def build(compiler, level, source, destination):
    includes = ["include", "src/common/generic/include",
                f"src/precomp/ref/lvl{level}/include",
                f"src/gf/ref/lvl{level}/include", f"src/gf/ref/lvl{level}"]
    bits = {1: 256, 3: 384, 5: 512}[level]
    files = [source / name for name in ("fp.c", "fp2.c", f"fp_hd{bits}.c")]
    if (source / "fp_exp.c").exists():
        files.append(source / "fp_exp.c")
    wrapper = destination.with_suffix(".c")
    wrapper.write_text(WRAPPER)
    command = [compiler, *FLAGS, f"-DSQISIGN_VARIANT=lvl{level}"]
    for include in includes:
        command += ["-I", str(ROOT / include)]
    command += [*map(str, files), str(wrapper), "-o", str(destination)]
    subprocess.run(command, check=True, capture_output=True)
    library = ctypes.CDLL(str(destination))
    library.run_batch.restype = ctypes.c_uint64
    return library


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cc", default="cc")
    args = parser.parse_args()
    if (ROOT / "src/gf/ref/lvl1/fp_backend.c").exists():
        parser.error("Use bench_fp_backend.py with a complete pre-migration checkout; the field layouts now differ.")
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    rng = random.Random(20260908)
    report = {"compiler": subprocess.check_output([args.cc, "--version"], text=True).splitlines()[0],
              "flags": FLAGS, "cpu_affinity": cpu, "iterations": 1000,
              "rounds": 9, "statistic": "median ns/call", "levels": []}
    for line in Path("/proc/cpuinfo").read_text().splitlines():
        if line.startswith("model name"):
            report["cpu"] = line.split(":", 1)[1].strip()
            break
    with tempfile.TemporaryDirectory(prefix="sqisign-fixed-bench-") as work:
        for level, (twos, threes) in PARAMETERS.items():
            n = {1: 4, 3: 6, 5: 8}[level]
            prime = (1 << twos) * 3**threes - 1
            montgomery = 1 << (64 * n)
            field = ctypes.c_uint64 * n

            class Fp2(ctypes.Structure):
                _fields_ = [("re", field), ("im", field)]

            def pack(value):
                value = value * montgomery % prime
                return field(*[(value >> (64 * i)) & ((1 << 64) - 1) for i in range(n)])

            inputs = (Fp2 * 64)()
            for i in range(64):
                a, b = rng.randrange(1, prime), rng.randrange(1, prime)
                inputs[i] = Fp2(pack((a*a - b*b) % prime), pack(2*a*b % prime))
            sources = [args.baseline / f"lvl{level}", ROOT / f"src/gf/ref/lvl{level}"]
            libraries = [build(args.cc, level, source, Path(work) / f"lvl{level}_{j}.so")
                         for j, source in enumerate(sources)]
            # Also check the compiled exponentiation against Python integers.
            for library in libraries:
                for value in [0, 1, prime - 1] + [rng.randrange(prime) for _ in range(64)]:
                    input_value, result = pack(value), field()
                    library.fp_exp3div4(result, input_value)
                    assert list(result) == list(pack(pow(value, (prime - 3)//4, prime)))
                    library.fp_sqrt(input_value)
                    assert list(input_value) == list(pack(pow(value, (prime + 1)//4, prime)))
            cases = [(j, op) for j in range(2) for op in range(4)]
            samples = {case: [] for case in cases}
            for j, op in cases:
                libraries[j].run_batch(op, 64, inputs)
            for _ in range(report["rounds"]):
                rng.shuffle(cases)
                for j, op in cases:
                    start = time.perf_counter_ns()
                    libraries[j].run_batch(op, report["iterations"], inputs)
                    samples[j, op].append((time.perf_counter_ns() - start) / report["iterations"])
            row = {"level": level,
                   "baseline_fp_sha256": hashlib.sha256((sources[0] / "fp.c").read_bytes()).hexdigest(),
                   "operations": {}}
            for op, name in enumerate(("fp_exp3div4", "fp_sqrt", "fp_inv", "fp2_sqrt")):
                old = statistics.median(samples[0, op])
                new = statistics.median(samples[1, op])
                row["operations"][name] = {"before_ns": round(old, 1), "after_ns": round(new, 1),
                                           "time_reduction_percent": round(100*(1-new/old), 2)}
            report["levels"].append(row)
            print(json.dumps(row), flush=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
