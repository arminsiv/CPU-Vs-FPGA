#include <iostream>

#define INPUT_SIZE 64
#define OUTPUT_SIZE 64
#define MAX_TEST_BATCH 16

typedef int data_t;

void matmul_parallel(const data_t *activations,
                     const data_t *weights,
                     data_t *output,
                     int batch_size);

static void fill_inputs(data_t activations[MAX_TEST_BATCH * INPUT_SIZE],
                        data_t weights[INPUT_SIZE * OUTPUT_SIZE],
                        int batch_size) {
    for (int i = 0; i < batch_size; i++) {
        for (int k = 0; k < INPUT_SIZE; k++) {
            activations[i * INPUT_SIZE + k] = ((i * 3 + k * 5) % 17) - 8;
        }
    }

    for (int k = 0; k < INPUT_SIZE; k++) {
        for (int j = 0; j < OUTPUT_SIZE; j++) {
            weights[k * OUTPUT_SIZE + j] = ((k * 7 + j * 11) % 13) - 6;
        }
    }
}

static void reference_matmul(const data_t activations[MAX_TEST_BATCH * INPUT_SIZE],
                             const data_t weights[INPUT_SIZE * OUTPUT_SIZE],
                             data_t expected[MAX_TEST_BATCH * OUTPUT_SIZE],
                             int batch_size) {
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < OUTPUT_SIZE; j++) {
            data_t acc = 0;
            for (int k = 0; k < INPUT_SIZE; k++) {
                acc += activations[i * INPUT_SIZE + k] * weights[k * OUTPUT_SIZE + j];
            }
            expected[i * OUTPUT_SIZE + j] = acc;
        }
    }
}

static bool check_output(const data_t output[MAX_TEST_BATCH * OUTPUT_SIZE],
                         const data_t expected[MAX_TEST_BATCH * OUTPUT_SIZE],
                         int batch_size) {
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < OUTPUT_SIZE; j++) {
            int index = i * OUTPUT_SIZE + j;
            if (output[index] != expected[index]) {
                std::cout << "Mismatch at row " << i
                          << ", col " << j
                          << ": got " << output[index]
                          << ", expected " << expected[index] << std::endl;
                return false;
            }
        }
    }
    return true;
}

static bool run_case(int batch_size) {
    data_t activations[MAX_TEST_BATCH * INPUT_SIZE] = {};
    data_t weights[INPUT_SIZE * OUTPUT_SIZE] = {};
    data_t output[MAX_TEST_BATCH * OUTPUT_SIZE] = {};
    data_t expected[MAX_TEST_BATCH * OUTPUT_SIZE] = {};

    fill_inputs(activations, weights, batch_size);
    reference_matmul(activations, weights, expected, batch_size);
    matmul_parallel(activations, weights, output, batch_size);

    if (!check_output(output, expected, batch_size)) {
        std::cout << "Test failed for batch_size=" << batch_size << std::endl;
        return false;
    }

    std::cout << "Test passed for batch_size=" << batch_size << std::endl;
    return true;
}

int main() {
    const int test_batches[] = {1, 3, MAX_TEST_BATCH};
    const int num_tests = sizeof(test_batches) / sizeof(test_batches[0]);

    for (int t = 0; t < num_tests; t++) {
        if (!run_case(test_batches[t])) {
            return 1;
        }
    }

    std::cout << "All matmul_parallel tests passed." << std::endl;
    return 0;
}
