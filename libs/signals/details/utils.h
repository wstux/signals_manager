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

#ifndef _SIGNALS_MANAGER_SIGNALS_UTILS_H
#define _SIGNALS_MANAGER_SIGNALS_UTILS_H

#include <csignal>
#include <chrono>

#include "signals/types.h"

namespace wstux {
namespace signals {
namespace details {

using sig_action_fn_t = void (*)(sig_num_t, sig_info_t*, void*);
using sig_set_t = ::sigset_t;

bool block_signal(sig_num_t num);

bool block_sigset(const sig_set_t& set);

bool is_safe_signal(sig_num_t sig);

bool register_signal_handler(sig_num_t sig, sig_action_fn_t on_signal_fn);

bool unblock_signal(sig_num_t sig);

bool unblock_sigset(const sig_set_t& set);

bool unregister_signal_handler(sig_num_t sig);

bool wait_signal(const sig_set_t& set);

bool wait_signal(const sig_set_t& set, const std::chrono::milliseconds& msec);

} // namespace details
} // namespace signals
} // namespace wstux

#endif /* _SIGNALS_MANAGER_SIGNALS_UTILS_H */

