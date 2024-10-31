#include <string.h>

#define TEST_IMPLEMENTATION
#include "ctest.h"

// Function to find a substring
int strfind(const char* haystack, const char* needle) {
    for (size_t i = 0; i < strlen(haystack); ++i) {
        if (strcmp(&haystack[i], needle) == 0) return i;
    }

    return -1;
}

// Test for strfind
Test(strfind_test) {
    const char* haystack = "hello, world";
    const char* needle = "world";
    int result = strfind(haystack, needle);
    test_assert_eq(result, 7);

    return TEST_SUCCESS;
}

int main(void) {
    // Register tests
    test_register(strfind_test);
    // more...

    // Run tests
    test_run_all_sync();
    return 0;
}
