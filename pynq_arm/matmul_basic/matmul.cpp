#define MAX_BATCH 2048
#define INPUT_SIZE 64
#define OUTPUT_SIZE 64

typedef int data_t;

void matmul(const data_t *activations, const data_t *weights, data_t *output, int batch_size) {
#pragma HLS INTERFACE m_axi port=activations offset=slave bundle=gmem0 depth=131072
#pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem1 depth=4096
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2 depth=131072
#pragma HLS INTERFACE s_axilite port=activations bundle=CTRL
#pragma HLS INTERFACE s_axilite port=weights bundle=CTRL
#pragma HLS INTERFACE s_axilite port=output bundle=CTRL
#pragma HLS INTERFACE s_axilite port=batch_size bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    Row: for (int i = 0; i < batch_size; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=2048
        Col: for (int j = 0; j < OUTPUT_SIZE; j++) {
            data_t acc = 0;
            Dot: for (int k = 0; k < INPUT_SIZE; k++) {
#pragma HLS PIPELINE II=1
                acc += activations[i * INPUT_SIZE + k] * weights[k * OUTPUT_SIZE + j];
            }
            output[i * OUTPUT_SIZE + j] = acc;
        }
    }
}
