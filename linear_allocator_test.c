// test_linear.c
#include "allocator.h"
#include "linear_allocator.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Создаём аллокатор и возвращаем. Не забудь объявление из .h
IAllocator create_linear_alloc(void *memory, size_t memory_size);

void test_basic_allocation() {
  printf("Тест 1: Базовое выделение... ");

  char memory[100];
  IAllocator alloc = create_linear_alloc(memory, 100);

  void *p1 = i_alloc(&alloc, 30);
  assert(p1 != NULL);
  assert(p1 == (void *)memory); // первый блок в начале

  void *p2 = i_alloc(&alloc, 40);
  assert(p2 != NULL);
  assert(p2 == (void *)(memory + 30)); // второй блок сразу за первым

  printf("Пройден\n");
}

void test_allocation_until_full() {
  printf("Тест 2: Заполнение до конца... ");

  char memory[50];
  IAllocator alloc = create_linear_alloc(memory, 50);

  void *p1 = i_alloc(&alloc, 50);
  assert(p1 != NULL); // ровно столько, сколько есть

  void *p2 = i_alloc(&alloc, 1);
  assert(p2 == NULL); // места уже нет

  printf("Пройден\n");
}

void test_multiple_small_allocs() {
  printf("Тест 3: Много маленьких выделений... ");

  char memory[30];
  IAllocator alloc = create_linear_alloc(memory, 30);

  void *blocks[6];
  for (int i = 0; i < 6; i++) {
    blocks[i] = i_alloc(&alloc, 5);
    assert(blocks[i] != NULL);
  }

  void *over = i_alloc(&alloc, 1);
  assert(over == NULL); // 6 * 5 = 30, больше места нет

  // Проверяем, что блоки не пересекаются
  for (int i = 0; i < 6; i++) {
    assert(blocks[i] == (void *)(memory + i * 5));
  }

  printf("Пройден\n");
}

void test_write_and_read() {
  printf("Тест 4: Запись и чтение данных... ");

  char memory[64];
  IAllocator alloc = create_linear_alloc(memory, 64);

  char *str = i_alloc(&alloc, 12);
  strcpy(str, "Hello World");

  int *num = i_alloc(&alloc, sizeof(int));
  *num = 42;

  assert(strcmp(str, "Hello World") == 0);
  assert(*num == 42);
  assert((void *)num == (void *)(str + 12)); // int сразу за строкой

  printf("Пройден\n");
}

void test_reset() {
  printf("Тест 5: Сброс аллокатора... ");

  char memory[50];
  IAllocator alloc = create_linear_alloc(memory, 50);

  void *p1 = i_alloc(&alloc, 30);
  assert(p1 != NULL);

  void *p2 = i_alloc(&alloc, 30);
  assert(p2 == NULL); // не хватает места

  i_reset(&alloc); // сбрасываем

  // После сброса должно работать как новый
  void *p3 = i_alloc(&alloc, 30);
  assert(p3 != NULL);
  assert(p3 == p1); // тот же адрес, что и в первый раз

  void *p4 = i_alloc(&alloc, 20);
  assert(p4 != NULL); // теперь хватает

  printf("Пройден\n");
}

void test_zero_allocation() {
  printf("Тест 6: Выделение 0 байт... ");

  char memory[10];
  IAllocator alloc = create_linear_alloc(memory, 10);

  void *p1 = i_alloc(&alloc, 0);
  assert(p1 != NULL); // 0 байт — допустимо
  assert(p1 == (void *)memory);

  void *p2 = i_alloc(&alloc, 10);
  assert(p2 == (void *)memory); // offset не сдвинулся

  printf("Пройден\n");
}

void test_empty_buffer() {
  printf("Тест 7: Пустой буфер... ");

  IAllocator alloc = create_linear_alloc(NULL, 0);

  void *p = i_alloc(&alloc, 1);
  assert(p == NULL); // некуда выделять

  printf("Пройден\n");
}

int main() {
  test_basic_allocation();
  test_allocation_until_full();
  test_multiple_small_allocs();
  test_write_and_read();
  test_reset();
  test_zero_allocation();
  test_empty_buffer();

  printf("\nВсе тесты пройдены!\n");
  return 0;
}
