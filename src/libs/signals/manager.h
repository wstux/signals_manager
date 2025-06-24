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

#ifndef _LIBS_SIGNALS_MANAGER_H_
#define _LIBS_SIGNALS_MANAGER_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "signals/types.h"
#include "signals/details/queue.h"
#include "signals/details/semaphore.h"

namespace wstux {
namespace signals {

/**
 *  \brief  Signal manager.
 *
 *  The signal manager allows you to install custom signal handlers. Signal
 *  processing is performed in a separate thread.
 *
 *  The manager accepts as input the numbers of signals that will be intercepted
 *  and processed, and their handlers. These handlers are registered inside the
 *  manager, and for signals, a blocking is performed in the current thread and
 *  a default handler is registered.
 *
 *  Signal processing occurs in a separate thread, which personally removes
 *  locks for registered signals. Handlers cannot be added or removed while the
 *  signal processing thread is running.
 *
 *  When the manager is destroyed, the installed signal blocks are unblocked,
 *  and all handlers are deregistered.
 *
 *  Work order:
 *  1. the main thread registers the necessary handlers and blocks the
 *     registered signals;
 *  2. all subsequent threads inherit the handler block mask;
 *  3. when the application is ready for work, the manager starts a separate
 *     thread to receive signals;
 *  4. the signal processing thread removes blocks from registered signals and
 *     waits for them to be triggered;
 *  5. when a signal is received, the signal is processed in the processing
 *     thread - information from the signal is placed in a queue and informs
 *     about the presence of a signal, after which signal processing is
 *     completed.
 *  6. control returns to the processing thread, which locks all registered
 *     signals, reads signals from the queue and calls real registered handlers
 *     for them;
 *  7. go to 4.
 *
 *  Signals can be triggered not only in the signal processing thread. If
 *  someone removes a block from their thread or starts a new thread before all
 *  handlers are registered, then signals will start to be triggered in that
 *  thread as well. To protect against such situations, a lock-free signal queue
 *  is used, and the signal processing thread will also see the presence of a
 *  new signal via a semaphore.
 */
class manager final
{
public:
    ~manager() { clear(); }

    void clear();

    bool is_stopped() const { return m_is_stop; }

    /// \brief  Remove the handler for the specified signal.
    /// \param  sig - signal number.
    void remove_handler(sig_num_t sig);

    /// \brief  Changing a signal handler.
    /// \param  sig - signal number.
    /// \param  func - new custom signal handler.
    /// \return True - signal handler has been installed successfully.
    ///     False - signal handling process has started.
    bool reset_handler(sig_num_t sig, std::function<void()> func);

    /// \brief  Changing a signal handler.
    /// \param  sig - signal number.
    /// \param  func - new custom signal handler.
    /// \return True - signal handler has been installed successfully.
    ///     False - signal handling process has started.
    bool reset_handler(sig_num_t sig, sig_handler_fn_t func);

    /// \brief  Setting a signal handler.
    /// \param  sig - signal number.
    /// \param  func - custom signal handler.
    /// \return True - signal handler has been installed successfully.
    ///     False - signal handler has already been installed or signal handling
    ///     process has started.
    bool set_handler(sig_num_t sig, std::function<void()> func);

    /// \brief  Setting a signal handler.
    /// \param  sig - signal number.
    /// \param  func - custom signal handler.
    /// \return True - signal handler has been installed successfully.
    ///     False - signal handler has already been installed or signal handling
    ///     process has started.
    bool set_handler(sig_num_t sig, sig_handler_fn_t func);

    void signals_processing();

    void signals_processing(const std::chrono::milliseconds& msec, bool exit_after_timeout = false);

    void stop_processing();

    void threaded_signals_processing(const std::chrono::milliseconds& msec = std::chrono::milliseconds(0));

private:
    using handlers_map_t = std::unordered_map<sig_num_t, sig_handler_fn_t>;
    using signals_queue_t = details::signals_queue_t<sig_info_t, 31>;

private:
    void erase(sig_num_t sig);

    static void on_signal_fn(sig_num_t /*sig_num*/, sig_info_t* sig_info, void*)
    {
        m_sig_queue.push(*sig_info);
        wake();
    }

    static bool pop_signal(sig_info_t& sig_info)
    {
        if (m_sig_queue.empty()) {
            return false;
        }
        m_sig_queue.pop(sig_info);
        return true;
    }

    static void processing();

    static void processing_to(const std::chrono::milliseconds& msec, bool exit_after_timeout);

    static void wait() { m_sem.wait(); }

    static void wait(const std::chrono::milliseconds& ms) { m_sem.timed_wait(ms); }

    static void wake() { m_sem.post(); }

private:
    static std::atomic_bool m_is_stop;
    static details::semaphore m_sem;
    static std::unique_ptr<std::thread> m_p_thread;

    static std::mutex m_handlers_mutex;
    static handlers_map_t m_handlers;

    static signals_queue_t m_sig_queue;
};

} // namespace signals
} // namespace wstux

#endif /* _LIBS_SIGNALS_MANAGER_H_ */

