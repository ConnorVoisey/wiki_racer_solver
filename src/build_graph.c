#include "header.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parses links within the buffer and adds them into the interner and the edges.
// When the start of a link exists in the buffer but isn't returned, a pointer
// to the start of the link is returned. Otherwise NULL is returned
char* parse_links(struct Str* buf, struct Interner* interner,
                  struct VecEdge* edges, uint32_t from_id) {
  uint32_t len = buf->length;
  printf("called parse_links: %u\n", len);
  char* found = buf->data;
  while ((found = memchr(found, '[', len)) != NULL) {
    str_advance_to(buf, found);
    printf("called parse_links inner: %u. %s\n", len, found);
    if (found[1] != '[') {
      found += 1;
      puts("parse_links: continued");
      continue;
    }
    found += 2;
    str_advance_to(buf, found);

    // we assume that no links have a ] in them
    char* link_close = memchr(found, ']', len);
    if (link_close == NULL) {
      puts("parse_links: found returned");
      return found - 2;
    }

    // we assume that no links have a | in them
    char* label_start = memchr(found, '|', link_close - found);
    char* link_title = found;
    uint32_t link_len =
        label_start == NULL ? link_close - found : label_start - found;

    uint32_t to_id = intern_from_cstr(interner, link_title, link_len);

    struct Edge edge = {from_id, to_id};
    vec_edge_push(edges, edge);
  }
  return NULL;
}

char* parse_buffer(struct Str* buf, struct Interner* interner,
                   struct VecEdge* edges, uint32_t* from_id) {
  printf("called parse_buffer: %u\n", buf->length);
  // TODO: do math to reduce the len when we move the buffer pointer
  char* open_tag = NULL;
  // TODO: could replace this with a simd check instead, maybe this would be
  // move overhead
  while ((open_tag = memchr(buf->data, '<', buf->length))) {
    puts("HERE");
    if (open_tag[1] == 't' && open_tag[2] == 'i' && open_tag[3] == 't' &&
        open_tag[4] == 'l' && open_tag[5] == 'e') {
      // Is title tag
      puts("is title");

      str_advance_to(buf, open_tag + 6);

      char* tag_end = memchr(buf->data, '>', buf->length);
      puts("tag_end");
      if (tag_end == NULL) {
        return open_tag;
      }
      char* tag_close_start = memchr(buf->data, '<', buf->length);
      if (tag_close_start == NULL) {
        return open_tag;
      }
      puts("tag_close_start");
      *from_id = intern_from_cstr(interner, tag_end + 1,
                                  tag_close_start - tag_end - 1);
      // TODO: add test showing that this should be returned, it currenty isn't
      puts("from_id");

    } else if (open_tag[1] == 't' && open_tag[2] == 'e' && open_tag[3] == 'x' &&
               open_tag[4] == 't') {
      puts("is text");
      // Is text tag
      char* extra_links = parse_links(buf, interner, edges, *from_id);
      if (extra_links != NULL) {
        puts("Returning");
        return extra_links;
      }
      // Not sure about this, we need the opening < so that it can recall this
      // function but it seems inefficent, maybe a buffer overhang might be more
      // efficent
    } else {
      str_advance_to(buf, open_tag + 1);
    }
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

int build_graph_inner(FILE* xml_file, uint64_t buff_size,
                      struct Interner* interner, struct VecEdge* edges,
                      char* output_path) {
  fseek(xml_file, 0, SEEK_END);
  uint64_t file_size = ftell(xml_file);
  fseek(xml_file, 0, SEEK_SET);

  char* buf = malloc(buff_size);
  uint64_t buf_offset = 0;
  uint64_t amount_read = 0;
  uint64_t amount_read_total = 0;
  uint32_t from_id = UINT32_MAX;

  while ((amount_read = fread(buf + buf_offset, 1, buff_size, xml_file)) > 0) {
    amount_read_total += amount_read;
    // printf("\nbuf_offset: %lu\n", buf_offset);

    struct Str str = {.data = buf, .length = buff_size};
    char* buffer_end = parse_buffer(&str, interner, edges, &from_id);
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
    print_progress(amount_read_total, file_size);
  }
  // TODO: sort the edges by from
  // TODO: write the edges
  // TODO: write the interner
  return 0;
}

// builds the graph and writes it to the output graph file
int build_graph() {
  FILE* xml_file = fopen(XML_FILE_PATH, "r");
  if (xml_file == NULL) {
    perror("Failed to open xml file");
    return 1;
  }

  struct Interner interner = interner_init(1 << 20);
  struct VecEdge edges = vec_edge_init(1 << 20);

  char* output_path = "inputs/graph.bin";

  return build_graph_inner(xml_file, BUFF_SIZE, &interner, &edges, output_path);
}
