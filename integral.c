#include <math.h>
#include <stdio.h>

double raw_integral(double (*function)(double), double start, double end,
                    int num_of_steps) {
  double area = 0.5 * (function(start) + function(end));
  double step = (end - start) / num_of_steps;

  double compensation = 0.0;
  double x, y, t;
  for (int i = 1; i < num_of_steps; i++) {
    x = start + i * step;
    y = function(x) - compensation;
    t = area + y;
    compensation = (t - area) - y;
    area = t;
  }
  return area * step;
}
double romberg_integral(double (*function)(double), double start, double end) {
  double R[10][10];
  double eps = 1e-10;
  int n = 1;

  R[0][0] = raw_integral(function, start, end, n);

  for (int i = 1; i < 10; i++) {
    n *= 2;

    R[i][0] = raw_integral(function, start, end, n);

    double factor = 4.0;
    for (int j = 1; j <= i; j++) {
      R[i][j] = (factor * R[i][j - 1] - R[i - 1][j - 1]) / (factor - 1.0);
      factor *= 4.0;
    }

    if (fabs(R[i][i] - R[i - 1][i - 1]) < eps) {
      return R[i][i];
    }
  }

  return R[9][9];
}
