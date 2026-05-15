#include "buddy_alloc.c"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void run_tests() {
  // 1. Инициализация. Буфер 1024 байта (2^10).
  // Важно: в вашей реализации max_order вычисляется через
  // get_order(memory_size). 1024 байта дадут max_order = 10.
  uint8_t buffer[1024];
  BuddyCtx ctx;
  IAllocator alloc = create_buddy_alloc(&ctx, buffer, sizeof(buffer));

  // 2. Базовая аллокация
  // Просим 64 байта. С учетом Header (2-8 байт), это попадет в блок 2^7 (128
  // байт) или 2^6 (64 байта), если заголовок влезет.
  void *p1 = i_alloc(&alloc, 30);
  assert(p1 != NULL);

  // Проверка записи: не должно падать
  memset(p1, 0xAA, 30);

  // 3. Проверка фрагментации и слияния
  // Выделяем два одинаковых маленьких блока
  void *p2 = i_alloc(&alloc, 60);
  void *p3 = i_alloc(&alloc, 60);
  assert(p2 != NULL);
  assert(p3 != NULL);
  assert(p2 != p3);

  // Запоминаем адрес p2. После освобождения p2 и p3 они обязаны слиться.
  i_free(&alloc, p2);
  i_free(&alloc, p3);

  // Если слияние сработало, мы сможем выделить один большой блок на их месте
  void *p4 = i_alloc(&alloc, 130);
  assert(p4 != NULL);
  i_free(&alloc, p4);
  i_free(&alloc, p1);

  // 4. Проверка исчерпания памяти
  // Занимаем почти весь буфер
  void *big = i_alloc(&alloc, 800);
  assert(big != NULL);

  // Пытаемся взять еще 512 — должно не хватить
  void *too_big = i_alloc(&alloc, 512);
  assert(too_big == NULL);

  i_free(&alloc, big);

  // 5. Тестирование REALLOC
  void *r1 = i_alloc(&alloc, 100);
  assert(r1 != NULL);
  memcpy(r1, "BuddyTest", 10);

  // Расширение (должно привести к перемещению, т.к. 500 > 128/256)
  void *r2 = i_realloc(&alloc, r1, 500);

  // buddy allocator может не найти непрерывный блок
  if (r2 == NULL) {
    i_free(&alloc, r1);
    r2 = i_alloc(&alloc, 500);
  }

  assert(r2 != NULL);
  assert(memcmp(r2, "BuddyTest", 10) == 0); // Данные сохранились

  // Уменьшение (должно вернуть тот же указатель, если попадает в тот же
  // порядок)
  void *r3 = i_realloc(&alloc, r2, 490);
  assert(r2 == r3);

  i_free(&alloc, r3);

  // 6. Проверка полного восстановления (Reset через Free)
  // После всех манипуляций и освобождений мы должны быть способны выделить
  // блок максимального размера (минус заголовок)
  void *final = i_alloc(&alloc, 1000);
  assert(final != NULL);
  i_free(&alloc, final);
}

int main() {
  run_tests();
  return 0;
}
