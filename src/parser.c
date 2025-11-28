#include "header.h"
#include <stdint.h>
#include <string.h>

// Parses links within the buffer and adds them into the interner and the edges.
// When the start of a link exists in the buffer but isn't returned, a pointer
// to the start of the link is returned. Otherwise NULL is returned
char* parse_links(char* buf, uint32_t len, struct Interner* interner,
                  struct VecSlice* edges, uint32_t from_id) {

  char* found = buf;
  while ((found = memchr(found, '[', len)) != NULL) {
    len -= found - buf;
    if (found[1] != '[') {
      continue;
    }
    found += 2;

    // we assume that no links have a ] in them
    char* link_close = memchr(found, ']', len);
    if (link_close == NULL) {
      return found;
    }

    // we assume that no links have a | in them
    char* label_start = memchr(found, '|', link_close - found);
    char* link_title = found;
    uint32_t link_len =
        label_start == NULL ? link_close - found : label_start - found;

    uint32_t to_id = intern_from_cstr(interner, link_title, link_len);

    struct Slice edge = {from_id, to_id};
    vec_slice_push(edges, edge);
  }
  return NULL;
}

char* parse_buffer(char* buf, uint32_t len, struct Interner* interner,
                  struct VecSlice* edges, uint32_t from_id) {
}
