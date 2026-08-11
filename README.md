# RIICHI testing framework

This is a _rudimentary_ framework created as an exercise in the C programming language. 
The repostory may also serve as a project template for convenience in setting up the framework.

To build either use your IDE, execute `build.sh` or simply `cmake -B build && cd build && make`.
That should create your main program executable and a `_tests` executable alongside it. 
If unchanged, the project name will remain `riichi`.

## Code structure

The entirety of the framework is contained within `tests/riichi.h` and `tests/riichi.c`.
Any publicly exposed API is prefixed with the `riichi_`/`RIICHI_` "namespace" to avoid identifier collisions.

A sample test file is provided as usage example in the form of `tests/tests.c`. 
Anything that file contains is what should be written by the end user. 


### A brief description of the API is provided below:

### `riichi_test_fn`
This is the function-signature type that any test case should follow. 
The function takes no arguments and should return a locally-constructed`riichi_test_case_result` by value.

### `riichi_test_case_result`
The case result to be returned by any test case. 
Defined as:
```
struct riichi_test_case_result {
    riichi_test_case_status status;
    riichi_string message;
};
```
The case should always set a value to `status`. \
The message (`riichi_string` is a convenience typedef to `const char *`) in only expected in case the status is not a `SUCCESS`.
The message should be a string literal. Optionally the user may allocate a string on the heap, store a pointer and free it manually, after the tests have concluded.
There is currently no support for dynamically allocated messages out-of-the-box.

### `riichi_test_case_status` (enum)
One of`SUCCESS`,`FAILURE`,`ERROR`.
Self-descriptive. Use `FAILURE` for expected test-case failures. 
Use `ERROR`, if possible, for unexpected errors such as I/O, third-party failures.

### `riichi_test_case`
- Should be created via the `RIICHI_TEST_CASE` macro.

Structure containing data about the test case.
Defined as:
```c
struct riichi_test_case {
    riichi_string name;
    riichi_test_fn fn;
};
```
Currently, there is no support for sub-cases out-of-the-box. \
The structure may be declared manually if the user so prefers, but should usually be handled by the `RIICHI_TEST_CASE` macro.


### `RIICHI_TEST_CASE` (macro)
Macro creating a test case structure, declared for convenient usage right below the actual test function.
Takes the same arguments as the structure. Name should be enclosed in quotes. \
The resulting test case will have the name of the passed function with a `_case` postfix. 
This handle-name will then be used to register the case with `RIICHI_RUN_TESTS`.
This handle-name is only used in-code and the actual name is used for logging purposes.

### `riichi_run_tests`
- Should be called via the `RIICHI_RUN_TESTS` macro.
- `RIICHI_INSTALL_HANDLERS` should be called before this function.

Main entrypoint to the testing flow. Takes an array of `riichi_test_case` and its size.

Every test case is run in a parallel thread. Be wary of the resources you're using. \
For each test case, execution time is measured and reported at the end of the flow.
Total execution time is also measured and result status counts are noted and reported as well. \
Currently, there is a rudimentary safety measure in place in the form of signal handling. 
See `RIICHI_INSTALL_HANDLERS`. However, due to the nature of multithreading, that solution is intrinsically unsafe as that would require full process separation.
Multi-process and single-thread execution are currently not supported out-of-the-box.

### `RIICHI_RUN_TESTS` (macro)
Var-arg macro handling and passing the test cases to the testing flow. 
Requires that the cases be declared as instances of `riichi_test_case`, either via `RIICHI_TEST_CASE` or manually.

### `riichi_install_handler`
- Should be called via the `RIICHI_INSTALL_HANDLERS` macro.

Installs a `riichi_signal_handler` via `sigaction` for the given signal. 

### `RIICHI_INSTALL_HANDLERS` (macro)
Var-arg macro to install a signal handler for any of the signals below. \
The macro packs the arguments and creates a common handler for all the declared signals.

The allowed signals are: \
`SIGABRT`,
`SIGALRM`,
`SIGBUS`,
`SIGCHLD`,
`SIGCONT`,
`SIGFPE`,
`SIGHUP`,
`SIGILL`,
`SIGINT`,
`SIGKILL`,
`SIGPIPE`,
`SIGQUIT`,
`SIGSEGV`,
`SIGSTOP`,
`SIGTERM`,
`SIGTSTP`,
`SIGTTIN`,
`SIGTTOU`,
`SIGUSR1`,
`SIGUSR2`,
`SIGPOLL`,
`SIGPROF`,
`SIGSYS`,
`SIGTRAP`,
`SIGURG`,
`SIGVTALRM`,
`SIGXCPU`,
`SIGXFSZ`


### `riichi_signal_handler`
- Should not be called manually.

A simple handler, whose main role is displaying an appropriate diagnostic message. This attempt is separated into two parts:
- The async-signal-safe part which uses `write` and `strlen` to print diagnostics. By default, the program will exit here.
- The optional unsafe part which uses `printf` to display just a little more potentially-useful information and then attempts to resume the program.
    This extra info includes any data received within `siginfo_t` and the current thread id as obtained by `pthread_self`.
    The thread id may be useful in conjunction with the respective test-launch log line to corellate the appropriate failing test number. 
    Note however, that this will usually not be the case, as such behavior may be expected only if the test itself fired `raise`. \
    This behavior is configured by defining `RIICHI_UNSAFE_DIAG` and `RIICHI_RESUME_ON_SIGNAL` respectively before the inclusion of `riichi.h`. \
    By default, both macros are left undefined, commented-out in the template `tests.c` file.

## References that helped me implement this
- https://en.cppreference.com/c/23
- clock_gettime(3)
- pthreads(7)
- pthread_create(3)
- https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/
- signal-safety(7)
  - https://en.wikipedia.org/wiki/Reentrancy_(computing)
- https://stackoverflow.com/questions/5282099/signal-handling-in-pthreads
- https://codereview.stackexchange.com/questions/284179/proper-implementation-of-signal-handler-and-multithreading-pthread
- https://stackoverflow.com/questions/16891019/how-to-avoid-using-printf-in-a-signal-handler
- https://sourceware.org/glibc/manual/latest/html_node/Sigaction-Function-Example.html
- and many others :)


