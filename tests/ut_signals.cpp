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
    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(); });
    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, call_chain)
{
    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, []() -> void { ::kill(::getpid(), SIGUSR2); }));
    EXPECT_TRUE(sm.set_handler(SIGUSR2, []() -> void { ::kill(::getpid(), SIGTERM); }));
    EXPECT_TRUE(sm.set_handler(SIGTERM, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(); });
    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, rt_signal)
{
    const int kSigRT = SIGRTMIN + 13;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(kSigRT, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(); } );
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

    sm.threaded_signals_processing();

    std::this_thread::sleep_for(200ms);
    ::kill(::getpid(), SIGUSR1);

    m.lock();
    EXPECT_TRUE(has_signal);
    m.unlock();

    sm.stop_processing();
    EXPECT_TRUE(sm.is_stopped());
}

TEST(signals, timeout)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(200ms); });
    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

TEST(signals, change_handler_while_processing)
{
    using namespace std::chrono_literals;

    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(); } );
    EXPECT_FALSE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));

    std::this_thread::sleep_for(200ms);
    sm.stop_processing();
    tr.join();
}

TEST(signals, reset_handler)
{
    wstux::signals::manager sm;
    EXPECT_TRUE(sm.set_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));
    EXPECT_TRUE(sm.reset_handler(SIGUSR1, [&sm]() -> void { sm.stop_processing(); }));

    std::thread tr([&sm] { sm.signals_processing(); } );
    ::kill(::getpid(), SIGUSR1);
    tr.join();
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_TESTS();
}
