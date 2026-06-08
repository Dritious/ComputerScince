#include "square_equation.c"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void assert_double_eq(double actual, double expected, double tol,
                             const char *test_name) {
  double diff = fabs(actual - expected);
  if (diff > tol) {
    fprintf(stderr, "%s: FAILED\n", test_name);
    fprintf(stderr, "expected: %.15g\n", expected);
    fprintf(stderr, "actual:   %.15g\n", actual);
    fprintf(stderr, "diff:     %e (tol: %e)\n", diff, tol);
    assert(!"Test failed");
  }
}

static void assert_count(int actual, int expected, const char *test_name) {
  if (actual != expected) {
    fprintf(stderr, "%s: FAILED\n", test_name);
    fprintf(stderr, "expected count: %d\n", expected);
    fprintf(stderr, "actual count:   %d\n", actual);
    assert(!"Test failed");
  }
}

int main() {
  double roots[2];
  int n;

  n = solve_square(0.0, 2.0, 1.0, roots);
  assert_count(n, -1, "a == 0: вычисление невозможно");

  n = solve_square(1.0, 0.0, -1.0, roots);
  assert_count(n, 2, "положительный дискриминант: количество");
  assert_double_eq(roots[0], -1.0, 1e-12, "положительный дискриминант: x1");
  assert_double_eq(roots[1], 1.0, 1e-12, "положительный дискриминант: x2");

  n = solve_square(1.0, 0.0, 0.0, roots);
  assert_count(n, 1, "нулевой дискриминант: количество");
  assert_double_eq(roots[0], 0.0, 1e-12, "нулевой дискриминант: x1");

  n = solve_square(1.0, 0.0, 1.0, roots);
  assert_count(n, 0, "отрицательный дискриминант: корней нет");

  n = solve_square(1.0, 0.0, -1e-7, roots);
  assert_count(n, 2, "малый положительный D: количество");
  assert_double_eq(roots[0], -3e-4, 1e-4, "малый положительный D: x1");
  assert_double_eq(roots[1], 3e-4, 1e-4, "малый положительный D: x2");

  n = solve_square(1.0, -1e10, -1.0, roots);
  assert_count(n, 2, "большой b: количество");
  assert_double_eq(roots[0], -1e-10, 1e-11, "большой b: малый корень");
  assert_double_eq(roots[1], 1e10, 1e-1, "большой b: большой корень");

  n = solve_square(1.0, 0.0, -1e-8, roots);
  assert_count(n, 1, "околонулевой D: количество");
  assert_double_eq(roots[0], 0.0, 1e-7, "околонулевой D: x1");

  printf("Все тесты square_equation пройдены.\n");
  return 0;
}
