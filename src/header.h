#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XML_FILE_PATH "inputs/enwiki-20251101-pages-articles-multistream.xml"
#define BUFF_SIZE 8388608 // 8mb
#define OVERLAP 512       // Bigger than the largest link

struct Arena {
  void* data;
  uint32_t length;
  uint32_t capacity;
};

struct Slice {
  uint32_t offset;
  uint32_t length;
};

struct Edge {
  uint32_t from;
  uint32_t to;
};

struct VecSlice {
  struct Slice* data;
  uint32_t capacity;
  uint32_t length;
};

// ====== Interner ===== //

struct Interner {
  struct Arena arena;
  struct VecSlice strs;
  // HashMap map;
};

struct Interner interner_init(uint32_t capacity);
void interner_destroy(struct Interner* interner);
void interner_set_arena_context(struct Arena* arena);
uint32_t intern_from_cstr(struct Interner* interner, const char* s,
                              size_t len);
// Ultra simple progess bar
void print_progress(size_t count, size_t max);

// ====== Arena ===== //
struct Arena arena_init(unsigned long capacity);
unsigned long arena_push(struct Arena* arena, void* contents,
                         unsigned long length);
void* arena_get_slice(struct Arena* arena, struct Slice slice);

// ====== Vec ====== //
struct VecSlice vec_slice_init(unsigned long capacity);
void vec_slice_push(struct VecSlice* vec, struct Slice val);

// Parses links within the buffer and adds them into the interner and the edges.
// When the start of a link exists in the buffer but isn't returned, a pointer
// to the start of the link is returned. Otherwise NULL is returned
char* parse_links(char* buf, uint32_t len, struct Interner* interner,
                  struct VecSlice* edges, uint32_t from_id);
