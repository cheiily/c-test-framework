
// Uncomment these to change signal handling behavior.
#define RIICHI_RESUME_ON_SIGNAL
// #define RIICHI_UNSAFE_DIAG

#include "riichi.c"
#include "../program/program.h"


static riichi_test_case_result test_sample() {
    return (riichi_test_case_result) {
        .status = SUCCESS,
    };
}
RIICHI_TEST_CASE("should pass", test_sample);

riichi_test_case_result test_raise() {
    raise(SIGSEGV);
};
RIICHI_TEST_CASE("should raise signal", test_raise);


static riichi_test_case_result test_program_returns_0() {
    const int ret = invoke(0, nullptr);
    return (riichi_test_case_result) {
        .status = ret == 0 ? SUCCESS : FAILURE,
        .message = ret == 0 ? nullptr : "Program did not return 0"
    };
}
RIICHI_TEST_CASE("program should return 0", test_program_returns_0);

static riichi_test_case_result test_sleep() {
    sleep(1);
    return (riichi_test_case_result) {
        .status = SUCCESS,
    };
}
RIICHI_TEST_CASE("should sleep for 1 second", test_sleep);

static riichi_test_case_result test_long_loop() {
    bool acc = false;
    for (int i = 0; i < 1'000'000'000; ++i) {
        acc = !acc;
    }
    printf("%i\n", acc);
    return (riichi_test_case_result) {
        .status = SUCCESS,
    };
}
RIICHI_TEST_CASE("should execute a long loop to delay the execution", test_long_loop);

static riichi_test_case_result test_writes_and_error() {
    return (riichi_test_case_result) {
        .status = ERROR,
        .message = "Third party error simulation"
    };
}
RIICHI_TEST_CASE("should return error with a message", test_writes_and_error);

static riichi_test_case_result test_writes_and_failure() {
    return (riichi_test_case_result) {
        .status = FAILURE,
        .message = "Test failure simulation"
    };
}
RIICHI_TEST_CASE("should return failure with a message", test_writes_and_failure);


int main(void) {
    RIICHI_INSTALL_HANDLERS(SIGSEGV, SIGABRT, SIGINT, SIGTERM, SIGHUP)
    RIICHI_RUN_TESTS(
        test_sample_case,
        test_program_returns_0_case,
        test_sleep_case,
        test_long_loop_case,
        test_raise_case,
        test_writes_and_error_case,
        test_writes_and_failure_case
    )

    return riichi_test_exit_code;
}
