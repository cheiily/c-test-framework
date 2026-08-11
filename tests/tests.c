
// Uncomment these when desired.
// #define TESTS_EXIT_ON_SIGNAL
// #define TESTS_UNSAFE_DIAG

#include "tests_framework.c"
#include "../program/program.h"
#include <unistd.h>


test_case_result test_sample() {
    return (test_case_result) {
        .status = SUCCESS,
    };
}
TEST_CASE("sample test case", test_sample);

test_case_result test_raise() {
    raise(SIGSEGV);
};
TEST_CASE("raise signal error test case", test_raise);


test_case_result test_program_returns_0() {
    int ret = invoke(0, nullptr);
    return (test_case_result) {
        .status = ret == 0 ? SUCCESS : FAILURE,
        .message = ret == 0 ? nullptr : "Program did not return 0"
    };
}
TEST_CASE("program should return 0", test_program_returns_0);

test_case_result test_sleep() {
    sleep(1);
    return (test_case_result) {
        .status = SUCCESS,
    };
}
TEST_CASE("should sleep for 1 second", test_sleep);

test_case_result test_long_loop() {
    bool acc = false;
    for (int i = 0; i < 1'000'000'000; ++i) {
        acc = !acc;
    }
    printf("%i\n", acc);
    return (test_case_result) {
        .status = SUCCESS,
    };
}
TEST_CASE("should execute a long loop to delay the execution", test_long_loop);


int main(void) {
    RUN_TESTS(
        test_sample_case,
        test_program_returns_0_case,
        test_sleep_case,
        test_long_loop_case,
        test_raise_case
    )

    return test_exit_code;
}
