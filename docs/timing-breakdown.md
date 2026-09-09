# Benchmark timing categories

The benchmark reports process CPU milliseconds from `clock()`. These are not
wall-clock latency measurements. Percentages use the CPU time of the entire
protocol call, including instrumentation. The separate `rdtsc` measurement is
in TSC ticks; it is not a count of core cycles at the current CPU frequency.

The categories use disjoint algorithm-level intervals:

| Category | Included work |
| --- | --- |
| isogeny | Isogeny-chain construction/evaluation (`ec_eval_*`, theta chains), including all point and field arithmetic performed inside those calls. |
| elliptic-curve | Point multiplication, basis generation, pairings/DLP, curve normalization and isomorphisms outside isogeny-chain intervals. |
| ideal/quaternion | Integer representation, ideals, lattices, quaternion arithmetic, action matrices and integer kernel coefficients. |
| other | Residual CPU time: hashing and its encoding/preparation, object management, unclassified glue code, and timing overhead. |

Do not add a whole composite operation to a category when its children already
record their categories. Shared ideal-to-kernel conversions split integer
coefficient work from point multiplication internally; keygen and signing use
the same split. Signature retries accumulate category times, including failed
sampling attempts. Phase totals (`ms_commit`, `ms_challenge`, `ms_response`) are
an alternative view of the work and are never added to the category totals.

Verification directly accumulates the same three categories in
`ms_ver_ec_isog`, `ms_ver_ec_non_isog`, and `ms_ver_quat`.
`ec_eval_three_prev()` is counted once in isogeny time. Kernel recovery and
theta setup are included in EC time; integer arithmetic selecting the even
kernel is included in quaternion time. Hashing and the final check remain in
`other`. Theta-chain time is part of isogeny time, not an additional category.

`other` is calculated once as total minus the three measured categories. It is
not independently measured and does not imply that every instruction has a
semantic classification. Negative residuals are not clamped. The benchmark
fails if overlap exceeds four `clock()` units plus 0.001% of total time (float
accumulator rounding allowance). This accounting check detects overlap, but
cannot prove that an individual interval has the correct semantic label.

Use a Release build for performance comparisons. Debug assertions and sanitizer
instrumentation change the measured work. Small intervals are affected by clock
resolution and measurement overhead; these are operation-level profiles, not
precise counts of primitive field operations.

After building the benchmark targets, run:

```sh
python3 scripts/test_timing_breakdown.py --build build --iterations 100
```

This runs all three levels, requires successful keygen/sign/verify, and checks
the reported sums. The existing `sqisign_test_sqisign2dpush_lvl*` round-trip tests
exercise protocol calls with null timing pointers.
