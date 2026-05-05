#include <iostream>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

#define MAX_N 256

typedef ap_axis<32, 0, 0, 0> axis_t;

void matmul_bram_axis(
    hls::stream<axis_t>& input_stream,
    hls::stream<axis_t>& output_stream,
    int size
);

static axis_t make_input_word(short value, bool last = false) {
    axis_t word;
    word.data = value;
    word.keep = -1;
    word.strb = -1;
    word.user = 0;
    word.id   = 0;
    word.dest = 0;
    word.last = last ? 1 : 0;
    return word;
}

static int golden_value(short A[MAX_N][MAX_N], short B[MAX_N][MAX_N], int N, int row, int col) {
    int sum = 0;

    for (int k = 0; k < N; k++) {
        sum += (int)A[row][k] * (int)B[k][col];
    }

    return sum;
}

static void fill_test_matrices(short A[MAX_N][MAX_N], short B[MAX_N][MAX_N], int N, int test_id) {
    /*
        Fill matrices with deterministic small values.

        Values are intentionally small so 32-bit accumulation does not overflow.
        This is suitable for test benches and image-style data.
    */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {

            if (test_id == 0) {
                A[i][j] = (short)((i + j) % 8);
                B[i][j] = (short)((i == j) ? 1 : 0);  // identity-like
            } else if (test_id == 1) {
                A[i][j] = (short)((i * 3 + j * 2) % 11 - 5);
                B[i][j] = (short)((i * 2 - j * 4) % 9);
            } else {
                A[i][j] = (short)(((i + 1) * (j + 2)) % 13 - 6);
                B[i][j] = (short)(((i + 3) - (j + 1)) % 7);
            }
        }
    }
}

static bool run_one_test(int N, int test_id) {
    static short A[MAX_N][MAX_N];
    static short B[MAX_N][MAX_N];

    hls::stream<axis_t> input_stream;
    hls::stream<axis_t> output_stream;

    fill_test_matrices(A, B, N, test_id);

    /*
        Send A first.
    */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            input_stream.write(make_input_word(A[i][j]));
        }
    }

    /*
        Send B second.
    */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            bool last = (i == N - 1 && j == N - 1);
            input_stream.write(make_input_word(B[i][j], last));
        }
    }

    /*
        Run DUT.
    */
    matmul_bram_axis(input_stream, output_stream, N);

    /*
        Verify all C outputs.
    */
    bool pass = true;
    int output_count = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {

            if (output_stream.empty()) {
                std::cout << "ERROR: output stream ended early\n";
                return false;
            }

            axis_t out_word = output_stream.read();
            int got = out_word.data.to_int();
            int exp = golden_value(A, B, N, i, j);

            output_count++;

            if (got != exp) {
                std::cout << "ERROR: mismatch at N=" << N
                          << ", test_id=" << test_id
                          << ", C[" << i << "][" << j << "]"
                          << ", got=" << got
                          << ", expected=" << exp << "\n";
                pass = false;
            }

            bool should_be_last = (i == N - 1 && j == N - 1);

            if ((bool)out_word.last != should_be_last) {
                std::cout << "ERROR: TLAST wrong at N=" << N
                          << ", test_id=" << test_id
                          << ", C[" << i << "][" << j << "]\n";
                pass = false;
            }
        }
    }

    if (!output_stream.empty()) {
        std::cout << "ERROR: output stream has extra data\n";
        pass = false;
    }

    if (output_count != N * N) {
        std::cout << "ERROR: wrong output count. Got "
                  << output_count << ", expected " << N * N << "\n";
        pass = false;
    }

    if (pass) {
        std::cout << "PASS: N=" << N << ", test_id=" << test_id << "\n";
    } else {
        std::cout << "FAIL: N=" << N << ", test_id=" << test_id << "\n";
    }

    return pass;
}

int main() {
    bool all_pass = true;

    /*
        These sizes are chosen for C simulation speed.

        Full 256x256 C simulation may take a long time because software
        simulation does not run like real FPGA hardware.
    */
    int sizes[] = {
        1,
        2,
        3,
        4,
        8,
        16,
        17,
        31,
        32
    };

    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];

        for (int test_id = 0; test_id < 3; test_id++) {
            bool pass = run_one_test(N, test_id);

            if (!pass) {
                all_pass = false;
            }
        }
    }

    if (all_pass) {
        std::cout << "GOOD TEST BENCH PASSED\n";
        return 0;
    } else {
        std::cout << "GOOD TEST BENCH FAILED\n";
        return 1;
    }
}