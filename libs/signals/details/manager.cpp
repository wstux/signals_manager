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

namespace wstux {
namespace signals {

manager::impl manager::m_impl;

manager::~manager()
{
    clear();
}

void manager::clear()
{
}

void manager::process_signals()
{
}

void manager::remove_handler(sig_num_t)
{
}

bool manager::reset_handler(sig_num_t sig, handler_fn_t func)
{
    return reset_handler(sig, [func](sig_num_t, const sig_info_t&) -> void { func(); });
}

bool manager::reset_handler(sig_num_t, sig_handler_fn_t)
{
    return false;
}

bool manager::set_handler(sig_num_t sig, handler_fn_t func)
{
    return set_handler(sig, [func](sig_num_t, const sig_info_t&) -> void { func(); });
}

bool manager::set_handler(sig_num_t, sig_handler_fn_t)
{
    return false;
}

void manager::stop_process_signals()
{}

} // namespace signals
} // namespace wstux

