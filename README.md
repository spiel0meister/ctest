# ctest

Simple testing framework for C.

## Example

Let's say you have such function somewhere:
```c
int add(int a, int b) {
    return a + b;
}
```

A test file would look like this:
```c
#define TEST_IMPLEMENTATION
#include "ctest.h"

Test(add_test) {
    test_assert_eq(add(1, 1), 2);

    return TEST_SUCCESS;
}

int main(void) {
    test_register(add_test);

    test_run_all_sync();

    return 0;
}
```

Then you compile the file into an executable and just run it:
```sh
gcc -o test test.c
./test
```
