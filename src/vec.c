#include "header.h"

struct VecSlice vec_slice_init(unsigned long capacity) {
  return (struct VecSlice) {
      .capacity = capacity,
      .length = 0,
      .data = malloc(capacity),
  };
}

void vec_slice_push(struct VecSlice* vec, struct Slice val) {
  int has_grown = 0;
  while (vec->length + 1 >= vec->capacity) {
    vec->capacity *= 2;
    has_grown = 1;
  }
  if (has_grown) {
    vec->data = realloc(vec->data, vec->capacity);
  }

  vec->data[vec->length] = val;
  vec->length += 1;
}
