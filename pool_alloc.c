#include "allocator.c"
#include "allocator.h"

typedef struct Node {
  struct Node *next;
} Node;
typedef struct {
  void *buffer;
  size_t block_size;
  Node *free_list; // Указатель на первый свободный блок
} PoolCtx;

// Pool-functions
void pool_free_impl(IAllocator *self, void *ptr) {
  if (!ptr)
    return;
  PoolCtx *ctx = (PoolCtx *)self->ctx;
  Node *node = (Node *)ptr;

  node->next = ctx->free_list;
  ctx->free_list = node;
}

void *pool_alloc_impl(IAllocator *self, size_t size) {
  UNUSED(size); // Пул выдает блоки фиксированного размера
  PoolCtx *ctx = (PoolCtx *)self->ctx;

  if (!ctx->free_list)
    return NULL;

  Node *node = ctx->free_list;
  ctx->free_list = node->next;

  return node;
}

IAllocator create_pool_alloc(PoolCtx *ctx, void *memory, size_t memory_size,
                             size_t block_size) {
  ctx->buffer = memory;
  ctx->block_size = block_size;
  ctx->free_list = NULL;

  // Инициализация списка свободных блоков
  size_t block_count = memory_size / block_size;
  char *mem_ptr = (char *)memory;

  for (size_t i = 0; i < block_count; ++i) {
    Node *node = (Node *)(mem_ptr + i * block_size);
    node->next = ctx->free_list;
    ctx->free_list = node;
  }

  return (IAllocator){.alloc = pool_alloc_impl,
                      .realloc = stub_realloc,
                      .free = pool_free_impl,
                      .reset = stub_reset,
                      .ctx = ctx};
}
