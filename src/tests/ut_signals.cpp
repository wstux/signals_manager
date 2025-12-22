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

#include <csignal>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

#include <testing/testdefs.h>

#include "signals/manager.h"

TEST(signals, basic)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); });
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.is_stopped());

    sm.start();
    EXPECT_FALSE(sm.is_stopped());

    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, call_chain)
{
    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, []() -> void { ::kill(::getpid(), SIGUSR2); }));
    EXPECT_TRUE(sm.set_handler(SIGUSR2, []() -> void { ::kill(::getpid(), SIGTERM); }));
    EXPECT_TRUE(sm.set_handler(SIGTERM, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); });
    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, rt_signal)
{
    const int kSigRT = SIGRTMIN + 13;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(kSigRT, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); } );
    ::kill(::getpid(), kSigRT);
    tr.join();
}

TEST(signals, threaded_signals_processing)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    std::atomic_bool has_signal = {false};
    std::mutex m;
    m.lock();
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&has_signal, &m]() -> void {
        has_signal = true;
        m.unlock();
    }));

    sm.threaded_start();
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.is_stopped());

    sm.threaded_start();
    EXPECT_FALSE(sm.is_stopped());

    ::kill(::getpid(), SIGUSR1);

    m.lock();
    EXPECT_TRUE(has_signal);
    m.unlock();

    sm.stop();
    EXPECT_TRUE(sm.is_stopped());
}

TEST(signals, timeout)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(200ms); });
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.is_stopped());

    sm.start(200ms);
    EXPECT_FALSE(sm.is_stopped());

    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, exit_after_timeout)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    sm.start(20ms, true);
    EXPECT_FALSE(sm.is_stopped());

    ::kill(::getpid(), SIGUSR1);
    sm.process_signals(20ms);
    EXPECT_TRUE(sm.is_stopped());
}

TEST(signals, change_handler_while_processing)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); } );
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.set_handler(SIGUSR2, [&sm]() -> void { sm.stop(); }));

    EXPECT_FALSE(sm.is_stopped());
    sm.stop();
    tr.join();
}

TEST(signals, reset_handler)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));
    EXPECT_TRUE(sm.reset_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); } );
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.reset_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, reset_new_handler)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.reset_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    std::thread tr([&sm] { sm.start(); } );
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(sm.reset_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));

    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, remove_handler)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGCHLD, [&sm]() -> void { sm.stop(); }));
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop(); }));
    sm.remove_handler(SIGCHLD);
    sm.remove_handler(SIGUSR2);

    std::thread tr([&sm] { sm.start(); } );
    std::this_thread::sleep_for(200ms);
    ::kill(::getpid(), SIGCHLD);
    EXPECT_FALSE(sm.is_stopped());

    sm.remove_handler(SIGUSR1);
    ::kill(::getpid(), SIGUSR1);
    EXPECT_FALSE(sm.is_stopped());
    tr.join();
}

TEST(signals, unsafe_signal)
{
    wstux::signals::manager sm;
    EXPECT_FALSE(sm.set_handler(SIGSEGV, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.set_handler(SIGKILL, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.set_handler(SIGSTOP, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.set_handler(SIGCONT, [&sm]() -> void { sm.stop(); }));

    EXPECT_FALSE(sm.reset_handler(SIGSEGV, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.reset_handler(SIGKILL, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.reset_handler(SIGSTOP, [&sm]() -> void { sm.stop(); }));
    EXPECT_FALSE(sm.reset_handler(SIGCONT, [&sm]() -> void { sm.stop(); }));
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_TESTS();
}
