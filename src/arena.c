#include "header.h"

struct Arena arena_init(unsigned long capacity) {
  return (struct Arena) {
      .capacity = capacity,
      .length = 0,
      .data = malloc(capacity),
  };
}

unsigned long arena_push(struct Arena* arena, void* contents,
                         unsigned long length) {
  int has_grown = 0;
  while (arena->length + length >= arena->capacity) {
    arena->capacity *= 2;
    has_grown = 1;
  }
  if (has_grown) {
    arena->data = realloc(arena->data, arena->capacity);
  }

  unsigned long offset = arena->length;
  memcpy((char*) arena->data + arena->length, contents, length);
  arena->length += length;
  return offset;
}

void* arena_get_slice(struct Arena* arena, struct Slice slice) {
  return (char*) arena->data + slice.offset;
}
