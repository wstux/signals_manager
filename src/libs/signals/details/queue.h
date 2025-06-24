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

#ifndef _LIBS_SIGNALS_QUEUE_H_
#define _LIBS_SIGNALS_QUEUE_H_

#if defined(SIGNALS_MANAGER_USE_BOOST_LOCKFREE)
    #include <boost/lockfree/queue.hpp>
    #include <boost/lockfree/policies.hpp>
#else
    #include <mutex>
    #include <queue>
#endif

namespace wstux {
namespace signals {
namespace details {

#if ! defined(SIGNALS_MANAGER_USE_BOOST_LOCKFREE)

template<typename T, std::size_t N>
class queue final
{
public:
    bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void pop(T& ret)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        ret = m_queue.front();
        m_queue.pop();
    }

    void push(const T& value)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
};

template<typename T, std::size_t N>
using signals_queue_t = ::wstux::signals::details::queue<T, N>;

#else

template<typename T, std::size_t N>
using signals_queue_t = boost::lockfree::queue<T, boost::lockfree::fixed_sized<true>, boost::lockfree::capacity<N>>;

#endif

} // namespace details
} // namespace signals
} // namespace wstux

#endif /* _LIBS_SIGNALS_QUEUE_H_ */

