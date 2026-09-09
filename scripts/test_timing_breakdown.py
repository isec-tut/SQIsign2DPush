"""Run real protocol benchmarks and check their printed timing accounting."""

import argparse
import math
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=Path("build"))
    parser.add_argument("--iterations", type=int, default=100)
    parser.add_argument("--levels", nargs="+", choices=["1", "3", "5"],
                        default=["1", "3", "5"])
    args = parser.parse_args()
    if args.iterations <= 0:
        parser.error("--iterations must be positive")

    for level in args.levels:
        binary = (args.build / "src/sqisign2dpush/ref" / f"lvl{level}" /
                  "test" / f"sqisign_bench_sqisign2dpush_lvl{level}")
        run = subprocess.run([str(binary.resolve()), str(args.iterations)],
                             capture_output=True, text=True, check=True)
        print(f"level {level}\n{run.stdout}", flush=True)
        if run.stderr:
            raise RuntimeError(run.stderr)
        totals = re.search(
            r"average time: keygen ([\d.]+) ms, sign ([\d.]+) ms, "
            r"verify ([\d.]+) ms", run.stdout)
        if totals is None or "verify completed" not in run.stdout:
            raise RuntimeError("Missing completed benchmark results")
        for phase, total in zip(["keygen", "sign", "verify"], totals.groups()):
            line = re.search(rf"^  {phase}\s+isogeny .*", run.stdout, re.M)
            if line is None:
                raise RuntimeError(f"Missing {phase} breakdown")
            values = re.findall(r"(-?[\d.]+) ms \((-?[\d.]+)%\)", line[0])
            if len(values) != 4:
                raise RuntimeError(f"Incomplete breakdown: {line[0]}")
            times, percentages = zip(*[(float(t), float(p)) for t, p in values])
            if not all(math.isfinite(v) for v in (*times, *percentages)):
                raise RuntimeError(f"Non-finite result: {line[0]}")
            # Allow only the precision lost by printing three/one decimals.
            if min(times) < -0.005 or abs(sum(times) - float(total)) > 0.003:
                raise RuntimeError(f"Unbalanced times: {line[0]}")
            if abs(sum(percentages) - 100.0) > 0.21:
                raise RuntimeError(f"Unbalanced percentages: {line[0]}")
        print(f"level {level}: timing accounting passed", flush=True)


if __name__ == "__main__":
    main()
