#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "riichi.h"

#include <errno.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

// ---------------------------------- STRING HELPERS ----------------------------------
#define ECHO(A) #A

// reentrant
riichi_string signame(const int signal) {
    switch (signal) {
        case SIGABRT: return ECHO(SIGABRT);
        case SIGALRM: return ECHO(SIGALRM);
        case SIGBUS: return ECHO(SIGBUS);
        case SIGCHLD: return ECHO(SIGCHLD);
        case SIGCONT: return ECHO(SIGCONT);
        case SIGFPE: return ECHO(SIGFPE);
        case SIGHUP: return ECHO(SIGHUP);
        case SIGILL: return ECHO(SIGILL);
        case SIGINT: return ECHO(SIGINT);
        case SIGKILL: return ECHO(SIGKILL);
        case SIGPIPE: return ECHO(SIGPIPE);
        case SIGQUIT: return ECHO(SIGQUIT);
        case SIGSEGV: return ECHO(SIGSEGV);
        case SIGSTOP: return ECHO(SIGSTOP);
        case SIGTERM: return ECHO(SIGTERM);
        case SIGTSTP: return ECHO(SIGTSTP);
        case SIGTTIN: return ECHO(SIGTTIN);
        case SIGTTOU: return ECHO(SIGTTOU);
        case SIGUSR1: return ECHO(SIGUSR1);
        case SIGUSR2: return ECHO(SIGUSR2);
        case SIGPOLL: return ECHO(SIGPOLL);
        case SIGPROF: return ECHO(SIGPROF);
        case SIGSYS: return ECHO(SIGSYS);
        case SIGTRAP: return ECHO(SIGTRAP);
        case SIGURG: return ECHO(SIGURG);
        case SIGVTALRM: return ECHO(SIGVTALRM);
        case SIGXCPU: return ECHO(SIGXCPU);
        case SIGXFSZ: return ECHO(SIGXFSZ);
        default: return "UNKNOWN SIGNAL";
    };
}

static riichi_string padding = "....................................................................................................";
static int line_len = 100;
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_BOLD          "\x1b[1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

riichi_string tcs_name(const riichi_test_case_status status) {
    switch (status) {
        case SUCCESS: return "SUCCESS";
        case FAILURE: return "FAILURE";
        case ERROR: return "ERROR";
    }
    return "UNKNOWN_STATUS";
};

bool is_valid_tcs_status(riichi_test_case_status status) {
    return (status == SUCCESS || status == FAILURE || status == ERROR);
}

int ilen(const int i) {
    if (i >= -1 || i <= 1) return 1;
    return ((int)log10l(abs(i))) + (i < 0 ? 1 : 0);
}

double elapsed(const struct timespec start, const struct timespec end) {
    return (end.tv_nsec - start.tv_nsec) * 1.0 / 1'000'000'000 + (end.tv_sec - start.tv_sec);
}
// ------------------------------------------------------------------------------------


// ---------------------------------- ERRORMSG STACK ----------------------------------
static riichi_string * errstack;
static size_t errstack_i = 0;
static size_t errstack_size;
void errstack_init(const size_t size) {
    errstack_size = size;
    errstack = malloc(size * sizeof(riichi_string));
}
void errstack_cleanup() {
    for (int i = 0; i < errstack_size; ++i) {
        free(errstack[errstack_i]);
    }
    free(errstack);
}
riichi_string errstr(const size_t len) {
    errstack[errstack_i] = malloc(len * sizeof(char));
    return errstack[errstack_i++];
}
riichi_string errstack_pop() {
    return errstack[errstack_i--];
}
// ------------------------------------------------------------------------------------


// ---------------------------------- THREAD HELPERS ----------------------------------
struct proxy {
    riichi_test_case * tcase;
    riichi_test_case_result * result;
    struct timespec * clock_start;
    struct timespec * clock_end;
};
typedef struct proxy proxy_t;

void * proxy_case(void * args) {
    const auto proxy = (proxy_t *) args;
    clock_gettime(CLOCK_MONOTONIC_RAW, proxy->clock_start);
    const auto result = proxy->tcase->fn();
    clock_gettime(CLOCK_MONOTONIC_RAW, proxy->clock_end);
    memcpy(proxy->result, &result, sizeof(riichi_test_case_result));
    return nullptr;
}


enum RIICHI_PTHREAD_ACTION {
    CREATION,
    JOIN
};
riichi_string riichi_pthread_action_name(enum RIICHI_PTHREAD_ACTION action) {
    switch (action) {
        case CREATION: return ECHO(CREATION);
        case JOIN: return ECHO(JOIN);
    }
    return "UNKNOWN";
}

