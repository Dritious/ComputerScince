#include "allocator.h"

typedef struct {
  void *buffer;
  size_t size;
  size_t offset;
} LinearCtx;

IAllocator create_linear_alloc(void *memory, size_t memory_size) {
  ctx = LinearCtx{
      .buffer = memory, .size = memory_size, .offset = 0} return IAllocator {
    .alloc = linear_alloc, .free = stub_free, .realloc = stub_realloc,
    .reset = linear_reset, .ctx = ctx
  }
}

void *linear_alloc(IAllocator *self, size_t alloc_size) {
  LinearCtx * = self->ctx;
  if (ctx->offset + alloc_size > ctx->size) {
    return NULL;
  }
  void *result = ctx->buffer + ctx->offset;
  ctx->offset += alloc_size;
  return result;
}

void linear_reset(IAllocator *self) { self->ctx->offset = 0; }
