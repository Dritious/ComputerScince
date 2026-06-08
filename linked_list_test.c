#include "linked_list.c"
#include <assert.h>
#include <stdio.h>

static void assert_int_eq(int actual, int expected, const char *test_name) {
  if (actual != expected) {
    fprintf(stderr, "%s: FAILED\n", test_name);
    fprintf(stderr, "expected: %d\n", expected);
    fprintf(stderr, "actual:   %d\n", actual);
    assert(!"Test failed");
  }
}

static int get(const List *list, int index) {
  int v;
  int rc = list_get(list, index, &v);
  assert(rc == 0);
  return v;
}

int main() {
  List list;
  list_init(&list);

  assert_int_eq(list_size(&list), 0, "пустой список: размер 0");

  list_push_back(&list, 10);
  list_push_back(&list, 20);
  list_push_back(&list, 30);
  assert_int_eq(list_size(&list), 3, "push_back: размер");
  assert_int_eq(get(&list, 0), 10, "push_back: элемент 0");
  assert_int_eq(get(&list, 1), 20, "push_back: элемент 1");
  assert_int_eq(get(&list, 2), 30, "push_back: элемент 2");

  list_push_front(&list, 5);
  assert_int_eq(list_size(&list), 4, "push_front: размер");
  assert_int_eq(get(&list, 0), 5, "push_front: элемент 0");
  assert_int_eq(get(&list, 1), 10, "push_front: элемент 1");

  int rc = list_insert(&list, 2, 15);
  assert_int_eq(rc, 0, "insert середина: код возврата");
  assert_int_eq(list_size(&list), 5, "insert середина: размер");
  assert_int_eq(get(&list, 2), 15, "insert середина: элемент 2");
  assert_int_eq(get(&list, 3), 20, "insert середина: сдвиг");

  rc = list_insert(&list, 0, 1);
  assert_int_eq(rc, 0, "insert в начало: код возврата");
  assert_int_eq(get(&list, 0), 1, "insert в начало: элемент 0");

  rc = list_insert(&list, list_size(&list), 99);
  assert_int_eq(rc, 0, "insert в конец: код возврата");
  assert_int_eq(get(&list, list_size(&list) - 1), 99,
                "insert в конец: последний элемент");
  assert_int_eq(list_size(&list), 7, "после вставок: размер");

  rc = list_insert(&list, 100, 0);
  assert_int_eq(rc, -1, "insert вне диапазона: отказ");
  rc = list_insert(&list, -1, 0);
  assert_int_eq(rc, -1, "insert отрицательный индекс: отказ");

  rc = list_remove(&list, 3);
  assert_int_eq(rc, 0, "remove середина: код возврата");
  assert_int_eq(list_size(&list), 6, "remove середина: размер");
  assert_int_eq(get(&list, 3), 20, "remove середина: сдвиг");

  rc = list_remove(&list, 0);
  assert_int_eq(rc, 0, "remove начало: код возврата");
  assert_int_eq(get(&list, 0), 5, "remove начало: новый элемент 0");

  rc = list_remove(&list, list_size(&list) - 1);
  assert_int_eq(rc, 0, "remove конец: код возврата");
  assert_int_eq(get(&list, list_size(&list) - 1), 30,
                "remove конец: новый последний");
  assert_int_eq(list_size(&list), 4, "после удалений: размер");

  rc = list_remove(&list, 4);
  assert_int_eq(rc, -1, "remove вне диапазона: отказ");

  int tmp;
  rc = list_get(&list, 10, &tmp);
  assert_int_eq(rc, -1, "get вне диапазона: отказ");

  list_free(&list);
  assert_int_eq(list_size(&list), 0, "list_free: размер 0");

  printf("Все тесты linked_list пройдены.\n");
  return 0;
}