void save_pthread_error(riichi_test_case_result * result, const int code, const enum RIICHI_PTHREAD_ACTION action) {
    if (code != 0) {
        result->status = ERROR;

        // TODO message may be saved to upon joining failure, zero and compare to nullptr, strcat
        const riichi_string mode = riichi_pthread_action_name(action);
        const size_t len = strlen("Pthread  error !\n") + strlen(mode) + ilen(code) + 1;
        result->message = errstr(len);
        sprintf(result->message, "Pthread %s error %i!\n", mode, code);
    }
}
// ------------------------------------------------------------------------------------


// ---------------------------------- SIGNAL HANDLER ----------------------------------
void riichi_signal_handler(const int signal, siginfo_t * info, void *ucontext) {
    const riichi_string sname = signame(signal);

    write(STDOUT_FILENO, ANSI_COLOR_RED, strlen(ANSI_COLOR_RED));
    write(STDOUT_FILENO, "Caught signal ", strlen("Caught signal "));
    write(STDOUT_FILENO, sname, strlen(sname));
    write(STDOUT_FILENO, "!\n", strlen("!\n"));

#ifndef RIICHI_RESUME_ON_SIGNAL
    write(STDOUT_FILENO, "EXITING...\n", strlen("EXITING...\n"));
    write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
    exit(signal);
#endif

    const riichi_string msg = "RIICHI_EXIT_ON_SIGNAL not defined. Attempting to resume program execution. Anything from now on is undefined behaviour!.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));

#ifdef RIICHI_UNSAFE_DIAG
    // printf is not async-signal-safe, illegal here
    // no color to not change it forever in case of crash
    printf("Signal code: %i\n", signal);
    printf("[siginfo_t] Signal number: %u. Signal errno: %u. Signal code: %u. Sender PID: %i.\n", info->si_signo, info->si_errno, info->si_code, info->si_pid);
    printf("Current thread id: %u\n\n", pthread_self());
#endif
}

void riichi_install_handler(int signal, const struct sigaction * handler) {
    struct sigaction old_action;
    sigaction(signal, nullptr, &old_action);
    if (old_action.sa_handler != SIG_IGN && sigaction(signal, handler, nullptr) != 0) {
        printf("Couldn't install signal handler for signal %i. Errno %i.", signal, errno);
    }
}
// ------------------------------------------------------------------------------------



int riichi_run_tests(riichi_test_case * cases, int num_cases) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    errstack_init(num_cases);

    int num_failures = 0;
    int num_errors = 0;
    volatile riichi_test_case_result results[num_cases];
    pthread_t threads[num_cases];
    volatile struct timespec clocks[num_cases * 2];
    volatile proxy_t thread_proxies[num_cases];

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
        if (code != 0) save_pthread_error(&results[i], code, CREATION);
    }

    for (int i = 0; i < num_cases; ++i) {
        const auto code = pthread_join(threads[i], nullptr);
        if (code != 0) save_pthread_error(&results[i], code, JOIN);
    }

    printf("\n");
    for (int i = 0; i < num_cases; ++i) {
        const auto result = &results[i];
        if (result->status == FAILURE)
            num_failures++;
        if (result->status == ERROR || !is_valid_tcs_status(result->status))
            num_errors++;


        const auto clr = result->status != SUCCESS ? ANSI_COLOR_RED : ANSI_COLOR_GREEN;
        const auto status = tcs_name(result->status);
        const auto slen = strlen(status) + strlen(cases[i].name) + ilen(i) + 14;

        printf("[%i] (%s) %*.*s %.3fs %s%s%s\n", i, cases[i].name, 3, line_len - slen, padding, elapsed(clocks[2 * i], clocks[2 * i + 1]), clr, status, ANSI_COLOR_RESET);
        if (result->status != SUCCESS)
            printf("%s[%i] \t%s\n%s", ANSI_COLOR_RED, i, result->message, ANSI_COLOR_RESET);
        printf("\n");
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    const auto rede = num_errors > 0 ? ANSI_COLOR_RED ANSI_BOLD : "";
    const auto resete = num_errors > 0 ? ANSI_COLOR_RESET : "";
    const auto redf = num_failures > 0 ? ANSI_COLOR_RED : "";
    const auto resetf = num_failures > 0 ? ANSI_COLOR_RESET : "";
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


