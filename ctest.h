#ifndef TEST_H
#define TEST_H
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    SUCCESS = 0,

    ASSERT,
    ASSERT_NEG,
    ASSERT_EQ,
    ASSERT_NQ,
    ASSERT_LT,
    ASSERT_GT,
    ASSERT_LTE,
    ASSERT_GTE,
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
    const char* assert_neg;
    BooleanOp assert_bool;
    RangeAssert assert_range;
}TestResultAs;

typedef struct {
    TestResultType type;
    TestResultAs as;

    const char* file;
    int line;
}TestResult;

#define TEST_SUCCES ((TestResult) { SUCCESS, {0}, 0, 0 })

typedef TestResult (*test_function_t)(void);
typedef struct Test Test;

//error_buffer_write("%s:%d: Assertion failed: '%s' is false\n", __FILE__, __LINE__, #expr);
#define test_assert(expr) if (!(expr)) { \
        return ((TestResult) { ASSERT, .as.assert = #expr, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: '%s' is true\n", __FILE__, __LINE__, #expr);
#define test_assert_neg(expr) if (expr) { \
        return ((TestResult) { ASSERT_NEG, .as.assert_neg = #expr, __FILE__, __LINE__  }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is different from right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_eq(left, right) if ((left) != (right)) { \
        return ((TestResult) { ASSERT_EQ, .as.assert_bool = { #left, #right, "left is different from right" }, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is equal to right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_nq(left, right) if ((left) == (right)) { \
        return ((TestResult) { ASSERT_NQ, .as.assert_bool = { #left, #right, "left is equal to right" }, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is greater or equal to right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_lt(left, right) if ((left) >= (right)) { \
        return ((TestResult) { ASSERT_LT, .as.assert_bool = { #left, #right, "left is greater or equal to right" }, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is less or equal to right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_gt(left, right) if ((left) <= (right)) { \
        return ((TestResult) { ASSERT_GT, .as.assert_bool = { #left, #right, "left is less or equal to right" }, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is greater than right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_lte(left, right) if ((left) > (right)) { \
        return ((TestResult) { ASSERT_LTE, .as.assert_bool = { #left, #right, "left is greater than right" }, __FILE__, __LINE__ }); \
    }

// error_buffer_write("%s:%d: Assertion failed: left is less than right (left: %s, right: %s)\n", __FILE__, __LINE__, #left, #right);
#define test_assert_gte(left, right) if ((left) < (right)) { \
        return ((TestResult) { ASSERT_GTE, .as.assert_bool = { #left, #right, "left is less than right" }, __FILE__, __LINE__ }); \
    }

#define test_assert_in_range(min, max, value) if (value < (min) || value > (max)) { \
        return ((TestResult) { ASSERT_RANGE, .as.assert_range = { #min, #max, #value }, __FILE__, __LINE__ }); \
    }

void error_buffer_write(const char* fmt, ...);

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
#include <stdarg.h>
#include <pthread.h>

struct Test {
    const char* name;
    test_function_t test;
    size_t i;
};

#ifndef TESTS_MAX
    #define TESTS_MAX 1024
#endif // TESTS_MAX

static Test tests_global[TESTS_MAX] = {};
static size_t tests_count = 0;
void test_register_(const char* name, test_function_t test) {
    assert(tests_count < TESTS_MAX);
    size_t i = tests_count++;
    tests_global[i] = (Test){ name, test, i };
}

static bool print_passes;

TestResult results[TESTS_MAX] = {};
void* test_run_job(void* arg) {
    Test* test = (Test*)arg;
    TestResult result = tests_global[test->i].test();
    results[test->i] = result;
    return &results[test->i];
}

static void print_result(size_t i, TestResult* result) {
    if (result->type == SUCCESS) {
        if (print_passes) printf("\x1b[1;32mPASS\x1b[1;0m: %s\n", tests_global[i].name);
    } else {
        printf("\x1b[1;31mFAILURE\x1b[1;0m: %s: %s:%d: ", tests_global[i].name, result->file, result->line);
        switch (result->type) {
            case ASSERT:
                printf("Assertion Failed: '%s' is false\n", result->as.assert);
                break;
            case ASSERT_NEG:
                printf("Assertion Failed: '%s' is true\n", result->as.assert_neg);
                break;
            case ASSERT_EQ:
            case ASSERT_NQ:
            case ASSERT_LT:
            case ASSERT_GT:
            case ASSERT_LTE:
            case ASSERT_GTE:
                BooleanOp bool_op = result->as.assert_bool;
                printf("Assertion Failed: %s (left is '%s', right is '%s')\n", bool_op.msg, bool_op.left, bool_op.right);
                break;
            case ASSERT_RANGE:
                RangeAssert range_assert = result->as.assert_range;
                printf("Assertion Failed: value is out of range (value is '%s', min is '%s', max is '%s')\n", range_assert.value, range_assert.min, range_assert.max);
                break;
            case SUCCESS: 
                assert(0);
            default:
                assert(0);
        }
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
        if (result->type == SUCCESS) success++;
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
        if (result->type == SUCCESS) success++;
        else failed++;
    }
    printf("%s: %ld tests: %ld \x1b[1;32mPASS\x1b[1;0m, %ld \x1b[1;31mFAIL\x1b[1;0m\n", file, tests_count, success, failed);
}
#endif
