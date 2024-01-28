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

#include "signals/manager.h"
#include "signals/details/utils.h"

namespace wstux {
namespace signals {

manager::impl manager::m_impl;

void manager::clear()
{
    for (const handlers_map_t::value_type& handler : m_impl.handlers) {
        details::unregister_signal_handler(handler.first);
        details::unblock_signal(handler.first);
    }

    m_impl.handlers.clear();

    while (! m_impl.sig_queue.empty()) {
        m_impl.sig_queue.pop();
    }
}

void manager::erase(sig_num_t sig)
{
    handlers_map_t::iterator it = m_impl.handlers.find(sig);
    if (it == m_impl.handlers.cend()) {
        return;
    }

    m_impl.handlers.erase(it);
    details::unregister_signal_handler(sig);
    details::unblock_signal(sig);
}

void manager::process_signals()
{
    std::lock_guard<std::mutex> lock(m_impl.handlers_mutex);

    details::sig_set_t set;
    ::sigemptyset(&set);

    for (const handlers_map_t::value_type& handler : m_impl.handlers) {
        if (::sigaddset(&set, handler.first) != 0) {
            return;
        }
    }

    m_impl.is_stop = false;
    while (! m_impl.is_stop) {
        details::unblock_sigset(set);
        details::wait_signal(set);
        details::block_sigset(set);

        sig_info_t info = {};
        while (pop_signal(info)) {
            const handlers_map_t::const_iterator it = m_impl.handlers.find(info.si_signo);
            if (it != m_impl.handlers.cend()) {
                it->second(info.si_signo, info);
            }
        }
    }
}

void manager::process_signals(const std::chrono::milliseconds& msec, bool exit_after_timeout)
{
    std::lock_guard<std::mutex> lock(m_impl.handlers_mutex);

    details::sig_set_t set;
    ::sigemptyset(&set);

    for (const handlers_map_t::value_type& handler : m_impl.handlers) {
        if (::sigaddset(&set, handler.first) != 0) {
            return;
        }
    }

    m_impl.is_stop = false;
    while (! m_impl.is_stop) {
        details::unblock_sigset(set);
        details::wait_signal(set, msec);
        details::block_sigset(set);

        sig_info_t info = {};
        while (pop_signal(info)) {
            const handlers_map_t::const_iterator it = m_impl.handlers.find(info.si_signo);
            if (it != m_impl.handlers.cend()) {
                it->second(info.si_signo, info);
            }
        }

        if (exit_after_timeout) {
            break;
        }
    }
}

void manager::remove_handler(sig_num_t sig)
{
    std::unique_lock<std::mutex> lock(m_impl.handlers_mutex, std::defer_lock);
    if (! lock.try_lock()) {
        return;
    }
    erase(sig);
}

bool manager::reset_handler(sig_num_t sig, std::function<void()> func)
{
    return reset_handler(sig, [func](sig_num_t, const sig_info_t&) -> void { func(); });
}

bool manager::reset_handler(sig_num_t sig, sig_handler_fn_t func)
{
    std::unique_lock<std::mutex> lock(m_impl.handlers_mutex, std::defer_lock);
    if (! lock.try_lock()) {
        return false;
    }

    std::pair<handlers_map_t::iterator, bool> rc = m_impl.handlers.emplace(sig, func);
    if (! rc.second) {
        rc.first->second = func;
        return true;
    }
    if (! details::block_signal(sig)) {
        erase(sig);
        return false;
    }
    if (! details::register_signal_handler(sig, &on_signal_fn)) {
        erase(sig);
        return false;
    }
    return true;
}

bool manager::set_handler(sig_num_t sig, std::function<void()> func)
{
    return set_handler(sig, [func](sig_num_t, const sig_info_t&) -> void { func(); });
}

bool manager::set_handler(sig_num_t sig, sig_handler_fn_t func)
{
    std::unique_lock<std::mutex> lock(m_impl.handlers_mutex, std::defer_lock);
    if (! lock.try_lock()) {
        return false;
    }

    std::pair<handlers_map_t::iterator, bool> rc = m_impl.handlers.emplace(sig, func);
    if (! rc.second) {
        return false;
    }
    if (! details::block_signal(sig)) {
        erase(sig);
        return false;
    }
    if (! details::register_signal_handler(sig, &on_signal_fn)) {
        erase(sig);
        return false;
    }
    return true;
}

} // namespace signals
} // namespace wstux

