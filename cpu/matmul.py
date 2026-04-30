import numpy as np


BATCH_SIZE = 32
INPUT_SIZE = 64
OUTPUT_SIZE = 64
DTYPE = np.int32


def run_matmul(batch_size=BATCH_SIZE):
    rng = np.random.default_rng(42)
    activations = rng.integers(-8, 8, size=(batch_size, INPUT_SIZE), dtype=DTYPE)
    weights = rng.integers(-8, 8, size=(INPUT_SIZE, OUTPUT_SIZE), dtype=DTYPE)

    return activations @ weights


if __name__ == "__main__":
    output = run_matmul()
    print(f"shape={output.shape} checksum={int(output.sum())}")
