#ifndef MLP_MNIST_H
#define MLP_MNIST_H

#include "ap_int.h"
#include "hls_stream.h"

// Network dimensions — match your trained model exactly
#define INPUT_SIZE   784
#define HIDDEN1_SIZE 256
#define HIDDEN2_SIZE 128
#define OUTPUT_SIZE  10

// Data types
// int8  for weights and input pixels (quantized)
// int32 for accumulators (prevents overflow during MAC)
typedef ap_int<8>  data8_t;
typedef ap_int<32> data32_t;

// Top-level function declaration
void mlp_mnist(
    data8_t  input[INPUT_SIZE],          // flattened 28x28 image, already quantized
    data8_t  w0[INPUT_SIZE][HIDDEN1_SIZE],
    data8_t  b0[HIDDEN1_SIZE],
    data8_t  w1[HIDDEN1_SIZE][HIDDEN2_SIZE],
    data8_t  b1[HIDDEN2_SIZE],
    data8_t  w2[HIDDEN2_SIZE][OUTPUT_SIZE],
    data8_t  b2[OUTPUT_SIZE],
    data32_t output[OUTPUT_SIZE]         // raw logits — take argmax in Python
);

#endif
