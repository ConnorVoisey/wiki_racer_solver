#include "header.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void str_advance_to(struct Str* initial_str, char* new_add) {
  int64_t diff = new_add - initial_str->data;
  if (diff >= 0) {
    initial_str->data = new_add;
    initial_str->length -= diff;
  } else if (diff > initial_str->length) {
    perror("Called str_advance_to to pointer outside the bounds of this str");
  } else {
    perror("Called str_advance_to on unrelated buffer");
    exit(1);
  }
}
