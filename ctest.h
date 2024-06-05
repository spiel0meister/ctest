/*
 *  Copyright 2024 Žan Sovič <soviczan7@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TEST_H
#define TEST_H
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    SUCCESS = 0,

    ASSERT,
    ASSERT_NOT,
    ASSERT_NULL,
    ASSERT_NOT_NULL,
    ASSERT_BOOL,
    ASSERT_RANGE
}TestResultType;


typedef struct {
    const char* left;
    const char* right;
    const char* msg;
}BooleanOp;

typedef struct {
    const char* min;
    const char* max;
    const char* value;
}RangeAssert;

typedef union {
    const char* assert;
    BooleanOp assert_bool;
    RangeAssert assert_range;
}TestResultAs;

typedef struct {
    bool success;
    char* msg;

    const char* file;
    int line;
}TestResult;

#define TEST_SUCCESS ((TestResult) { .success = true })

typedef TestResult (*test_function_t)(void);
typedef struct Test Test;

#define test_assert(expr) if (!(expr)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: '%s' is false", #expr), __FILE__, __LINE__ }); \
    }

#define test_assert_not(expr) if (expr) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: '%s' is true", #expr), __FILE__, __LINE__ }); \
    }

#define test_assert_eq(left, right) if ((left) != (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is different from right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_str_eq(left, right) if (strcmp(left, right) != 0) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is different from right (left: '%s', right: '%s')", left, right), __FILE__, __LINE__ }); \
    }

#define test_assert_nq(left, right) if ((left) == (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is equal to right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_str_nq(left, right) if (strcmp(left, right) == 0) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is equal to right (left: '%s', right: '%s')", left, right), __FILE__, __LINE__ }); \
    }

#define test_assert_lt(left, right) if ((left) >= (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is greater or equal to right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_gt(left, right) if ((left) <= (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is less or equal to right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_lte(left, right) if ((left) > (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is greater than right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_gte(left, right) if ((left) < (right)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: left is less than right (left: '%s', right: '%s')", #left, #right), __FILE__, __LINE__ }); \
    }

#define test_assert_in_range(min, max, value) if ((value) < (min) || (value) > (max)) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: value is out of range (min: '%s', max: '%s', value: '%s')", #min, #max, #value), __FILE__, __LINE__ }); \
    }

#define test_assert_null(value) if ((value) != NULL) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: '%s' is not null", #value), __FILE__, __LINE__ }); \
    }

#define test_assert_not_null(value) if ((value) == NULL) { \
        return ((TestResult) { false, test_sprintf("Assertion failed: '%s' is null", #value), __FILE__, __LINE__ }); \
    }

#define Test(name) TestResult name(void)

__attribute__((format (printf, 1, 2)))
    char* sprintf_(const char* fmt, ...);

void test_register_(const char* name, test_function_t test);
#define test_register(f) test_register_(#f, f)

void test_run_all_async_(const char* file, bool print_passes);
#define test_run_all_async(print_passes) test_run_all_async_(__FILE__, print_passes)

void test_run_all_sync_(const char* file, bool print_passes);
#define test_run_all_sync(print_passes) test_run_all_sync_(__FILE__, print_passes)
#endif // TEST_H

#ifdef TEST_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

char* test_sprintf(const char* fmt, ...) {
    char buf[1024] = {};
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, 1024, fmt, args);
    va_end(args);
    assert(n < 1024);
    return strdup(buf);
}

struct Test {
    const char* name;
    test_function_t test;
    size_t i;
};

#ifndef TESTS_MAX
    #define TESTS_MAX 128
#endif // TESTS_MAX

static Test tests_global[TESTS_MAX] = {};
static size_t tests_count = 0;
void test_register_(const char* name, test_function_t test) {
    assert(tests_count < TESTS_MAX);
    size_t i = tests_count++;
    tests_global[i] = (Test){ name, test, i };
}

TestResult results[TESTS_MAX] = {};
void* test_run_job(void* arg) {
    Test* test = (Test*)arg;
    results[test->i] = tests_global[test->i].test();
    return &results[test->i];
}

static bool print_passes;
static void print_result(size_t i, TestResult* result) {
    if (result->success) {
        if (print_passes) printf("\x1b[1;32mPASS\x1b[1;0m: %s\n", tests_global[i].name);
    } else {
        printf("\x1b[1;31mFAILURE\x1b[1;0m: %s: %s:%d: %s\n", tests_global[i].name, result->file, result->line, result->msg);
        free(result->msg);
        result->msg = NULL;
    }
}

void test_run_all_async_(const char* file, bool print_passes_) {
    size_t success = 0;
    size_t failed = 0;
    print_passes = print_passes_;

    pthread_t threads[TESTS_MAX] = {};
    for (size_t i = 0; i < tests_count; i++) {
        pthread_create(&threads[i], NULL, test_run_job, &tests_global[i]);
    }

    for (size_t i = 0; i < tests_count; i++) {
        TestResult* result;
        pthread_join(threads[i], (void**)&result);
        print_result(i, result);
        if (result->success) success++;
        else failed++;
    }
    printf("%s: %ld tests: %ld \x1b[1;32mPASS\x1b[1;0m, %ld \x1b[1;31mFAIL\x1b[1;0m\n", file, tests_count, success, failed);
}

void test_run_all_sync_(const char* file, bool print_passes_) {
    size_t success = 0;
    size_t failed = 0;
    print_passes = print_passes_;
    for (size_t i = 0; i < tests_count; i++) {
        TestResult* result = test_run_job(&tests_global[i]);
        print_result(i, result);
        if (result->success) success++;
        else failed++;
    }
    printf("%s: %ld tests: %ld \x1b[1;32mPASS\x1b[1;0m, %ld \x1b[1;31mFAIL\x1b[1;0m\n", file, tests_count, success, failed);
}
#endif
