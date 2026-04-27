#include <iostream>
#include <cstdlib>
#include "mlp_mnist.h"

int main() {

    // ── Dummy weight arrays (all ones for smoke test) ──
    static data8_t  w0[INPUT_SIZE][HIDDEN1_SIZE];
    static data8_t  b0[HIDDEN1_SIZE];
    static data8_t  w1[HIDDEN1_SIZE][HIDDEN2_SIZE];
    static data8_t  b1[HIDDEN2_SIZE];
    static data8_t  w2[HIDDEN2_SIZE][OUTPUT_SIZE];
    static data8_t  b2[OUTPUT_SIZE];
    static data8_t  input[INPUT_SIZE];
    static data32_t output[OUTPUT_SIZE];

    // Fill with small values so accumulator doesn't overflow int32
    for (int i = 0; i < INPUT_SIZE;   i++) input[i] = 1;
    for (int i = 0; i < HIDDEN1_SIZE; i++) { b0[i] = 0; }
    for (int i = 0; i < HIDDEN2_SIZE; i++) { b1[i] = 0; }
    for (int i = 0; i < OUTPUT_SIZE;  i++) { b2[i] = 0; }

    for (int i = 0; i < INPUT_SIZE;   i++)
        for (int j = 0; j < HIDDEN1_SIZE; j++) w0[i][j] = 1;
    for (int i = 0; i < HIDDEN1_SIZE; i++)
        for (int j = 0; j < HIDDEN2_SIZE; j++) w1[i][j] = 1;
    for (int i = 0; i < HIDDEN2_SIZE; i++)
        for (int j = 0; j < OUTPUT_SIZE;  j++) w2[i][j] = 1;

    // ── Run the function ───────────────────────────────
    mlp_mnist(input, w0, b0, w1, b1, w2, b2, output);

    // ── Find predicted class (argmax) ──────────────────
    int predicted = 0;
    data32_t max_val = output[0];
    for (int i = 1; i < OUTPUT_SIZE; i++) {
        if (output[i] > max_val) {
            max_val = output[i];
            predicted = i;
        }
    }

    std::cout << "Output logits:" << std::endl;
    for (int i = 0; i < OUTPUT_SIZE; i++)
        std::cout << "  class " << i << ": " << output[i] << std::endl;
    std::cout << "Predicted digit: " << predicted << std::endl;

    // With all-ones weights and input, all logits should be equal
    // so the testbench passes if all outputs are identical
    bool pass = true;
    for (int i = 1; i < OUTPUT_SIZE; i++)
        if (output[i] != output[0]) pass = false;

    std::cout << (pass ? "TESTBENCH PASSED" : "TESTBENCH FAILED") << std::endl;
    return pass ? 0 : 1;
}
