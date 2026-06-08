#include <stdlib.h>

typedef struct Node {
  int value;
  struct Node *next;
} Node;

typedef struct {
  Node *head;
  int size;
} List;

void list_init(List *list) {
  list->head = NULL;
  list->size = 0;
}

int list_size(const List *list) { return list->size; }

void list_push_front(List *list, int value) {
  Node *node = malloc(sizeof(Node));
  node->value = value;
  node->next = list->head;
  list->head = node;
  list->size++;
}

void list_push_back(List *list, int value) {
  Node *node = malloc(sizeof(Node));
  node->value = value;
  node->next = NULL;

  if (list->head == NULL) {
    list->head = node;
  } else {
    Node *cur = list->head;
    while (cur->next != NULL) {
      cur = cur->next;
    }
    cur->next = node;
  }
  list->size++;
}

int list_insert(List *list, int index, int value) {
  if (index < 0 || index > list->size) {
    return -1;
  }
  if (index == 0) {
    list_push_front(list, value);
    return 0;
  }

  Node *prev = list->head;
  for (int i = 0; i < index - 1; i++) {
    prev = prev->next;
  }

  Node *node = malloc(sizeof(Node));
  node->value = value;
  node->next = prev->next;
  prev->next = node;
  list->size++;
  return 0;
}

int list_remove(List *list, int index) {
  if (index < 0 || index >= list->size) {
    return -1;
  }

  Node *to_delete;
  if (index == 0) {
    to_delete = list->head;
    list->head = to_delete->next;
  } else {
    Node *prev = list->head;
    for (int i = 0; i < index - 1; i++) {
      prev = prev->next;
    }
    to_delete = prev->next;
    prev->next = to_delete->next;
  }

  free(to_delete);
  list->size--;
  return 0;
}

int list_get(const List *list, int index, int *out) {
  if (index < 0 || index >= list->size) {
    return -1;
  }
  Node *cur = list->head;
  for (int i = 0; i < index; i++) {
    cur = cur->next;
  }
  *out = cur->value;
  return 0;
}

void list_free(List *list) {
  Node *cur = list->head;
  while (cur != NULL) {
    Node *next = cur->next;
    free(cur);
    cur = next;
  }
  list->head = NULL;
  list->size = 0;
}
