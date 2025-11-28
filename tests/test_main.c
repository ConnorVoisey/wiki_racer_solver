#include "../src/header.h"
#include "munit.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ====== Helper Functions ======

// Verify Slice points to expected string
static void assert_slice_equals(struct Interner* interner, struct Slice slice,
                                const char* expected) {
  const char* actual = (const char*) arena_get_slice(&interner->arena, slice);
  munit_assert_string_equal(actual, expected);
  munit_assert_size(slice.length, ==, strlen(expected));
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
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

static const MunitSuite test_suite = {(char*) "/wiki_racer_tests",
                                      test_suite_tests, NULL, 1,
                                      MUNIT_SUITE_OPTION_NONE};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  return munit_suite_main(&test_suite, (void*) "wiki_racer", argc, argv);
}
