#define MAX_BATCH 2048
#define INPUT_SIZE 64
#define OUTPUT_SIZE 64
#define UNROLL_FACTOR 8

typedef int data_t;

void matmul_parallel(const data_t *activations, const data_t *weights, data_t *output, int batch_size) {
#pragma HLS INTERFACE m_axi port=activations offset=slave bundle=gmem0 depth=131072
#pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem1 depth=4096
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2 depth=131072
#pragma HLS INTERFACE s_axilite port=activations bundle=CTRL
#pragma HLS INTERFACE s_axilite port=weights bundle=CTRL
#pragma HLS INTERFACE s_axilite port=output bundle=CTRL
#pragma HLS INTERFACE s_axilite port=batch_size bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    data_t weight_local[INPUT_SIZE][OUTPUT_SIZE];
    data_t activation_row[INPUT_SIZE];
    data_t partial[UNROLL_FACTOR];

#pragma HLS ARRAY_PARTITION variable=weight_local cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=activation_row cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=partial complete dim=1

    LoadWeightsK: for (int k = 0; k < INPUT_SIZE; k++) {
        LoadWeightsJ: for (int j = 0; j < OUTPUT_SIZE; j++) {
#pragma HLS PIPELINE II=1
            weight_local[k][j] = weights[k * OUTPUT_SIZE + j];
        }
    }

    Row: for (int i = 0; i < batch_size; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=2048
        LoadActivation: for (int k = 0; k < INPUT_SIZE; k++) {
#pragma HLS PIPELINE II=1
            activation_row[k] = activations[i * INPUT_SIZE + k];
        }

        Col: for (int j = 0; j < OUTPUT_SIZE; j++) {
            InitPartial: for (int u = 0; u < UNROLL_FACTOR; u++) {
#pragma HLS UNROLL
                partial[u] = 0;
            }

            DotGroup: for (int k = 0; k < INPUT_SIZE; k += UNROLL_FACTOR) {
#pragma HLS PIPELINE II=1
                DotLane: for (int u = 0; u < UNROLL_FACTOR; u++) {
#pragma HLS UNROLL
                    partial[u] += activation_row[k + u] * weight_local[k + u][j];
                }
            }

            data_t acc = 0;
            Reduce: for (int u = 0; u < UNROLL_FACTOR; u++) {
#pragma HLS UNROLL
                acc += partial[u];
            }

            output[i * OUTPUT_SIZE + j] = acc;
        }
    }
}
