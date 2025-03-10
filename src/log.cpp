#include "log.hpp"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>

std::mutex debug::log_mutex;

#ifndef __DEBUG__
// The following code overrides the default signal handler for SIGSEGV.
// Debug has sanitizer support for SIGSEGV instead

class signal_capture {
public:
    signal_capture() {
        std::signal(SIGSEGV, [](int) {
            const char * msg = "Segmentation violation detected!\n";
            write(STDERR_FILENO, msg, strlen(msg));
            exit(EXIT_FAILURE);
        });
    }
} signal_capture_instance;

#endif // __DEBUG__
