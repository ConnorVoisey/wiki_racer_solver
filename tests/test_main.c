#include "../src/header.h"
#include "munit.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ====== Helper Functions ======

// Verify Slice points to expected string
static void assert_slice_equals(struct Interner* interner, struct Slice slice,
                                const char* expected) {
  const char* actual = (const char*) arena_get_slice(&interner->arena, slice);
  munit_assert_string_equal(actual, expected);
  munit_assert_size(slice.length, ==, strlen(expected));
}

// Verify edge exists in edges vector
static void assert_edge_exists(struct VecSlice* edges, uint32_t from_id,
                               uint32_t to_id, const char* description) {
  for (uint32_t i = 0; i < edges->length; i++) {
    struct Slice edge = edges->data[i];
    if (edge.offset == from_id && edge.length == to_id) {
      return; // Found it
    }
  }
  munit_errorf("Edge not found: %s (from=%u, to=%u)", description, from_id,
               to_id);
}

// Create temporary file for testing (auto-deleted)
static FILE* create_test_file(const char* content, size_t len) {
  FILE* tmp = tmpfile(); // Auto-deleted on close
  if (tmp == NULL) {
    munit_error("Failed to create temp file");
  }
  fwrite(content, 1, len, tmp);
  rewind(tmp); // Reset to beginning for reading
  return tmp;
}

// Verify edge count
static void assert_edges_count(struct VecSlice* edges, uint32_t expected,
                               const char* context) {
  if (edges->length != expected) {
    munit_errorf("Expected %u edges (%s), got %u", expected, context,
                 edges->length);
  }
}

// Get ID of interned string
static uint32_t get_interned_id(struct Interner* interner, const char* str) {
  uint32_t id = intern_from_cstr(interner, str, strlen(str));
  return id;
}

// Create test buffer with content
static char* create_buffer_with_content(const char* content, size_t* out_len) {
  size_t len = strlen(content);
  char* buf = malloc(len);
  memcpy(buf, content, len);
  *out_len = len;
  return buf;
}

// Check if Slice exists in Vec
static short slice_in_vec(struct Interner* interner, struct Slice slice) {
  for (uint32_t i = 0; i < interner->strs.length; i++) {
    struct Slice* vec_slice = &interner->strs.data[i];
    if (vec_slice->offset == slice.offset &&
        vec_slice->length == slice.length) {
      return 1;
    }
  }
  return 0;
}

/* ====== Interner Tests ====== */

/* Test 1: Intern a single string */
static MunitResult test_interner_single_string(const MunitParameter params[],
                                               void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  // Intern one string
  const char* test_str = "hello";
  uint32_t index = intern_from_cstr(&interner, test_str, strlen(test_str));
  struct Slice slice = interner.strs.data[index];

  // Verify the slice has valid offset and length
  munit_assert_uint32(slice.length, ==, 5);

  // Verify string content is correct
  assert_slice_equals(&interner, slice, "hello");

  // // Verify it's in the hashmap
  // munit_assert_true(slice_in_hashmap(&interner, slice));

  // Verify it's in the vec
  munit_assert_true(slice_in_vec(&interner, slice));

  // Verify vec has exactly 1 entry
  munit_assert_uint32(interner.strs.length, ==, 1);

  // // Verify hashmap has exactly 1 entry
  // munit_assert_size(HashMap_size(&interner.map), ==, 1);

  interner_destroy(&interner);
  return MUNIT_OK;
}

