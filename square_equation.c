#include <math.h>

#define EPS 1e-7

int solve_square(double a, double b, double c, double *roots) {
  if (a == 0.0) {
    return -1;
  }

  double d = b * b - 4.0 * a * c;

  if (d < -EPS) {
    return 0;
  }

  if (d <= EPS) {
    roots[0] = -b / (2.0 * a);
    return 1;
  }

  double sqrt_d = sqrt(d);
  double sign = (b >= 0.0) ? 1.0 : -1.0;
  double q = -0.5 * (b + sign * sqrt_d);

  double x1 = q / a;
  double x2 = c / q;

  if (x1 <= x2) {
    roots[0] = x1;
    roots[1] = x2;
  } else {
    roots[0] = x2;
    roots[1] = x1;
  }
  return 2;
}
