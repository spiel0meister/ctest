#define TEST_IMPLEMENTATION
#include "../ctest.h"

#include <unistd.h>

int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

int mul(int a, int b) {
    return a * b;
}

int div_(int a, int b) {
    return a / b;
}

int randnum(int min, int max) {
    return rand() % (max - min + 1) + min;
}

Test(add_test) {
    test_assert_eq(add(1, 1), 2);
    test_assert_nq(add(1, 1), 3);

    test_assert_lt(add(1, 2), 4);
    test_assert_lte(add(1, 2), 3);
    test_assert_gt(add(1, 2), 2);
    test_assert_gte(add(1, 2), 3);

    return TEST_SUCCESS;
}

Test(sub_test) {
    test_assert_eq(sub(1, 1), 0);
    test_assert_nq(sub(1, 1), 1);

    test_assert_lt(sub(1, 1), 1);

    return TEST_SUCCESS;
}

Test(mul_test) {
    test_assert_eq(mul(1, 1), 1);
    test_assert_eq(mul(1, 0), 0);
    test_assert_eq(mul(0, 1), 0);

    return TEST_SUCCESS;
}

Test(div_test) {
    test_assert_eq(div_(1, 1), 1);
    test_assert_eq(div_(2, 2), 1);

    return TEST_SUCCESS;
}

Test(randnum_test) {
    test_assert_in_range(int, randnum(0, 10), 0, 10);
    test_assert_in_range(int, randnum(0, 100), 0, 100);

    return TEST_SUCCESS;
}

int main(void) {
    test_register(add_test);
    test_register(sub_test);
    test_register(mul_test);
    test_register(div_test);
    test_register(randnum_test);

    test_run_all_sync(true);

    return 0;
}