/* Test 2: Intern three unique strings */
static MunitResult
test_interner_three_unique_strings(const MunitParameter params[], void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  // Intern three different strings
  const char* str1 = "apple";
  const char* str2 = "banana";
  const char* str3 = "cherry";

  uint32_t s1_index = intern_from_cstr(&interner, str1, strlen(str1));
  struct Slice s1 = interner.strs.data[s1_index];
  uint32_t s2_index = intern_from_cstr(&interner, str2, strlen(str2));
  struct Slice s2 = interner.strs.data[s2_index];
  uint32_t s3_index = intern_from_cstr(&interner, str3, strlen(str3));
  struct Slice s3 = interner.strs.data[s3_index];

  // Verify all three are different slices
  munit_assert_true(s1.offset != s2.offset || s1.length != s2.length);
  munit_assert_true(s2.offset != s3.offset || s2.length != s3.length);
  munit_assert_true(s1.offset != s3.offset || s1.length != s3.length);

  // Verify contents
  assert_slice_equals(&interner, s1, "apple");
  assert_slice_equals(&interner, s2, "banana");
  assert_slice_equals(&interner, s3, "cherry");

  // // Verify all are in hashmap
  // munit_assert_true(slice_in_hashmap(&interner, s1));
  // munit_assert_true(slice_in_hashmap(&interner, s2));
  // munit_assert_true(slice_in_hashmap(&interner, s3));

  // Verify all are in vec
  munit_assert_true(slice_in_vec(&interner, s1));
  munit_assert_true(slice_in_vec(&interner, s2));
  munit_assert_true(slice_in_vec(&interner, s3));

  // Verify counts
  munit_assert_uint32(interner.strs.length, ==, 3);
  // munit_assert_size(HashMap_size(&interner.map), ==, 3);

  interner_destroy(&interner);
  return MUNIT_OK;
}

/* Test 3: Intern three strings with two being the same */
static MunitResult test_interner_with_duplicates(const MunitParameter params[],
                                                 void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  // Intern: "foo", "bar", "foo" (duplicate)
  const char* str1 = "foo";
  const char* str2 = "bar";

  uint32_t s1_index = intern_from_cstr(&interner, str1, strlen(str1));
  struct Slice s1 = interner.strs.data[s1_index];
  uint32_t s2_index = intern_from_cstr(&interner, str2, strlen(str2));
  struct Slice s2 = interner.strs.data[s2_index];
  uint32_t s3_index = intern_from_cstr(&interner, str1, strlen(str1));
  struct Slice s3 = interner.strs.data[s3_index];

  // Verify s1 and s3 are identical (same offset and length)
  munit_assert_uint32(s1.offset, ==, s3.offset);
  munit_assert_uint32(s1.length, ==, s3.length);

  // Verify s1 and s2 are different
  munit_assert_true(s1.offset != s2.offset || s1.length != s2.length);

  // Verify contents
  assert_slice_equals(&interner, s1, "foo");
  assert_slice_equals(&interner, s2, "bar");
  assert_slice_equals(&interner, s3, "foo");

  // Verify only 2 unique entries in collections
  munit_assert_uint32(interner.strs.length, ==, 2);
  // munit_assert_size(HashMap_size(&interner.map), ==, 2);

  interner_destroy(&interner);
  return MUNIT_OK;
}

/* Test 4: Intern empty string */
static MunitResult test_interner_empty_string(const MunitParameter params[],
                                              void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  const char* empty = "";
  uint32_t index = intern_from_cstr(&interner, empty, 0);
  struct Slice slice = interner.strs.data[index];

  // Verify length is 0
  munit_assert_uint32(slice.length, ==, 0);

  // Verify it's stored
  // munit_assert_true(slice_in_hashmap(&interner, slice));
  munit_assert_true(slice_in_vec(&interner, slice));

  // Verify counts
  munit_assert_uint32(interner.strs.length, ==, 1);
  // munit_assert_size(HashMap_size(&interner.map), ==, 1);

  interner_destroy(&interner);
  return MUNIT_OK;
}

/* Test 5: Intern the same string multiple times */
static MunitResult
test_interner_multiple_duplicates(const MunitParameter params[], void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  const char* str = "repeated";

  // Intern the same string 5 times
  struct Slice slices[5];
  for (int i = 0; i < 5; i++) {
    uint32_t index = intern_from_cstr(&interner, str, strlen(str));
    slices[i] = interner.strs.data[index];
  }

  // Verify all returned slices are identical
  for (int i = 1; i < 5; i++) {
    munit_assert_uint32(slices[0].offset, ==, slices[i].offset);
    munit_assert_uint32(slices[0].length, ==, slices[i].length);
  }

  // Verify only one copy stored
  munit_assert_uint32(interner.strs.length, ==, 1);
  // munit_assert_size(HashMap_size(&interner.map), ==, 1);

  interner_destroy(&interner);
  return MUNIT_OK;
}

