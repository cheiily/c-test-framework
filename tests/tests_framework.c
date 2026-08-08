#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "tests_framework.h"

#include <math.h>
#include <string.h>

static string padding = "....................................................................................................";
static int line_len = 60;


string tcs_name(test_case_status status) {
    switch (status) {
        case SUCCESS: return "SUCCESS";
        case FAILURE: return "FAILURE";
        case ERROR: return "ERROR";
    }
    return "UNKNOWN_STATUS";
};


int run_tests(test_case * cases, int num_cases) {
    int num_failures = 0;
    int num_errors = 0;

    printf("\n");
    printf("Running %i tests...\n\n", num_cases);

    for (int i = 0; i < num_cases; ++i) {
        test_case_result result = cases[i].fn();
        if (result.status == FAILURE)
            num_failures++;
        if (result.status == ERROR)
            num_errors++;

        auto status = tcs_name(result.status);
        auto slen = strlen(status) + strlen(cases[i].name) + (int)log10l(i + 1);
        printf("[%i] %s %*.*s %s\n", i, cases[i].name, 3, line_len - slen, padding, status);
        if (result.message)
            printf("[%i] \t%s\n", i, result.message);
        printf("\n");
    }

    printf("Ran %i test cases. Errors: %i. Failures: %i. Successes: %i.\n", num_cases, num_errors, num_failures, num_cases - num_errors - num_failures);
    return num_errors + num_failures;
}


