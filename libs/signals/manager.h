/*
 * The MIT License
 *
 * Copyright 2024 Chistyakov Alexander.
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

#ifndef _SIGNALS_MANAGER_SIGNALS_MANAGER_H
#define _SIGNALS_MANAGER_SIGNALS_MANAGER_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "signals/types.h"

namespace wstux {
namespace signals {

class manager final
{
public:
    ~manager() { clear(); }

    void clear();

    void process_signals();

    void process_signals(const std::chrono::milliseconds& msec, bool exit_after_timeout);

    void remove_handler(sig_num_t sig);

    bool reset_handler(sig_num_t sig, std::function<void()> func);

    bool reset_handler(sig_num_t sig, sig_handler_fn_t func);

    bool set_handler(sig_num_t sig, std::function<void()> func);

    bool set_handler(sig_num_t sig, sig_handler_fn_t func);

    void stop_process_signals() { m_impl.is_stop = true; }

private:
    using handlers_map_t = std::unordered_map<sig_num_t, sig_handler_fn_t>;

private:
    void erase(sig_num_t sig);

    static void on_signal_fn(sig_num_t /*sig_num*/, sig_info_t* sig_info, void*)
    {
        std::unique_lock<std::mutex> lock(m_impl.queue_mutex);
        m_impl.sig_queue.push(*sig_info);
    }

    static bool pop_signal(sig_info_t& sig_info)
    {
        std::unique_lock<std::mutex> lock(m_impl.queue_mutex);
        if (m_impl.sig_queue.empty()) {
            return false;
        }
        sig_info = m_impl.sig_queue.front();
        m_impl.sig_queue.pop();
        return true;
    }

private:
    struct impl
    {
        std::atomic_bool is_stop = {false};
    
        std::mutex handlers_mutex;
        handlers_map_t handlers;

        std::mutex queue_mutex;
        std::queue<sig_info_t> sig_queue;
    };

private:
    static impl m_impl;
};

} // namespace signals
} // namespace wstux

#endif /* _SIGNALS_MANAGER_SIGNALS_MANAGER_H */