/* Test 6: Intern a long string */
static MunitResult test_interner_long_string(const MunitParameter params[],
                                             void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);

  const char* long_str = "This is a much longer string that should still be "
                         "interned correctly without any issues";
  uint32_t index = intern_from_cstr(&interner, long_str, strlen(long_str));
  struct Slice slice = interner.strs.data[index];

  // Verify length
  munit_assert_uint32(slice.length, ==, strlen(long_str));

  // Verify content
  assert_slice_equals(&interner, slice, long_str);

  // Verify storage
  // munit_assert_true(slice_in_hashmap(&interner, slice));
  munit_assert_true(slice_in_vec(&interner, slice));

  interner_destroy(&interner);
  return MUNIT_OK;
}
static MunitResult test_str_advance_normal(const MunitParameter params[],
                                           void* data) {
  (void) params;
  (void) data;

  char* contents = "Example str <tag>";
  struct Str str = {.data = contents, .length = strlen(contents)};

  {
    str_advance_to(&str, str.data);
    munit_assert_ptr_equal(str.data, contents);
  }

  {
    char* new_offset = memchr(str.data, '<', str.length);
    str_advance_to(&str, new_offset);
    munit_assert_ptr_equal(str.data, new_offset);
  }

  {
    char* new_offset = memchr(str.data, '>', str.length);
    str_advance_to(&str, new_offset);
    munit_assert_ptr_equal(str.data, new_offset);
  }

  return MUNIT_OK;
}

/* ====== Parse Links Tests ====== */

/* Test 1: Parse a single complete link */
static MunitResult
test_parse_links_single_complete(const MunitParameter params[], void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  const char* content = "Some text [[Link]] more text";
  struct Str str = {.data = content, .length = strlen(content)};
  uint32_t from_id = 1; // Arbitrary page ID
  char* result = parse_links(&str, &interner, &edges, from_id);

  // Should return NULL (no incomplete link)
  munit_assert_null(result);

  // Should have created 1 edge
  assert_edges_count(&edges, 1, "single complete link");

  // Verify the edge points to "Link"
  uint32_t link_id = get_interned_id(&interner, "Link");
  assert_edge_exists(&edges, from_id, link_id, "edge to Link");

  interner_destroy(&interner);
  free(edges.data);
  return MUNIT_OK;
}

/* Test 3: Parse multiple complete links */
static MunitResult
test_parse_links_multiple_complete(const MunitParameter params[], void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  const char* content = "[[Link1]] text [[Link2]]";
  struct Str str = {.data = content, .length = strlen(content)};
  uint32_t from_id = get_interned_id(&interner, "Page");
  char* result = parse_links(&str, &interner, &edges, from_id);

  // Should return NULL (no incomplete link)
  munit_assert_null(result);

  // Should have created 2 edges
  assert_edges_count(&edges, 2, "two complete links");

  // Verify both edges
  char* link_1 = "Link1";
  uint32_t link1_id = intern_from_cstr(&interner, link_1, strlen(link_1));
  char* link_2 = "Link2";
  uint32_t link2_id = intern_from_cstr(&interner, link_2, strlen(link_2));

  assert_edge_exists(&edges, from_id, link1_id, "edge to Link1");
  assert_edge_exists(&edges, from_id, link2_id, "edge to Link2");

  interner_destroy(&interner);
  free(edges.data);
  return MUNIT_OK;
}

/* ====== Parse Buffer Tests ====== */

/* Test 9: Parse buffer with title tag */
static MunitResult test_parse_buffer_title_tag(const MunitParameter params[],
                                               void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  const char* content = "<title>PageName</title>";
  struct Str str = {.data = content, .length = strlen(content)};
  uint32_t from_id = UINT32_MAX; // Should be set by parse_buffer
  char* result = parse_buffer(&str, &interner, &edges, &from_id);

  // Should return NULL (no incomplete link)
  munit_assert_null(result);

  // from_id should be set to "PageName"
  uint32_t expected_id = get_interned_id(&interner, "PageName");
  munit_assert_uint32(from_id, ==, expected_id);

  interner_destroy(&interner);
  free(edges.data);
  return MUNIT_OK;
}

