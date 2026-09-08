# C implementation of SQIsign2dPush

This library is a C implementation of SQIsign2DPush
It uses the base code of [SQIsign-v2.0](https://github.com/SQIsign/the-sqisign/tree/nist-v2) and [SQIsignHD-v1.0](https://github.com/Pierrick-Dartois/SQISignHD-lib/tree/v1).

## Requirements

- CMake (version 3.5 or later)
- C99-compatible C compiler and an assembler
- GMP development files, unless building GMP with `-DENABLE_GMP_BUILD=ON`
- Make, or another build tool supported by the selected CMake generator

## Build and test

```sh
rm -rf build
cmake -S . -B build -DSQISIGN_BUILD_TYPE=ref -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

This builds the reference implementation in Release mode. If you only want to
build the SQIsign2DPush timing tests, choose the security level you want and run
one of the following commands from the repository root:

```sh
cmake --build build --target sqisign_test_sqisign2dpush_lvl1
cmake --build build --target sqisign_test_sqisign2dpush_lvl3
cmake --build build --target sqisign_test_sqisign2dpush_lvl5
```

Here `lvl1`, `lvl3`, and `lvl5` are the three parameter sets. Larger levels are
slower and correspond to stronger security parameters.

After building, run the matching test binary:

```sh
./build/src/sqisign2dpush/ref/lvl1/test/sqisign_test_sqisign2dpush_lvl1
./build/src/sqisign2dpush/ref/lvl3/test/sqisign_test_sqisign2dpush_lvl3
./build/src/sqisign2dpush/ref/lvl5/test/sqisign_test_sqisign2dpush_lvl5
```

For example, to measure level 1 only:

```sh
cmake --build build --target sqisign_test_sqisign2dpush_lvl1
./build/src/sqisign2dpush/ref/lvl1/test/sqisign_test_sqisign2dpush_lvl1
```

## Finite-field benchmarks

The `Fp` arithmetic in each parameter level's `fp.c` uses the corresponding
generated file: `fp_hd256.c` for level 1, `fp_hd384.c` for level 3, and
`fp_hd512.c` for level 5. These files are generated using
[Fiat-Crypto](https://github.com/mit-plv/fiat-crypto).

The finite-field test binaries accept either `test <reps>` for correctness
tests or `bench <reps>` for cycle-count benchmarks. For example, to benchmark
the level 1 $GF(p)$ and $GF(p^2)$ arithmetic from the repository root:

```sh
cmake --build build --target sqisign_test_gf_lvl1_fp
cmake --build build --target sqisign_test_gf_lvl1_fp2

./build/src/gf/ref/lvl1/test/sqisign_test_gf_lvl1_fp bench 100000
./build/src/gf/ref/lvl1/test/sqisign_test_gf_lvl1_fp2 bench 100000
```

The corresponding level 3 and level 5 binaries are:

```sh
./build/src/gf/ref/lvl3/test/sqisign_test_gf_lvl3_fp bench 100000
./build/src/gf/ref/lvl3/test/sqisign_test_gf_lvl3_fp2 bench 100000
./build/src/gf/ref/lvl5/test/sqisign_test_gf_lvl5_fp bench 100000
./build/src/gf/ref/lvl5/test/sqisign_test_gf_lvl5_fp2 bench 100000
```

## SQIsign2DPush timing output

The SQIsign2DPush test repeats key generation, signing, and verification, then
prints their average running times. For each operation, it also splits the
average time into four categories:

- `Isogeny computations`: one-dimensional isogeny evaluation and initialization
  such as `ec_eval_even`, `ec_eval_three`, `isog_init_three`, and
  `isog_init_two`, plus two-dimensional isogeny chains such as
  `theta_chain_compute_and_eval` and `theta_chain_compute_and_eval_verify`.
- `EC/basis except isogeny`: elliptic-curve and basis operations which are not
  counted as isogeny computation, such as basis generation, basis changes,
  biscalar multiplication, pairings, dlogs, kernel setup, and matrix
  application to bases.
- `Quaternion algorithms`: quaternion and ideal/lattice algorithms, including
  `quat_*`, `represent_integer`, and `sample_response`. Conversion routines
  between quaternion ideals and elliptic-curve data are split according to
  their internal quaternion, elliptic-curve, and other work when separately
  measured.
- `Other`: remaining measured time not assigned to the above categories, such
  as hashing, setup, final checks, and bookkeeping.

The percentage is computed with the average operation time as the denominator.
For example, if average signing takes `53.62923 ms`, then
`Quaternion algorithms: 27.23998 ms (50.79%)` means that about half of the
signing time was spent in quaternion-related code.

Example output:

```text
Average keygen time [14.20469 ms]
Avg keygen:      35.45607 Mcycles
Median keygen:   33.23159 Mcycles
Min keygen:      24.34429 Mcycles
Max keygen:      73.93395 Mcycles
  Keygen category ratios (denominator: avg keygen)
    Isogeny computations:         3.48792 ms ( 24.55%)
    EC/basis except isogeny:      4.98671 ms ( 35.11%)
    Quaternion algorithms:        5.58854 ms ( 39.34%)
    Other:                        0.14152 ms (  1.00%)
average signing time [28.90103 ms]
Avg signing:     72.13934 Mcycles
Median signing:  70.71151 Mcycles
Min signing:     58.24607 Mcycles
Max signing:     103.74216 Mcycles
  Signing category ratios (denominator: avg signing)
    Isogeny computations:         10.46034 ms ( 36.19%)
    EC/basis except isogeny:      9.32175 ms ( 32.25%)
    Quaternion algorithms:        8.69469 ms ( 30.08%)
    Other:                        0.42425 ms (  1.47%)
average verification time [4.73436 ms]
Avg verify:      11.81735 Mcycles
Median verify:   11.79898 Mcycles
Min verify:      11.65920 Mcycles
Max verify:      12.18837 Mcycles
  Verify category ratios (denominator: avg verify)
    Isogeny computations:         3.03324 ms ( 64.07%)
    EC/basis except isogeny:      1.63858 ms ( 34.61%)
    Quaternion algorithms:        0.00000 ms (  0.00%)
    Other:                        0.06254 ms (  1.32%)
All tests passed!
```

## Build options

CMake build options can be specified with `-D<BUILD_OPTION>=<VALUE>`.

### ENABLE_GMP_BUILD

If set to `OFF` (by default), the gmp library on the system is dynamically linked.
If set to `ON`, a custom gmp library is linked, which is built as part of the overall build process. 

In the latter case, the following further options are available:
- `ENABLE_GMP_STATIC`: Does static linking against gmp. The default is `OFF`.
- `GMP_BUILD_CONFIG_ARGS`: Provides additional config arguments for the gmp build (for example `--disable-assembly`). By default, no config arguments are provided.

### ENABLE_DOCS

If set to `OFF` (by default), documentation targets are not configured. If set
to `ON`, Doxygen is required and the `doc` target is added:

```sh
cmake -S . -B build -DENABLE_DOCS=ON
cmake --build build --target doc
```

### SQISIGN_BUILD_TYPE

Specifies the build type for which SQIsign is built. The currently supported flags are:
- `ref`, which builds the plain C reference implementation.

### CMAKE_BUILD_TYPE

Can be used to specify special build types. The options are:

- `Release`: Builds with optimizations enabled and assertions disabled.
- `Debug`: Builds with debug symbols.
- `ASAN`: Builds with AddressSanitizer memory error detector.
- `LSAN`: Builds with LeakSanitizer for run-time memory leak detection.
- `UBSAN`: Builds with UndefinedBehaviorSanitizer for undefined behavior detection.

The default build type uses the flags `-O3 -Wstrict-prototypes -Wno-error=strict-prototypes -fvisibility=hidden -Wno-error=implicit-function-declaration -Wno-error=attributes`. (Notice that assertions remain enabled in this configuration, which harms performance.)

## License

SQIsign2DPush is licensed under Apache-2.0. See LICENSE and NOTICE in the root directory. 

Most functionalities of this code are based on [SQIsign-v2.0](https://github.com/SQIsign/the-sqisign/tree/nist-v2) and [SQIsignHD-v1.0](https://github.com/Pierrick-Dartois/SQISignHD-lib/tree/v1) under the license MIT: "Copyright (c) 2023 The SQIsign team".

Third party code is used in some test and common code files of this directory (`Signature`):

- `src/common/generic/aes_c.c`; MIT: "Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>"
- `src/common/generic/fips202.c`: Public Domain
- `src/common/generic/randombytes_system.c`: MIT: Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>
