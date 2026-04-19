#include "integral.c"
#include <assert.h>
#include <math.h>

// Тестовые функции
double linear(double x) { return 2.0 * x + 1.0; }
double quadratic(double x) { return x * x; }
double cubic(double x) { return x * x * x; }
double sin_func(double x) { return sin(x); }
double exp_func(double x) { return exp(x); }
double peak_func(double x) {
  return 1.0 / (1.0 + 25.0 * x * x);
} // Функция Рунге
double oscillatory(double x) { return sin(100.0 * x); } // Быстрые осцилляции

// Вспомогательная функция для проверки с допуском
static void assert_double_eq(double actual, double expected,
                             const char *test_name) {
  double diff = fabs(actual - expected);
  double tol = 1e-10;
  if (diff > tol) {
    fprintf(stderr, "%s: FAILED\n", test_name);
    fprintf(stderr, "expected: %.15f\n", expected);
    fprintf(stderr, "actual:   %.15f\n", actual);
    fprintf(stderr, "diff:     %e (tol: %e)\n", diff, tol);
    assert(!"Test failed");
  }
}

int main() {

  double res = romberg_integral(linear, 0.0, 1.0);
  assert_double_eq(res, 2.0, "linear [0,1]");

  res = romberg_integral(quadratic, -2.0, 2.0);
  assert_double_eq(res, 16.0 / 3.0, "quadratic [-2,2]");

  res = romberg_integral(cubic, 0.0, 1.0);
  assert_double_eq(res, 0.25, "cubic [0,1]");

  res = romberg_integral(sin_func, 0.0, M_PI);
  assert_double_eq(res, 2.0, "sin [0,π]");

  res = romberg_integral(exp_func, 0.0, 1.0);
  assert_double_eq(res, exp(1.0) - 1.0, "exp [0,1]");

  res = romberg_integral(quadratic, -1.0, 1.0);
  assert_double_eq(res, 2.0 / 3.0, "quadratic symmetric [-1,1]");

  res = romberg_integral(peak_func, -1.0, 1.0);
  assert_double_eq(res, 0.549360306778, "Runge function [-1,1]");

  res = romberg_integral(exp_func, 0.0, 1e-6);
  double expected = 1e-6 + 0.5e-12;
  assert_double_eq(res, expected, "tiny interval");

  res = romberg_integral(exp_func, 0.0, 10.0);
  assert_double_eq(res, exp(10.0) - 1.0, "large interval");

  res = romberg_integral(oscillatory, 0.0, 2.0 * M_PI);
  assert_double_eq(res, 0.0, "oscillatory [0,2π]");

  res = romberg_integral(quadratic, 1.0, 1.0);
  assert_double_eq(res, 0.0, "zero interval");

  res = romberg_integral(quadratic, 0.0, 1.0);
  assert_double_eq(res, 1.0 / 3.0, "coarse eps");

  res = romberg_integral(quadratic, 0.0, 1.0);
  assert_double_eq(res, 1.0 / 3.0, "fine eps");

  return 0;
}
