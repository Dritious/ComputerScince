#include "allocator.c"
#include "allocator.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MAX_ORDERS 32
#define MIN_ORDER 5 // Минимальный блок 32 байта (заголовок + выравнивание)

typedef struct BlockHeader {
  uint8_t order;
  bool is_free;
  struct BlockHeader *next_free;
} BlockHeader;

typedef struct {
  void *buffer;      // Выровненная база buddy-региона
  size_t total_size; // Размер buddy-региона, кратный степени двойки
  uint8_t max_order;
  BlockHeader *free_lists[MAX_ORDERS];
} BuddyCtx;

// Округление вверх до выравнивания
static void *align_up_ptr(void *ptr, size_t alignment) {
  uintptr_t p = (uintptr_t)ptr;
  uintptr_t aligned = (p + alignment - 1) & ~(uintptr_t)(alignment - 1);
  return (void *)aligned;
}

// Размер блока по порядку
static size_t get_block_size(uint8_t order) { return (1ULL << order); }

// для allocation (ceil)
static uint8_t get_order(size_t size) {
  uint8_t order = MIN_ORDER;
  while (order < MAX_ORDERS - 1 && (1ULL << order) < size) {
    order++;
  }
  return order;
}

// для max_order (floor)
static uint8_t get_max_order(size_t size) {
  uint8_t order = MIN_ORDER;
  while (order < MAX_ORDERS - 1 && (1ULL << (order + 1)) <= size) {
    order++;
  }
  return order;
}

// Удаление конкретного узла из списка свободных (нужно при слиянии)
static void remove_from_list(BuddyCtx *ctx, BlockHeader *node, uint8_t order) {
  BlockHeader **curr = &ctx->free_lists[order];
  while (*curr) {
    if (*curr == node) {
      *curr = node->next_free;
      node->next_free = NULL;
      return;
    }
    curr = &(*curr)->next_free;
  }
}

void *buddy_alloc_impl(IAllocator *self, size_t size) {
  BuddyCtx *ctx = (BuddyCtx *)self->ctx;

  if (!ctx->buffer || ctx->total_size == 0) {
    return NULL;
  }

  // Учитываем размер заголовка
  if (size > SIZE_MAX - sizeof(BlockHeader)) {
    return NULL;
  }

  size_t actual_size = size + sizeof(BlockHeader);
  uint8_t order = get_order(actual_size);

  if (order > ctx->max_order) {
    return NULL;
  }

  // Ищем свободный блок начиная с нужного порядка
  uint8_t i = order;
  while (i <= ctx->max_order && ctx->free_lists[i] == NULL) {
    i++;
  }

  if (i > ctx->max_order) {
    return NULL; // Память кончилась
  }

  // Извлекаем блок
  BlockHeader *block = ctx->free_lists[i];
  ctx->free_lists[i] = block->next_free;
  block->next_free = NULL;

  // Расщепляем блоки (Splitting), если нашли блок большего размера
  while (i > order) {
    i--;

    // Находим адрес напарника (половина текущего блока)
    BlockHeader *buddy = (BlockHeader *)((uint8_t *)block + get_block_size(i));

    // Добавляем напарника в список свободных уровнем ниже
    buddy->order = i;
    buddy->is_free = true;
    buddy->next_free = ctx->free_lists[i];
    ctx->free_lists[i] = buddy;
  }

  // Оформляем выделенный блок
  block->order = order;
  block->is_free = false;
  block->next_free = NULL;

  return (void *)(block + 1);
}

void buddy_free_impl(IAllocator *self, void *ptr) {
  if (!ptr) {
    return;
  }

  BuddyCtx *ctx = (BuddyCtx *)self->ctx;
  BlockHeader *hdr = (BlockHeader *)ptr - 1;

  // Защита от двойного освобождения
  if (hdr->is_free) {
    return;
  }

  uint8_t order = hdr->order;
  void *curr_block = hdr;

  // Пытаемся слить с напарниками (Coalescing)
  while (order < ctx->max_order) {
    uintptr_t offset = (uintptr_t)curr_block - (uintptr_t)ctx->buffer;
    uintptr_t buddy_offset = offset ^ get_block_size(order);

    if (buddy_offset >= ctx->total_size) {
      break;
    }

    BlockHeader *buddy = (BlockHeader *)((uint8_t *)ctx->buffer + buddy_offset);

    // Условия слияния: напарник свободен и имеет тот же порядок
    if (!buddy->is_free || buddy->order != order) {
      break;
    }

    // Вынимаем напарника из списка свободных
    remove_from_list(ctx, buddy, order);

    // Переходим на уровень выше (адрес нового блока — меньший из двух)
    if (buddy_offset < offset) {
      curr_block = buddy;
    }

    order++;
  }

  // Добавляем итоговый (возможно слитый) блок в список
  BlockHeader *final_hdr = (BlockHeader *)curr_block;
  final_hdr->order = order;
  final_hdr->is_free = true;
  final_hdr->next_free = ctx->free_lists[order];
  ctx->free_lists[order] = final_hdr;
}

void *buddy_realloc_impl(IAllocator *self, void *ptr, size_t new_size) {
  if (!ptr) {
    return buddy_alloc_impl(self, new_size);
  }

  if (new_size == 0) {
    buddy_free_impl(self, ptr);
    return NULL;
  }

  BlockHeader *hdr = (BlockHeader *)ptr - 1;
  size_t current_block_size = (1ULL << hdr->order) - sizeof(BlockHeader);

  // Если новый размер влезает в текущий блок — оставляем как есть
  if (new_size <= current_block_size) {
    return ptr;
  }

  // Иначе: аллокация -> копирование -> освобождение
  void *new_ptr = buddy_alloc_impl(self, new_size);
  if (new_ptr) {
    size_t copy_size =
        (new_size < current_block_size) ? new_size : current_block_size;
    memcpy(new_ptr, ptr, copy_size);
    buddy_free_impl(self, ptr);
  }

  return new_ptr;
}

IAllocator create_buddy_alloc(BuddyCtx *ctx, void *memory, size_t memory_size) {
  memset(ctx, 0, sizeof(*ctx));

  if (!memory || memory_size < (1ULL << MIN_ORDER)) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  // Выравниваем базу под указатель/максимальное выравнивание
  void *aligned_memory = align_up_ptr(memory, alignof(max_align_t));
  size_t offset = (size_t)((uint8_t *)aligned_memory - (uint8_t *)memory);

  if (offset >= memory_size) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  size_t usable_size = memory_size - offset;
  uint8_t max_order = get_max_order(usable_size);
  size_t region_size = get_block_size(max_order);

  if (region_size < (1ULL << MIN_ORDER)) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  ctx->buffer = aligned_memory;
  ctx->total_size = region_size;
  ctx->max_order = max_order;

  // Обнуляем списки
  for (int i = 0; i < MAX_ORDERS; i++) {
    ctx->free_lists[i] = NULL;
  }

  // Инициализируем первый большой блок
  BlockHeader *first = (BlockHeader *)ctx->buffer;
  first->order = ctx->max_order;
  first->is_free = true;
  first->next_free = NULL;
  ctx->free_lists[ctx->max_order] = first;

  return (IAllocator){.alloc = buddy_alloc_impl,
                      .free = buddy_free_impl,
                      .realloc = buddy_realloc_impl,
                      .reset = stub_reset,
                      .ctx = ctx};
}