static MunitResult
test_parse_title_across_buffers(const MunitParameter params[], void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  const char* content = "starting noise <title>Page";
  struct Str str = {.data = content, .length = strlen(content)};
  uint32_t from_id = UINT32_MAX;
  char* result = parse_buffer(&str, &interner, &edges, &from_id);

  munit_assert_not_null(result);
  munit_assert_string_equal(result, "<title>Page");

  interner_destroy(&interner);
  free(edges.data);
  return MUNIT_OK;
}

static MunitResult test_parse_link_across_buffers(const MunitParameter params[],
                                                  void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  const char* content = "starting noise <title>Page</title><text>pre-text "
                        "[[first link]] then [[unclosed";
  struct Str str = {.data = content, .length = strlen(content)};
  uint32_t from_id = UINT32_MAX;
  char* result = parse_buffer(&str, &interner, &edges, &from_id);

  munit_assert_not_null(result);
  munit_assert_string_equal(result, "[[unclosed");

  interner_destroy(&interner);
  free(edges.data);
  return MUNIT_OK;
}

static MunitResult test_integration_simple_case(const MunitParameter params[],
                                                void* data) {
  (void) params;
  (void) data;

  struct Interner interner = interner_init(1024);
  struct VecSlice edges = vec_slice_init(128);

  FILE* xml_file = tmpfile();
  const char* content = "starting noise <title>Page</title><text>pre-text "
                        "[[first link]] then [[second link]].</text>";
  struct Str str = {.data = content, .length = strlen(content)};
  fwrite(str.data, 1, str.length, xml_file);
  fseek(xml_file, 0, SEEK_SET);
  build_graph_inner(xml_file, BUFF_SIZE, &interner, &edges,
                    "./tmp_test_out/test_integration_simple_case");
  munit_assert_size(interner.strs.length, ==, 3);

  char* title = "Page";
  uint32_t title_id = intern_from_cstr(&interner, title, strlen(title));
  munit_assert_size(interner.strs.length, ==, 3);
  char* link_1 = "first link";
  uint32_t link_1_id = intern_from_cstr(&interner, link_1, strlen(link_1));
  munit_assert_size(interner.strs.length, ==, 3);
  char* link_2 = "second link";
  uint32_t link_2_id = intern_from_cstr(&interner, link_2, strlen(link_2));
  munit_assert_size(interner.strs.length, ==, 3);

  munit_assert_size(edges.length, ==, 2);
  munit_assert_size(edges.data[0].offset, ==, title_id);
  munit_assert_size(edges.data[0].length, ==, link_1_id);

  munit_assert_size(edges.data[1].offset, ==, title_id);
  munit_assert_size(edges.data[1].length, ==, link_2_id);

  interner_destroy(&interner);
  free(edges.data);
  fclose(xml_file);
  return MUNIT_OK;
}

/* Test suite definition */
static MunitTest test_suite_tests[] = {
    {(char*) "/interner/single_string", test_interner_single_string, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/interner/three_unique", test_interner_three_unique_strings, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/interner/with_duplicates", test_interner_with_duplicates, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/interner/empty_string", test_interner_empty_string, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/interner/multiple_duplicates", test_interner_multiple_duplicates,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/interner/long_string", test_interner_long_string, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/str/advance_normal", test_str_advance_normal, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/parser/links_single_complete", test_parse_links_single_complete,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/parser/links_multiple_complete",
     test_parse_links_multiple_complete, NULL, NULL, MUNIT_TEST_OPTION_NONE,
     NULL},
    {(char*) "/parser/buffer_title_tag", test_parse_buffer_title_tag, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/parser/title_across_buffers", test_parse_title_across_buffers,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/parser/link_across_buffers", test_parse_link_across_buffers,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char*) "/integration/simple_case", test_integration_simple_case, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

static const MunitSuite test_suite = {(char*) "/wiki_racer_tests",
                                      test_suite_tests, NULL, 1,
                                      MUNIT_SUITE_OPTION_NONE};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  return munit_suite_main(&test_suite, (void*) "wiki_racer", argc, argv);
}
