# Developer notes

This repository contains a C reference implementation used to evaluate
SQIsign2DPush. It is based on the SQIsign codebase, with additional
SQIsign2DPush code and timing instrumentation.

## Main code layout

- `src/sqisign2dpush/ref/sqisign2dpushx/`
  - `keygen.c`: key generation.
  - `sign.c`: signing.
  - `verify.c`: verification.
  - `doublepath.c`: double-path and fast-commit related routines.
  - `test/test_sqisign2dpush.c`: integration test and timing report.
- `src/id2iso/ref/id2isox/id2iso.c`: ideal/isogeny conversion routines.
- `src/hd/ref/lvlx/`: two-dimensional isogeny and theta-chain routines.
- `src/ec/ref/`: elliptic-curve, basis, and one-dimensional isogeny routines.
- `src/quaternion/ref/`: quaternion algebra, ideals, lattices, and KLPT-related
  support code.
- `src/precomp/ref/lvl1`, `src/precomp/ref/lvl3`, `src/precomp/ref/lvl5`:
  precomputed constants for each parameter level.

The three tested parameter levels are `lvl1`, `lvl3`, and `lvl5`.

## Timing instrumentation

The timing report is printed by
`src/sqisign2dpush/ref/sqisign2dpushx/test/test_sqisign2dpush.c`.

The test repeats key generation, signing, and verification, then reports:

- average time and cycle statistics,
- category ratios for key generation,
- category ratios for signing,
- category ratios for verification.

The category ratios use the average time of the corresponding operation as the
denominator.

### Categories

- `Isogeny computations`
  - One-dimensional isogeny routines such as `ec_eval_even`, `ec_eval_three`,
    `isog_init_three`, and `isog_init_two`.
  - Two-dimensional isogeny chains such as `theta_chain_compute_and_eval` and
    `theta_chain_compute_and_eval_verify`.
- `EC/basis except isogeny`
  - Elliptic-curve and basis algorithms that are not counted as isogeny
    computation.
  - Examples: basis generation, basis changes, biscalar multiplication,
    pairings, dlogs, kernel setup, and matrix application to bases.
- `Quaternion algorithms`
  - Quaternion, ideal, and lattice algorithms.
  - Examples: `quat_*`, `represent_integer`, and `sample_response`.
  - Conversion routines between quaternion ideals and elliptic-curve data are
    split into quaternion, elliptic-curve, and other work when the code measures
    those parts separately.
- `Other`
  - Remaining measured time such as hashing, setup, final checks, and
    bookkeeping.

Fine-grained per-step timing printouts are intentionally disabled with `#if 0`
in `test_sqisign2dpush.c`; the measurements are still accumulated internally
for the category ratios.

## Build and run

From the repository root:

```sh
rm -rf build
cmake -S . -B build -DSQISIGN_BUILD_TYPE=ref -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

To build only the SQIsign2DPush tests:

```sh
cmake --build build --target sqisign_test_sqisign2dpush_lvl1
cmake --build build --target sqisign_test_sqisign2dpush_lvl3
cmake --build build --target sqisign_test_sqisign2dpush_lvl5
```

Run the tests from the repository root:

```sh
./build/src/sqisign2dpush/ref/lvl1/test/sqisign_test_sqisign2dpush_lvl1
./build/src/sqisign2dpush/ref/lvl3/test/sqisign_test_sqisign2dpush_lvl3
./build/src/sqisign2dpush/ref/lvl5/test/sqisign_test_sqisign2dpush_lvl5
```

