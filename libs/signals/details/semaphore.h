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

#ifndef _SIGNALS_MANAGER_SIGNALS_SEMAPHORE_H
#define _SIGNALS_MANAGER_SIGNALS_SEMAPHORE_H

#ifdef __linux__
    #include <semaphore.h>
#else
    #error "Unsupported platform for using semaphore"
#endif

#include <chrono>

namespace wstux {
namespace signals {
namespace details {

class semaphore final
{
public:
    /// \brief  Creates a semaphore with zero count.
    semaphore()
    {
        constexpr unsigned int count = 0;
#ifdef __linux__
        ::sem_init(&m_sem, 1, count);
#endif
    }

    /// \brief  Destroys the semaphore.
    ~semaphore() 
    {
#ifdef __linux__
        ::sem_destroy(&m_sem);
#endif
    }

    /// \brief  Increments the semaphore count. If there are processes/threads
    ///         blocked waiting for the semaphore, then one of these processes 
    ///         will return successfully from its wait function.
    inline void post()
    {
#ifdef __linux__
        ::sem_post(&m_sem);
#endif
    }

    /// \brief  Decrements the semaphore if the semaphore's value is greater than
    ///         zero and returns. Otherwise, waits for the semaphore to the posted
    ///         or the timeout expires.
    /// \param  msec - the timeout value.
    /// \return If the timeout expires or there is an error, the function returns
    ///         false. If the semaphore is posted the function returns true.
    inline bool timed_wait(const std::chrono::milliseconds& msec) 
    {
#ifdef __linux__
        const long ms = msec.count();
        const ::timespec ts{ms / 1000, (ms % 1000) * 1000000};
        const int rc = ::sem_timedwait(&m_sem, &ts);
        if (rc == 0) {
            return true;
        }
        if (rc > 0) {
            errno = rc; // Buggy glibc, copy the returned error code to errno.
        }
#endif
        return false;
    }

    /// \brief  Decrements the semaphore. If the semaphore value is not greater 
    ///         than zero, then the calling process/thread blocks until it can
    ///         decrement the counter.
    inline void wait()
    {
#ifdef __linux__
        ::sem_wait(&m_sem);
#endif
    }

private:
    semaphore(const semaphore&);
    semaphore& operator=(const semaphore&);

private:
#ifdef __linux__
        using semaphore_t = ::sem_t;
#endif

private:
    semaphore_t m_sem;
};

} // namespace details
} // namespace signals
} // namespace wstux

#endif /* _SIGNALS_MANAGER_SIGNALS_SEMAPHORE_H */

