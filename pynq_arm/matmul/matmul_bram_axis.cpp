#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

#define MAX_N 256
#define PAR_FACTOR 16

typedef ap_int<16> data_t;
typedef ap_int<32> acc_t;

typedef ap_axis<32, 0, 0, 0> axis_t;

void matmul_bram_axis(
    hls::stream<axis_t>& input_stream,
    hls::stream<axis_t>& output_stream,
    int size
) {
#pragma HLS INTERFACE axis port=input_stream
#pragma HLS INTERFACE axis port=output_stream

#pragma HLS INTERFACE s_axilite port=size bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    static data_t A[MAX_N][MAX_N];
    static data_t B[MAX_N][MAX_N];

#pragma HLS BIND_STORAGE variable=A type=ram_2p impl=bram
#pragma HLS BIND_STORAGE variable=B type=ram_2p impl=bram

#pragma HLS ARRAY_PARTITION variable=A cyclic factor=PAR_FACTOR dim=2
#pragma HLS ARRAY_PARTITION variable=B cyclic factor=PAR_FACTOR dim=1

    if (size > MAX_N) {
        size = MAX_N;
    }

    if (size <= 0) {
        return;
    }

    axis_t in_word;

READ_A_ROW:
    for (int i = 0; i < MAX_N; i++) {
    READ_A_COL:
        for (int j = 0; j < MAX_N; j++) {
#pragma HLS PIPELINE II=1
            if (i < size && j < size) {
                in_word = input_stream.read();
                A[i][j] = (data_t)in_word.data.range(15, 0);
            }
        }
    }

READ_B_ROW:
    for (int i = 0; i < MAX_N; i++) {
    READ_B_COL:
        for (int j = 0; j < MAX_N; j++) {
#pragma HLS PIPELINE II=1
            if (i < size && j < size) {
                in_word = input_stream.read();
                B[i][j] = (data_t)in_word.data.range(15, 0);
            }
        }
    }

COMPUTE_ROW:
    for (int i = 0; i < MAX_N; i++) {
    COMPUTE_COL:
        for (int j = 0; j < MAX_N; j++) {

            if (i < size && j < size) {

                acc_t partial[PAR_FACTOR];
#pragma HLS ARRAY_PARTITION variable=partial complete dim=1

            INIT_PARTIAL:
                for (int p = 0; p < PAR_FACTOR; p++) {
#pragma HLS UNROLL
                    partial[p] = 0;
                }

            COMPUTE_K:
                for (int kk = 0; kk < MAX_N; kk += PAR_FACTOR) {
#pragma HLS PIPELINE II=1

                MULT_PARALLEL:
                    for (int p = 0; p < PAR_FACTOR; p++) {
#pragma HLS UNROLL
                        int k = kk + p;

                        if (k < size) {
                            partial[p] += (acc_t)A[i][k] * (acc_t)B[k][j];
                        }
                    }
                }

                acc_t sum = 0;

            REDUCE_SUM:
                for (int p = 0; p < PAR_FACTOR; p++) {
#pragma HLS UNROLL
                    sum += partial[p];
                }

                axis_t out_word;
                out_word.data = sum;
                out_word.keep = -1;
                out_word.strb = -1;
                out_word.user = 0;
                out_word.id   = 0;
                out_word.dest = 0;
                out_word.last = (i == size - 1 && j == size - 1) ? 1 : 0;

                output_stream.write(out_word);
            }
        }
    }
}
