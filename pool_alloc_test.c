#include "allocator.h"
#include "pool_alloc.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Здесь подразумевается подключение твоего allocator.h
// #include "allocator.h"

void test_sys_alloc() {
  // Память и размер не используются в реализации sys_alloc, передаем нули
  IAllocator sys = create_sys_alloc(NULL, 0);

  // Тест выделения
  int *ptr = (int *)i_alloc(&sys, sizeof(int) * 10);
  assert(ptr != NULL);

  // Пишем данные, чтобы проверить их сохранность после realloc
  for (int i = 0; i < 10; ++i) {
    ptr[i] = i;
  }

  // Тест перевыделения
  ptr = (int *)i_realloc(&sys, ptr, sizeof(int) * 20);
  assert(ptr != NULL);
  for (int i = 0; i < 10; ++i) {
    assert(ptr[i] == i); // Данные должны сохраниться
  }

  // Тест освобождения (к сожалению, без инструментов типа Valgrind мы
  // можем только убедиться, что free не падает)
  i_free(&sys, ptr);

  // Проверка заглушки
  i_reset(&sys);

  printf("System allocator tests passed!\n");
}

void test_pool_alloc() {
  // Буфер на 128 байт. При размере блока 32 байта, это ровно 4 блока.
  char memory_buffer[128];
  PoolCtx ctx;
  IAllocator pool =
      create_pool_alloc(&ctx, memory_buffer, sizeof(memory_buffer), 32);

  // 1. Тест выделения всех доступных блоков
  void *p1 = i_alloc(&pool, 32);
  void *p2 = i_alloc(&pool, 32);
  void *p3 = i_alloc(&pool, 32);
  void *p4 = i_alloc(&pool, 32);

  assert(p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL);
  // Убеждаемся, что адреса разные
  assert(p1 != p2 && p1 != p3 && p1 != p4);

  // 2. Тест исчерпания памяти (Out of Memory)
  // Пул пуст, пятое выделение должно вернуть NULL
  void *p5 = i_alloc(&pool, 32);
  assert(p5 == NULL);

  // 3. Тест освобождения и переиспользования
  i_free(&pool, p2); // Возвращаем второй блок в пул

  void *p6 = i_alloc(&pool, 32);
  assert(p6 != NULL);
  assert(p6 == p2); // Аллокатор должен выдать именно тот блок, который мы
                    // только что освободили

  // 4. Тест работы заглушек для пула
  // realloc для фиксированного пула использует stub_realloc и должен возвращать
  // NULL
  void *p_realloc = i_realloc(&pool, p1, 64);
  assert(p_realloc == NULL);

  i_reset(&pool); // stub_reset не должен ничего ломать

  printf("Pool allocator tests passed!\n");
}

int main() {
  test_sys_alloc();
  test_pool_alloc();
  printf("All tests passed successfully.\n");
  return 0;
}
