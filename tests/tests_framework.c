#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "tests_framework.h"

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

        printf("[%i] %s - %s\n", i, cases[i].name, tcs_name(result.status));
        if (result.message)
            printf("[%i] \t%s\n", i, result.message);
        printf("\n");
    }

    printf("Ran %i test cases. Errors: %i. Failures: %i. Successes: %i.\n", num_cases, num_errors, num_failures, num_cases - num_errors - num_failures);
    return num_errors + num_failures;
}


