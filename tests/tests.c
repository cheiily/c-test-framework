//
// Created by cheily on 7.08.2026.
//

#include "tests_framework.c"
#include "../program/program.c"

test_case_result test_sample() {
    return (test_case_result) {
        .status = SUCCESS,
    };
}
TEST_CASE("sample test case", test_sample);

test_case_result test_raise() {
    raise(SIGINT);
    // should return 11 from thread?
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


int main(void) {
    RUN_TESTS(
        test_sample_case,
        test_program_returns_0_case
        // test_raise_case
    )

    return 0;
}
