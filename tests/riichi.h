#ifndef RIICHI_H
#define RIICHI_H


typedef const char * riichi_string;

enum riichi_test_case_status {
    SUCCESS,
    FAILURE,
    ERROR
};
typedef enum riichi_test_case_status riichi_test_case_status;

struct riichi_test_case_result {
    riichi_test_case_status status;
    riichi_string message;
};
typedef struct riichi_test_case_result riichi_test_case_result;

typedef riichi_test_case_result(*riichi_test_fn)(void);

struct riichi_test_case {
    riichi_string name;
    riichi_test_fn fn;
};
typedef struct riichi_test_case riichi_test_case;

#define RIICHI_TEST_CASE(_name, _fn)        \
    static riichi_test_case _fn##_case = {  \
        .name = _name,                      \
        .fn = _fn                           \
    }

int riichi_run_tests(riichi_test_case * cases, int num_cases);
#define RIICHI_RUN_TESTS(...)                                                           \
    riichi_test_case riichi_test_cases[] = {__VA_ARGS__};                               \
    int riichi_num_cases = sizeof(riichi_test_cases) / sizeof(riichi_test_cases[0]);    \
    int riichi_test_exit_code = riichi_run_tests(riichi_test_cases, riichi_num_cases);

void riichi_signal_handler(int signal, siginfo_t * info, void *ucontext);
void riichi_install_handler(int signal, const struct sigaction * handler);
#define RIICHI_INSTALL_HANDLERS(...)                                                    \
    int riichi_signals[] = {__VA_ARGS__};                                               \
    int riichi_num_signals = sizeof(riichi_signals) / sizeof(riichi_signals[0]);        \
    struct sigaction riichi_handler;                                                    \
    riichi_handler.sa_sigaction = riichi_signal_handler;                                \
    sigemptyset(&riichi_handler.sa_mask);                                               \
    sigaddset(&riichi_handler.sa_mask, SA_SIGINFO);                                     \
    riichi_handler.sa_flags = 0;                                                        \
    for (int i = 0; i < riichi_num_signals; ++i) {                                      \
        riichi_install_handler(riichi_signals[i], &riichi_handler);                     \
    }

#endif //RIICHI_H
