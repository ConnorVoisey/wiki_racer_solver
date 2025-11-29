#include "header.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parses links within the buffer and adds them into the interner and the edges.
// When the start of a link exists in the buffer but isn't returned, a pointer
// to the start of the link is returned. Otherwise NULL is returned
char* parse_links(char* buf, uint32_t len, struct Interner* interner,
                  struct VecSlice* edges, uint32_t from_id) {

  printf("called parse_links: %u\n", len);
  char* found = buf;
  while ((found = memchr(found, '[', len)) != NULL) {
    len -= found - buf;
    if (found[1] != '[') {
      found += 1;
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

static inline char* parse_buffer(char* buf, uint32_t len,
                                 struct Interner* interner,
                                 struct VecSlice* edges, uint32_t* from_id) {
  printf("called parse_buffer: %u\n", len);
  char* open_tag = memchr(buf, '<', len);
  // TODO: could replace this with a simd check instead, maybe this would be
  // move overhead
  if (open_tag[1] == 't' && open_tag[2] == 'i' && open_tag[3] == 't' &&
      open_tag[4] == 'l' && open_tag[5] == 'e') {
    // Is title tag
    // TODO: check buffer overflow
    char* tag_end = memchr(buf, '>', len);
    char* tag_close_start = memchr(buf, '<', len);
    *from_id =
        intern_from_cstr(interner, tag_end + 1, tag_close_start - tag_end - 1);

  } else if (open_tag[1] == 't' && open_tag[2] == 'e' && open_tag[3] == 'x' &&
             open_tag[4] == 't') {
    // Is text tag
    char* extra_links = parse_links(buf, len, interner, edges, *from_id);
    if (extra_links == NULL) {
      return extra_links;
    }
    // Not sure about this, we need the opening < so that it can recall this
    // function but it seems inefficent, maybe a buffer overhang might be more
    // efficent
  }

  return NULL;
}

void print_progress(size_t count, size_t max) {
  const int bar_width = 50;

  float progress = (float) count / max;
  int bar_length = progress * bar_width;

  printf("\rProgress: [");
  for (int i = 0; i < bar_length; ++i) {
    putchar('=');
  }
  putchar('>');
  for (int i = bar_length; i < bar_width - 1; ++i) {
    putchar(' ');
  }
  printf("] %.2f%%", progress * 100);

  fflush(stdout);
}

// builds the graph and writes it to the output graph file
int build_graph() {
  FILE* xml_file = fopen(XML_FILE_PATH, "r");
  if (xml_file == NULL) {
    perror("Failed to open xml file");
    return 1;
  }

  fseek(xml_file, 0, SEEK_END);
  uint64_t file_size = ftell(xml_file);
  fseek(xml_file, 0, SEEK_SET);

  char* buf = malloc(BUFF_SIZE);
  uint64_t buf_offset = 0;
  uint64_t amount_read = 0;
  uint64_t amount_read_total = 0;
  uint32_t from_id = UINT32_MAX;
  struct Interner interner = interner_init(1 << 20);
  struct VecSlice edges = vec_slice_init(1 << 20);

  while ((amount_read = fread(buf + buf_offset, 1, BUFF_SIZE, xml_file)) > 0) {
    amount_read_total += amount_read;
    printf("\nbuf_offset: %lu\n", buf_offset);

    char* buffer_end =
        parse_buffer(buf, BUFF_SIZE, &interner, &edges, &from_id);
    if (buffer_end) {
      printf("Returning pointer offset %ld\n", buffer_end - buf);
    }

    if (buffer_end != NULL) {
      puts("is null");
      buf_offset = buffer_end - buf;
      memmove(buf, buffer_end, buf_offset);
    } else {
      buf_offset = 0;
    }
    // print_progress(amount_read_total, file_size);
  }
  putchar('\n');

  return 0;
}
