#include "fixed_point.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

void print_fixed(int16_t raw, int16_t q) {
    /* Print raw / 2^q with exactly 6 decimals, truncating toward zero. */
    int64_t denom = (int64_t)1 << q;
    int64_t scaled = (int64_t)raw * 1000000LL;
    int64_t scaled_val = scaled / denom; /* truncates toward zero */

    if (scaled_val < 0) {
        putchar('-');
        scaled_val = -scaled_val;
    }

    int64_t int_part = scaled_val / 1000000LL;
    int64_t frac_part = scaled_val % 1000000LL;

    printf("%" PRId64 ".%06" PRId64, int_part, frac_part);
}

int16_t add_fixed(int16_t a, int16_t b) {
    return (int16_t)(a + b);
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    return (int16_t)(a - b);
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    /* raw_out = (raw_a * raw_b) / 2^q  (truncate toward zero) */
    int64_t prod = (int64_t)a * (int64_t)b;
    int64_t denom = (int64_t)1 << q;
    int64_t out = prod / denom;
    return (int16_t)out;
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x, int16_t a, int16_t b, int16_t c, int16_t q) {
    /* y = a*x^2 - b*x + c */
    int16_t x2 = multiply_fixed(x, x, q);
    int16_t ax2 = multiply_fixed(a, x2, q);
    int16_t bx = multiply_fixed(b, x, q);
    int16_t ax2_minus_bx = subtract_fixed(ax2, bx);
    int16_t y = add_fixed(ax2_minus_bx, c);

    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    print_fixed(y, q);
    printf("\n");
}
