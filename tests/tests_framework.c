#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

#include "tests_framework.h"

#include <math.h>
#include <string.h>

// ---------------------------------- STRING HELPERS ----------------------------------
static string padding = "....................................................................................................";
static int line_len = 100;
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_BOLD          "\x1b[1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

string tcs_name(test_case_status status) {
    switch (status) {
        case SUCCESS: return "SUCCESS";
        case FAILURE: return "FAILURE";
        case ERROR: return "ERROR";
    }
    return "UNKNOWN_STATUS";
};

int ilen(int i) {
    if (i >= -1 || i <= 1) return 1;
    return ((int)log10l(abs(i))) + (i < 0 ? 1 : 0);
}

double elapsed(struct timespec start, struct timespec end) {
    return (end.tv_nsec - start.tv_nsec) * 1.0 / 1'000'000'000 + (end.tv_sec - start.tv_sec);
}
// ------------------------------------------------------------------------------------


// ---------------------------------- THREAD HELPERS ----------------------------------
struct proxy {
    test_case * tcase;
    test_case_result * result;
    struct timespec * clock_start;
    struct timespec * clock_end;
};
typedef struct proxy proxy_t;

void * proxy_case(void * args) {
    const auto proxy = (proxy_t *) args;
    clock_gettime(CLOCK_MONOTONIC_RAW, proxy->clock_start);
    const auto result = proxy->tcase->fn();
    clock_gettime(CLOCK_MONOTONIC_RAW, proxy->clock_end);
    memcpy(proxy->result, &result, sizeof(test_case_result));
    return nullptr;
}
// ------------------------------------------------------------------------------------


// ---------------------------------- ERRORMSG STACK ----------------------------------
static string * errstack;
static size_t errstack_i = 0;
static size_t errstack_size;
void errstack_init(size_t size) {
    errstack_size = size;
    errstack = malloc(size * sizeof(string));
}
void errstack_cleanup() {
    for (int i = 0; i < errstack_size; ++i) {
        free(errstack[errstack_i]);
    }
    free(errstack);
}
string errstr(size_t len) {
    errstack[errstack_i] = malloc(len * sizeof(char));
    return errstack[errstack_i++];
}
string errstack_pop() {
    return errstack[errstack_i--];
}
// ------------------------------------------------------------------------------------


int run_tests(test_case * cases, int num_cases) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    errstack_init(num_cases);

    int num_failures = 0;
    int num_errors = 0;
    test_case_result results[num_cases];
    pthread_t threads[num_cases];
    struct timespec clocks[num_cases * 2];
    proxy_t thread_proxies[num_cases];

    for (int i = 0; i < num_cases; ++i) {
        results[i].message = "";
    }

    printf("\n");
    printf("Running %i tests...\n\n", num_cases);

    for (int i = 0; i < num_cases; ++i) {
        thread_proxies[i] = (proxy_t){
            .tcase = &cases[i],
            .result = &results[i],
            .clock_start = &clocks[2 * i],
            .clock_end = &clocks[2 * i + 1],
        };
        const auto code = pthread_create(&threads[i], nullptr, proxy_case, &thread_proxies[i]);
        printf("[%i] (%s) Starting in thread [%u]!\n", i, cases[i].name, threads[i]);
        if (code != 0) {
            results[i].status = ERROR;

            const size_t len = strlen("[] Pthread error !\n") + ilen(i) + ilen(code) + 1;
            results[i].message = errstr(len);
            sprintf(results[i].message, "[%i] Pthread error %i!\n", i, code);
        }
    }

    for (int i = 0; i < num_cases; ++i) {
        pthread_join(threads[i], nullptr);
    }

    printf("\n");
    for (int i = 0; i < num_cases; ++i) {
        const auto result = &results[i];
        if (result->status == FAILURE)
            num_failures++;
        if (result->status == ERROR)
            num_errors++;


        auto clr = result->status != SUCCESS ? ANSI_COLOR_RED : ANSI_COLOR_GREEN;
        auto status = tcs_name(result->status);
        auto slen = strlen(status) + strlen(cases[i].name) + ilen(i) + 14;

        printf("[%i] (%s) %*.*s %.3fs %s%s%s\n", i, cases[i].name, 3, line_len - slen, padding, elapsed(clocks[2 * i], clocks[2 * i + 1]), clr, status, ANSI_COLOR_RESET);
        if (result->status != SUCCESS)
            printf("%s[%i] \t%s\n%s", ANSI_COLOR_RED, i, result->message, ANSI_COLOR_RESET);
        printf("\n");
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    auto rede = num_errors > 0 ? ANSI_COLOR_RED ANSI_BOLD : "";
    auto resete = num_errors > 0 ? ANSI_COLOR_RESET : "";
    auto redf = num_failures > 0 ? ANSI_COLOR_RED : "";
    auto resetf = num_failures > 0 ? ANSI_COLOR_RESET : "";
    printf(
        "Ran %i test cases. %sErrors: %i%s. %sFailures: %i%s. Successes: %i. \n"
        "Total procedure time: %.3fs\n",
        num_cases,
        rede, num_errors, resete,
        redf, num_failures, resetf,
        num_cases - num_errors - num_failures,
        elapsed(start, end)
    );

    errstack_cleanup();
    return num_errors + num_failures;
}


