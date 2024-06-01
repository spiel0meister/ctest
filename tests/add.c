#define TEST_IMPLEMENTATION
#include "../test.h"

int add(int a, int b) {
    return a + b;
}

bool add_test(void) {
    test_assert_eq(add(0, 0), 0);
    test_assert_eq(add(1, 2), 3);
    test_assert_eq(add(1, 1), 3);

    return true;
}

int main(void) {
    test_register(add_test);

    return 0;
}
