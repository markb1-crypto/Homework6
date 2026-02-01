#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

/* Prints a fixed-point number (raw) in decimal, using q fractional bits. */
void    print_fixed(int16_t raw, int16_t q);

/* Fixed-point addition (same q for both inputs). */
int16_t add_fixed(int16_t a, int16_t b);

/* Fixed-point subtraction a - b (same q for both inputs). */
int16_t subtract_fixed(int16_t a, int16_t b);

/* Fixed-point multiplication (a*b)>>q (same q for both inputs). */
int16_t multiply_fixed(int16_t a, int16_t b, int16_t q);

/* Evaluate y = a*x^2 - b*x + c in fixed-point and print the required message. */
void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x,
                                            int16_t a,
                                            int16_t b,
                                            int16_t c,
                                            int16_t q);

#endif // FIXED_POINT_H
