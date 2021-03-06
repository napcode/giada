/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#ifndef G_QUEUE_H
#define G_QUEUE_H


#include <array>
#include <atomic>


namespace giada {
namespace m
{
/* Queue
Single producer, single consumer lock-free queue. */

template<typename T, size_t size>
class Queue
{
public:

    Queue() : m_head(0), m_tail(0)
    {
    }


    Queue(const Queue&) = delete;


    bool pop(T& item)
    {
        size_t curr = m_head.load();
        if (curr == m_tail.load())  // Queue empty, nothing to do
            return false;

        item = m_data[curr];
        m_head.store(increment(curr));
        return true;
    }


    bool push(const T& item)
    {
        size_t curr = m_tail.load();
        size_t next = increment(curr);

        if (next == m_head.load()) // Queue full, nothing to do
            return false;

        m_data[curr] = item;
        m_tail.store(next);
        return true;
    }

private:

    size_t increment(size_t i) const
    {
        return (i + 1) % size;
    }


    std::array<T, size> m_data;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
};
}} // giada::m::


#endif
