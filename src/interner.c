#include "header.h"
#include <stdint.h>
#include <string.h>

// returns index into slices or -1 if missing
static inline int interner_find_str(struct Interner* interner, const char* s,
                                    size_t len) {
  for (uint32_t i = 0; i < interner->strs.length; i++) {
    if (interner->strs.data[i].length != len)
      continue;

    if (strcmp(s, interner->arena.data + interner->strs.data->offset) == 0) {
      return i;
    }
  }
  return -1;
}

uint32_t interner_add_str(struct Interner* interner, const char* s,
                          size_t len) {
  uint32_t offset = interner->arena.length;
  arena_push(&interner->arena, (void*) s, len);
  const char zero = '\0';
  arena_push(&interner->arena, (void*) &zero, 1);

  struct Slice slice = {.offset = offset, .length = len};
  vec_slice_push(&interner->strs, slice);
  return interner->strs.length - 1;
}
uint32_t intern_from_cstr(struct Interner* interner, const char* s,
                          size_t len) {
  int index = interner_find_str(interner, s, len);

  if (index != -1) {
    return index;
  }

  return interner_add_str(interner, s, len);
}

struct Interner interner_init(uint32_t capacity) {
  struct Interner interner = {.arena = arena_init(capacity),
                              .strs = vec_slice_init(capacity)};
  return interner;
}

void interner_destroy(struct Interner* interner) {
  // TODO: implement this
}
