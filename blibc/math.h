#ifndef MATH_H
#define MATH_H

/* Bold C Library - math operations
 * Freestanding, integer/fixed-friendly. No libm dependency.
 */

/* Absolute value. */
int abs(int x);

/* Smaller of two values. */
int min(int a, int b);

/* Larger of two values. */
int max(int a, int b);

/* Integer power: base^exp (exp >= 0). */
long pow(int base, unsigned int exp);

/* Integer square root (floor of sqrt(x)), via Newton's method. */
unsigned int sqrt(unsigned int x);

/* Greatest common divisor. */
int gcd(int a, int b);

#endif /* BLIBC_MATH_H */
