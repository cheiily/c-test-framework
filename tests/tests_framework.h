//
// Created by cheily on 7.08.2026.
//

#ifndef PLAYGROUND_TESTS_FRAMEWORK_H
#define PLAYGROUND_TESTS_FRAMEWORK_H


typedef const char * string;

enum test_case_status {
    SUCCESS,
    FAILURE,
    ERROR
};
typedef enum test_case_status test_case_status;

struct test_case_result {
    test_case_status status;
    string message;
};
typedef struct test_case_result test_case_result;

typedef test_case_result(*test_case_fn)(void);

struct test_case {
    string name;
    test_case_fn fn;
};
typedef struct test_case test_case;

#define TEST_CASE(_name, _fn) \
    static test_case _fn##_case = { \
        .name = _name, \
        .fn = _fn \
    }

int run_tests(test_case * cases, int num_cases);
#define RUN_TESTS(...) \
    test_case __test_cases[] = {__VA_ARGS__}; \
    int __num_cases = sizeof(__test_cases) / sizeof(__test_cases[0]); \
    int test_exit_code = run_tests(__test_cases, __num_cases);


#endif //PLAYGROUND_TESTS_FRAMEWORK_H
