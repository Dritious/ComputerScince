#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int *data;
  int top;
  int capacity;
} Stack;

Stack *init_stack() {
  Stack *stack = malloc(sizeof(Stack));
  if (stack == NULL)
    return NULL;

  stack->capacity = 4;
  stack->data = malloc(stack->capacity * sizeof(int));
  if (stack->data == NULL) {
    free(stack);
    return NULL;
  }

  stack->top = -1;
  return stack;
}

void resize_stack(Stack *stack) {
  stack->capacity *= 2;
  int *new_data = realloc(stack->data, stack->capacity * sizeof(int));
  if (new_data == NULL) {
    exit(1);
  }
  stack->data = new_data;
}

int is_empty(Stack *stack) { return stack->top == -1; }

void push(Stack *stack, int num) {
  if (stack->top >= stack->capacity - 1) {
    // test_stack.c
    resize_stack(stack);
  }
  stack->data[++stack->top] = num;
}

int pop(Stack *stack) {
  assert(!is_empty(stack));
  return stack->data[stack->top--];
}

void free_stack(Stack *stack) {
  if (stack) {
    free(stack->data);
    free(stack);
  }
}
