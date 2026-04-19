#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stack.c"

void test_init_stack() {

  Stack *stack = init_stack();
  assert(stack != NULL);
  assert(stack->data != NULL);
  assert(stack->top == -1);
  assert(stack->capacity == 4);

  free_stack(stack);
}

void test_push_single_element() {

  Stack *stack = init_stack();
  push(stack, 42);

  assert(stack->top == 0);
  assert(stack->data[0] == 42);

  free_stack(stack);
}

void test_push_multiple_elements() {

  Stack *stack = init_stack();

  for (int i = 0; i < 3; i++) {
    push(stack, i * 10);
  }

  assert(stack->top == 2);
  assert(stack->data[0] == 0);
  assert(stack->data[1] == 10);
  assert(stack->data[2] == 20);

  free_stack(stack);
}

void test_pop_single_element() {

  Stack *stack = init_stack();
  push(stack, 100);

  int value = pop(stack);

  assert(value == 100);
  assert(stack->top == -1);
  assert(is_empty(stack));

  free_stack(stack);
}

void test_pop_multiple_elements() {

  Stack *stack = init_stack();
  push(stack, 1);
  push(stack, 2);
  push(stack, 3);

  assert(pop(stack) == 3);
  assert(pop(stack) == 2);
  assert(pop(stack) == 1);
  assert(is_empty(stack));

  free_stack(stack);
}

void test_is_empty() {

  Stack *stack = init_stack();

  assert(is_empty(stack));

  push(stack, 10);
  assert(!is_empty(stack));

  pop(stack);
  assert(is_empty(stack));

  free_stack(stack);
}

void test_resize_when_full() {

  Stack *stack = init_stack(); // capacity = 4

  for (int i = 0; i < 4; i++) {
    push(stack, i);
  }

  assert(stack->top == 3);
  assert(stack->capacity == 4);

  push(stack, 4);

  assert(stack->top == 4);
  assert(stack->capacity == 8);
  assert(stack->data[4] == 4);

  assert(stack->data[0] == 0);
  assert(stack->data[1] == 1);
  assert(stack->data[2] == 2);
  assert(stack->data[3] == 3);

  free_stack(stack);
}

void test_multiple_resizes() {

  Stack *stack = init_stack();
  int expected_capacity = 4;

  for (int i = 0; i < 20; i++) {
    push(stack, i);

    if (i == 4)
      expected_capacity = 8;
    if (i == 8)
      expected_capacity = 16;
    if (i == 16)
      expected_capacity = 32;

    assert(stack->capacity == expected_capacity);
  }

  for (int i = 19; i >= 0; i--) {
    assert(pop(stack) == i);
  }

  assert(is_empty(stack));
  free_stack(stack);
}

void test_large_numbers() {
  Stack *stack = init_stack();

  push(stack, 2147483647);
  push(stack, -2147483648);

  assert(pop(stack) == -2147483648);
  assert(pop(stack) == 2147483647);

  free_stack(stack);
}

void test_interleaved_ops() {
  Stack *stack = init_stack();

  push(stack, 10);
  push(stack, 20);
  assert(pop(stack) == 20);

  push(stack, 30);
  assert(pop(stack) == 30);
  assert(pop(stack) == 10);

  push(stack, 40);
  push(stack, 50);
  assert(pop(stack) == 50);
  assert(pop(stack) == 40);

  assert(is_empty(stack));

  free_stack(stack);
}

int main() {
  test_init_stack();
  test_push_single_element();
  test_push_multiple_elements();
  test_pop_single_element();
  test_pop_multiple_elements();
  test_is_empty();
  test_resize_when_full();
  test_multiple_resizes();
  test_large_numbers();
  test_interleaved_ops();
  return 0;
}
