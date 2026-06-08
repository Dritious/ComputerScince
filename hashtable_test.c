#include "buddy_alloc.c"
#include "hashtable.c"
#include <assert.h>
#include <stdio.h>

void test_sys_basic() {
  printf("Тест 1: Вставка и поиск на системном аллокаторе... ");
  IAllocator alloc = create_sys_alloc(NULL, 0);
  HashTable t;
  assert(hashtable_init(&t, &alloc, 16) == 0);

  int a = 1, b = 2, c = 3;
  assert(hashtable_insert(&t, "one", &a) == 0);
  assert(hashtable_insert(&t, "two", &b) == 0);
  assert(hashtable_insert(&t, "three", &c) == 0);
  assert(hashtable_size(&t) == 3);

  void *out;
  assert(hashtable_get(&t, "one", &out) == 0 && *(int *)out == 1);
  assert(hashtable_get(&t, "two", &out) == 0 && *(int *)out == 2);
  assert(hashtable_get(&t, "three", &out) == 0 && *(int *)out == 3);
  assert(hashtable_get(&t, "missing", &out) == -1);

  hashtable_free(&t);
  printf("Пройден\n");
}

void test_update_existing() {
  printf("Тест 2: Обновление значения по существующему ключу... ");
  IAllocator alloc = create_sys_alloc(NULL, 0);
  HashTable t;
  hashtable_init(&t, &alloc, 16);

  int a = 10, b = 20;
  hashtable_insert(&t, "key", &a);
  hashtable_insert(&t, "key", &b);
  assert(hashtable_size(&t) == 1);

  void *out;
  hashtable_get(&t, "key", &out);
  assert(*(int *)out == 20);

  hashtable_free(&t);
  printf("Пройден\n");
}

void test_collisions() {
  printf("Тест 3: Коллизии (метод цепочек)... ");
  IAllocator alloc = create_sys_alloc(NULL, 0);
  HashTable t;
  hashtable_init(&t, &alloc, 2);

  char keys[20][8];
  int values[20];
  for (int i = 0; i < 20; i++) {
    sprintf(keys[i], "k%d", i);
    values[i] = i * 100;
    assert(hashtable_insert(&t, keys[i], &values[i]) == 0);
  }
  assert(hashtable_size(&t) == 20);

  for (int i = 0; i < 20; i++) {
    void *out;
    assert(hashtable_get(&t, keys[i], &out) == 0);
    assert(*(int *)out == i * 100);
  }

  hashtable_free(&t);
  printf("Пройден\n");
}

void test_remove() {
  printf("Тест 4: Удаление с освобождением памяти... ");
  IAllocator alloc = create_sys_alloc(NULL, 0);
  HashTable t;
  hashtable_init(&t, &alloc, 4);

  int v[5] = {1, 2, 3, 4, 5};
  char keys[5][8];
  for (int i = 0; i < 5; i++) {
    sprintf(keys[i], "id%d", i);
    hashtable_insert(&t, keys[i], &v[i]);
  }

  assert(hashtable_remove(&t, "id2") == 0);
  assert(hashtable_size(&t) == 4);

  void *out;
  assert(hashtable_get(&t, "id2", &out) == -1);
  assert(hashtable_get(&t, "id0", &out) == 0);
  assert(hashtable_get(&t, "id4", &out) == 0);

  assert(hashtable_remove(&t, "id2") == -1);
  assert(hashtable_remove(&t, "nope") == -1);

  hashtable_free(&t);
  printf("Пройден\n");
}

void test_buddy_integration() {
  printf("Тест 5: Интеграция с Buddy-аллокатором... ");

  static char buffer[1 << 16];
  BuddyCtx ctx;
  IAllocator alloc = create_buddy_alloc(&ctx, buffer, sizeof(buffer));

  HashTable t;
  assert(hashtable_init(&t, &alloc, 16) == 0);

  char keys[50][8];
  int values[50];
  for (int i = 0; i < 50; i++) {
    sprintf(keys[i], "key%d", i);
    values[i] = i + 1;
    assert(hashtable_insert(&t, keys[i], &values[i]) == 0);
  }
  assert(hashtable_size(&t) == 50);

  for (int i = 0; i < 50; i++) {
    void *out;
    assert(hashtable_get(&t, keys[i], &out) == 0);
    assert(*(int *)out == i + 1);
  }

  for (int i = 0; i < 50; i += 2) {
    assert(hashtable_remove(&t, keys[i]) == 0);
  }
  assert(hashtable_size(&t) == 25);

  for (int i = 1; i < 50; i += 2) {
    void *out;
    assert(hashtable_get(&t, keys[i], &out) == 0);
    assert(*(int *)out == i + 1);
  }

  hashtable_free(&t);
  printf("Пройден\n");
}

int main() {
  test_sys_basic();
  test_update_existing();
  test_collisions();
  test_remove();
  test_buddy_integration();

  printf("\nВсе тесты пройдены!\n");
  return 0;
}
