#include "allocator.c"
#include "allocator.h"
#include <stdlib.h>

typedef struct {
  void *buffer;
  size_t size;
  size_t offset;
} LinearCtx;

static void *linear_alloc(IAllocator *self, size_t alloc_size) {
  LinearCtx *ctx = self->ctx;
  if (ctx->offset + alloc_size > ctx->size) {
    return NULL;
  }

  void *result = ctx->buffer + ctx->offset;
  ctx->offset += alloc_size;
  return result;
}

static void linear_reset(IAllocator *self) {
  LinearCtx *ctx = self->ctx;
  ctx->offset = 0;
}

IAllocator create_linear_alloc(void *memory, size_t memory_size) {
  LinearCtx *ctx = malloc(sizeof(LinearCtx)); // через указатель!
  ctx->buffer = memory;
  ctx->size = memory_size;
  ctx->offset = 0;

  return (IAllocator){.alloc = linear_alloc,
                      .free = stub_free,
                      .realloc = stub_realloc,
                      .reset = linear_reset,
                      .ctx = ctx};
}
