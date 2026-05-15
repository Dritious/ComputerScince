#include "allocator.h"
// Stub-functions
void stub_free(IAllocator *self, void *ptr) {
  UNUSED(self);
  UNUSED(ptr);
}

void *stub_realloc(IAllocator *self, void *ptr, size_t size) {
  UNUSED(self);
  UNUSED(ptr);
  UNUSED(size);
  return NULL;
}

void stub_reset(IAllocator *self) { UNUSED(self); }

// Sys-functions
IAllocator create_sys_alloc(void *memory, size_t memory_size) {
  return (IAllocator){.alloc = sys_alloc_impl,
                      .realloc = sys_realloc_impl,
                      .free = sys_free_impl,
                      .reset = stub_reset,
                      .ctx = NULL};
}
void sys_free_impl(IAllocator *self, void *ptr) {
  UNUSED(self);
  free(ptr);
}
void *sys_alloc_impl(IAllocator *self, size_t size) {
  UNUSED(self);
  return malloc(size);
}
void *sys_realloc_impl(IAllocator *self, void *ptr, size_t new_size) {
  return realloc(ptr, new_size);
}
