/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

extern "C" {
    #include <pthread.h>
}

#include <cstring>

#include "signals/details/utils.h"

namespace wstux {
namespace signals {
namespace details {

bool block_signal(sig_num_t sig)
{
    if (! is_safe_signal(sig)) {
        return false;
    }

    sig_set_t set;
    ::sigemptyset(&set);
    ::sigaddset(&set, sig);
    return (::pthread_sigmask(SIG_BLOCK, &set, nullptr) == 0);
}

bool block_sigset(const sig_set_t& set)
{
    return (::pthread_sigmask(SIG_BLOCK, &set, nullptr) == 0);
}

bool is_safe_signal(sig_num_t sig)
{
    return (sig != SIGSEGV) && (sig != SIGKILL) && (sig != SIGSTOP) && (sig != SIGCONT);
}

bool register_signal_handler(sig_num_t sig, sig_action_fn_t on_signal_fn)
{
    if (! is_safe_signal(sig)) {
        return false;
    }

    struct ::sigaction sa;
    ::memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_RESTART | SA_SIGINFO ;
    sa.sa_sigaction = on_signal_fn;
    return (::sigaction(sig, &sa, 0) == 0);
}

bool unblock_signal(sig_num_t sig)
{
    if (! is_safe_signal(sig)) {
        return false;
    }

    sig_set_t set;
    ::sigemptyset(&set);
    ::sigaddset(&set, sig);
    return (::pthread_sigmask(SIG_UNBLOCK, &set, nullptr) == 0);
}

bool unblock_sigset(const sig_set_t& set)
{
    return (::pthread_sigmask(SIG_UNBLOCK, &set, nullptr) == 0);
}

bool unregister_signal_handler(sig_num_t sig)
{
    if (! is_safe_signal(sig)) {
        return false;
    }

    struct ::sigaction sa;
    ::memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_DFL;
    return (::sigaction(sig, &sa, 0) == 0);
}

bool wait_signal(const sig_set_t& set)
{
    //sig_set_t set;
    //::sigemptyset(&set);
    sig_num_t sig;
    int rc = sigwait(&set, &sig);
    return (rc == 0);
}

bool wait_signal(const sig_set_t& set, const std::chrono::milliseconds& msec)
{
    //sig_set_t set;
    //::sigemptyset(&set);
    const long ms = msec.count();
    const ::timespec ts{ms / 1000, (ms % 1000) * 1000000};

    int rc = ::sigtimedwait(&set, NULL, &ts);
    return (rc == 0);
}

} // namespace details
} // namespace signals
} // namespace wstux

