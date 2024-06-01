#ifndef TEST_H
#define TEST_H
#include <stdbool.h>
#include <stddef.h>

typedef bool (*test_function_t)(void);
typedef struct Test Test;

#define test_assert(expr) if (!(expr)) { \
        error_buffer_write("Assertion failed: '%s' is false\n", #expr); \
        return false; \
    }

#define test_assert_neg(expr) if (expr) { \
        error_buffer_write("Assertion failed: '%s' is true\n", #expr); \
        return false; \
    }

#define test_assert_eq(left, right) if ((left) != (right)) { \
        error_buffer_write("Assertion failed: left is different from right (left: %s, right: %s)\n", #left, #right); \
        return false; \
    }

#define test_assert_nq(left, right) if ((left) == (right)) { \
        error_buffer_write("Assertion failed: left is equal to right (left: %s, right: %s)\n", #left, #right); \
        return false; \
    }

void error_buffer_write(const char* fmt, ...);

void test_register_(const char* name, test_function_t test);
#define test_register(f) test_register_(#f, f)

void test_run_all_(char* file);
#define test_run_all() test_run_all_(__FILE__)
#endif // TEST_H

#ifdef TEST_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

struct Test {
    const char* name;
    test_function_t test;
};

#define ERRORBUF_LEN 1024
static char error_buffer[ERRORBUF_LEN] = {};
void error_buffer_write(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(error_buffer, ERRORBUF_LEN - 1, fmt, args);
    va_end(args);
}

#ifndef TESTS_MAX
    #define TESTS_MAX 1024
#endif // TESTS_MAX

static Test tests_global[TESTS_MAX] = {};
static size_t tests_count = 0;
void test_register_(const char* name, test_function_t test) {
    assert(tests_count < TESTS_MAX);
    tests_global[tests_count++] = (Test){ name, test };
}

void test_run_all_(char* file) {
    size_t success = 0;
    size_t failed = 0;
    for (size_t i = 0; i < tests_count; i++) {
        if (tests_global[i].test()) {
            printf("\x1b[1;32mPASS\x1b[1;0m: %s\n", tests_global[i].name);
            success++;
        } else {
            printf("\x1b[1;31mFAILURE\x1b[1;0m: %s: %s", tests_global[i].name, error_buffer);
            failed++;
        }
    }
    printf("%s: %ld tests: %ld pass, %ld fail\n", file, tests_count, success, failed);
}
#endif
