#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fixed_point.h"

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: %s <x_raw> <a_raw> <b_raw> <c_raw> <q>\n", argv[0]);
        printf("All inputs must be integers. (x/a/b/c/q are int16 raw fixed-point values)\n");
        return 0;
    }

    int16_t x = (int16_t)atoi(argv[1]);
    int16_t a = (int16_t)atoi(argv[2]);
    int16_t b = (int16_t)atoi(argv[3]);
    int16_t c = (int16_t)atoi(argv[4]);
    int16_t q = (int16_t)atoi(argv[5]);

    eval_poly_ax2_minus_bx_plus_c_fixed(x, a, b, c, q);
    return 0;
}
