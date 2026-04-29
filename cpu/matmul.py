import json
from pathlib import Path
import time

import numpy as np


BATCH_SIZES = (1, 32, 512, 2048)
INPUT_SIZE = 64
OUTPUT_SIZE = 64
DTYPE = np.int32


def run_matmul(batch_size):
    rng = np.random.default_rng(42)
    activations = rng.integers(-8, 8, size=(batch_size, INPUT_SIZE), dtype=DTYPE)
    weights = rng.integers(-8, 8, size=(INPUT_SIZE, OUTPUT_SIZE), dtype=DTYPE)

    start = time.perf_counter()
    output = activations @ weights
    elapsed = time.perf_counter() - start

    return output, elapsed


if __name__ == "__main__":
    results = {
        "cpu": {
            "input_size": INPUT_SIZE,
            "output_size": OUTPUT_SIZE,
            "dtype": "int32",
            "batches": [],
        }
    }

    total_start = time.perf_counter()

    for batch_size in BATCH_SIZES:
        output, elapsed = run_matmul(batch_size)
        checksum = int(output.sum())
        results["cpu"]["batches"].append(
            {
                "batch": batch_size,
                "shape": list(output.shape),
                "execution_time": elapsed,
                "checksum": checksum,
            }
        )
        print(
            f"batch={batch_size:<3} shape={output.shape} "
            f"time={elapsed:.6f}s checksum={checksum}"
        )

    results["cpu"]["execution_time"] = time.perf_counter() - total_start

    results_dir = Path("results")
    results_dir.mkdir(exist_ok=True)
    output_path = results_dir / "matmul_results.json"
    with output_path.open("w") as f:
        json.dump(results, f, indent=4)

    print(f"Results saved to {output_path}")
