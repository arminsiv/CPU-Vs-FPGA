#include "mlp_mnist.h"

// ─────────────────────────────────────────────
// ReLU activation — clips int32 accumulator
// ─────────────────────────────────────────────
static data32_t relu(data32_t x) {
#pragma HLS INLINE
    return (x > 0) ? x : (data32_t)0;
}

// ─────────────────────────────────────────────
// Top-level function
// All arrays mapped to AXI-Lite (s_axilite)
// via the INTERFACE pragmas below
// ─────────────────────────────────────────────
void mlp_mnist(
    data8_t  input[INPUT_SIZE],
    data8_t  w0[INPUT_SIZE][HIDDEN1_SIZE],
    data8_t  b0[HIDDEN1_SIZE],
    data8_t  w1[HIDDEN1_SIZE][HIDDEN2_SIZE],
    data8_t  b1[HIDDEN2_SIZE],
    data8_t  w2[HIDDEN2_SIZE][OUTPUT_SIZE],
    data8_t  b2[OUTPUT_SIZE],
    data32_t output[OUTPUT_SIZE]
) {
    // ── Interface pragmas ──────────────────────
#pragma HLS INTERFACE s_axilite port=return      bundle=CTRL
#pragma HLS INTERFACE s_axilite port=input       bundle=CTRL
#pragma HLS INTERFACE s_axilite port=w0          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=b0          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=w1          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=b1          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=w2          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=b2          bundle=CTRL
#pragma HLS INTERFACE s_axilite port=output      bundle=CTRL

    // ── Intermediate activation buffers ───────
    // These live in BRAM on the FPGA
    data32_t hidden1[HIDDEN1_SIZE];
    data32_t hidden2[HIDDEN2_SIZE];

#pragma HLS ARRAY_PARTITION variable=hidden1 complete
#pragma HLS ARRAY_PARTITION variable=hidden2 complete

    // ══════════════════════════════════════════
    // LAYER 0: input (784) → hidden1 (256)
    // Y = W0^T · X + b0,  then ReLU
    // ══════════════════════════════════════════
    L0_out: for (int j = 0; j < HIDDEN1_SIZE; j++) {
#pragma HLS PIPELINE II=1
        data32_t acc = (data32_t)b0[j];   // start with bias
        L0_in: for (int i = 0; i < INPUT_SIZE; i++) {
            acc += (data32_t)w0[i][j] * (data32_t)input[i];
        }
        hidden1[j] = relu(acc);
    }

    // ══════════════════════════════════════════
    // LAYER 1: hidden1 (256) → hidden2 (128)
    // Y = W1^T · hidden1 + b1, then ReLU
    // ══════════════════════════════════════════
    L1_out: for (int j = 0; j < HIDDEN2_SIZE; j++) {
#pragma HLS PIPELINE II=1
        data32_t acc = (data32_t)b1[j];
        L1_in: for (int i = 0; i < HIDDEN1_SIZE; i++) {
            acc += (data32_t)w1[i][j] * (data32_t)hidden1[i];
        }
        hidden2[j] = relu(acc);
    }

    // ══════════════════════════════════════════
    // LAYER 2: hidden2 (128) → output (10)
    // Y = W2^T · hidden2 + b2
    // No activation — raw logits for argmax
    // ══════════════════════════════════════════
    L2_out: for (int j = 0; j < OUTPUT_SIZE; j++) {
#pragma HLS PIPELINE II=1
        data32_t acc = (data32_t)b2[j];
        L2_in: for (int i = 0; i < HIDDEN2_SIZE; i++) {
            acc += (data32_t)w2[i][j] * (data32_t)hidden2[i];
        }
        output[j] = acc;   // no ReLU on final layer
    }
}
