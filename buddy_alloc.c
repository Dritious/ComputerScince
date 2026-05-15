#include "allocator.c"
#include "allocator.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MIN_ORDER 5
#define MAX_ORDER 32
#define MIN_BLOCK (1ULL << MIN_ORDER)

typedef struct BuddyBlock {
  uint8_t order;
  uint8_t is_free;
  struct BuddyBlock *next;
} BuddyBlock;

typedef struct BuddyCtx {
  void *base;
  size_t size;
  uint8_t max_order;
  BuddyBlock *free_lists[MAX_ORDER];
} BuddyCtx;

static size_t block_size(uint8_t order) { return 1ULL << order; }

static BuddyBlock *user_to_block(BuddyCtx *ctx, void *ptr) {
  if (!ctx || !ctx->base || !ptr)
    return NULL;

  uintptr_t base = (uintptr_t)ctx->base;
  uintptr_t user = (uintptr_t)ptr;

  if (user < base + sizeof(BuddyBlock) || user >= base + ctx->size) {
    return NULL;
  }

  return (BuddyBlock *)(user - sizeof(BuddyBlock));
}

static int remove_from_free_list(BuddyCtx *ctx, BuddyBlock *block,
                                 uint8_t order) {
  BuddyBlock **cur = &ctx->free_lists[order];
  while (*cur) {
    if (*cur == block) {
      *cur = block->next;
      block->next = NULL;
      return 1;
    }
    cur = &(*cur)->next;
  }
  return 0;
}
static uint8_t order_from_size(size_t size) {
  uint8_t order = MIN_ORDER;
  while (order < MAX_ORDER - 1 && block_size(order) < size) {
    ++order;
  }
  return order;
}
static inline void *align_up(void *ptr, size_t alignment) {
  uintptr_t p = (uintptr_t)ptr;
  return (void *)((p + alignment - 1ULL) & ~(uintptr_t)(alignment - 1ULL));
}

static uint8_t floor_order(size_t size) {
  uint8_t order = MIN_ORDER;

  while (order + 1 < MAX_ORDER && block_size((uint8_t)(order + 1)) <= size) {
    ++order;
  }

  return order;
}

void *buddy_alloc_impl(IAllocator *self, size_t size) {
  BuddyCtx *ctx = self ? (BuddyCtx *)self->ctx : NULL;
  if (!ctx || !ctx->base || size > SIZE_MAX - sizeof(BuddyBlock)) {
    return NULL;
  }

  size_t need = size + sizeof(BuddyBlock);
  if (need < MIN_BLOCK) {
    need = MIN_BLOCK;
  }

  uint8_t order = order_from_size(need);

  if (block_size(order) < need) {
    return NULL;
  }

  if (order > ctx->max_order) {
    return NULL;
  }

  uint8_t current = order;
  while (current <= ctx->max_order && ctx->free_lists[current] == NULL) {
    ++current;
  }

  if (current > ctx->max_order) {
    return NULL;
  }

  BuddyBlock *block = ctx->free_lists[current];
  ctx->free_lists[current] = block->next;

  while (current > order) {
    --current;

    BuddyBlock *buddy = (BuddyBlock *)((uint8_t *)block + block_size(current));
    buddy->order = current;
    buddy->is_free = 1;
    buddy->next = ctx->free_lists[current];
    ctx->free_lists[current] = buddy;
  }

  block->order = order;
  block->is_free = 0;
  block->next = NULL;

  return (uint8_t *)block + sizeof(BuddyBlock);
}

void buddy_free_impl(IAllocator *self, void *ptr) {
  BuddyCtx *ctx = self ? (BuddyCtx *)self->ctx : NULL;
  BuddyBlock *block = user_to_block(ctx, ptr);
  if (!ctx || !block || block->is_free || block->order < MIN_ORDER ||
      block->order > ctx->max_order) {
    return;
  }

  uintptr_t base = (uintptr_t)ctx->base;
  uintptr_t current_addr = (uintptr_t)block;
  uint8_t order = block->order;

  while (order < ctx->max_order) {
    size_t size = block_size(order);
    uintptr_t offset = current_addr - base;
    uintptr_t buddy_offset = offset ^ size;

    if (buddy_offset >= ctx->size) {
      break;
    }

    BuddyBlock *buddy = (BuddyBlock *)(base + buddy_offset);

    if ((uintptr_t)buddy < base ||
        (uintptr_t)buddy + sizeof(BuddyBlock) > base + ctx->size) {
      break;
    }

    if (!buddy->is_free || buddy->order != order) {
      break;
    }
    if (!buddy->is_free || buddy->order != order) {
      break;
    }

    if (!remove_from_free_list(ctx, buddy, order)) {
      break;
    }

    if (buddy_offset < offset) {
      current_addr = base + buddy_offset;
    }

    ++order;
  }

  block = (BuddyBlock *)current_addr;
  block->order = order;
  block->is_free = 1;
  block->next = ctx->free_lists[order];
  ctx->free_lists[order] = block;
}

void *buddy_realloc_impl(IAllocator *self, void *ptr, size_t new_size) {
  BuddyCtx *ctx = self ? (BuddyCtx *)self->ctx : NULL;

  if (!ptr) {
    return buddy_alloc_impl(self, new_size);
  }

  if (new_size == 0) {
    buddy_free_impl(self, ptr);
    return NULL;
  }

  BuddyBlock *block = user_to_block(ctx, ptr);
  if (!ctx || !block || block->is_free || block->order < MIN_ORDER ||
      block->order > ctx->max_order) {
    return NULL;
  }

  size_t current_payload = block_size(block->order) - sizeof(BuddyBlock);
  if (new_size <= current_payload) {
    return ptr;
  }

  void *new_ptr = buddy_alloc_impl(self, new_size);
  if (!new_ptr) {
    return NULL;
  }

  memcpy(new_ptr, ptr, current_payload);
  buddy_free_impl(self, ptr);
  return new_ptr;
}

IAllocator create_buddy_alloc(BuddyCtx *ctx, void *memory, size_t memory_size) {
  if (ctx) {
    memset(ctx, 0, sizeof(*ctx));
  }

  if (!ctx || !memory || memory_size < MIN_BLOCK) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  void *base = align_up(memory, alignof(max_align_t));
  size_t offset = (size_t)((uint8_t *)base - (uint8_t *)memory);
  if (offset >= memory_size) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  size_t usable = memory_size - offset;
  if (usable < MIN_BLOCK) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  uint8_t max_order = floor_order(usable);
  size_t region_size = block_size(max_order);

  if (region_size < MIN_BLOCK || region_size > usable) {
    return (IAllocator){.alloc = buddy_alloc_impl,
                        .free = buddy_free_impl,
                        .realloc = buddy_realloc_impl,
                        .reset = stub_reset,
                        .ctx = ctx};
  }

  ctx->base = base;
  ctx->size = region_size;
  ctx->max_order = max_order;

  BuddyBlock *first = (BuddyBlock *)ctx->base;
  first->order = max_order;
  first->is_free = 1;
  first->next = NULL;
  ctx->free_lists[max_order] = first;

  return (IAllocator){.alloc = buddy_alloc_impl,
                      .free = buddy_free_impl,
                      .realloc = buddy_realloc_impl,
                      .reset = stub_reset,
                      .ctx = ctx};
}
