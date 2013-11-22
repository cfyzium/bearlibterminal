/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Because there is no semaphore in C++11 standard library

#ifndef BEARLIBTERMINAL_SEMAPHORE_HPP
#define BEARLIBTERMINAL_SEMAPHORE_HPP

#include <condition_variable>
#include <mutex>

namespace BearLibTerminal
{
	class Semaphore
	{
	public:
		Semaphore(): m_count(0)
		{}

		unsigned long GetValue() const
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			return m_count;
		}

		void SetValue(unsigned long value)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_count = value;
		}

		void Notify()
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_count += 1;
			m_condvar.notify_all();
		}

		// FIXME: just make binary semaphore
		void NotifyAtMost(unsigned long limit)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_count += 1;
			if (m_count > limit) m_count = limit;
			m_condvar.notify_all();
		}

		void Wait()
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condvar.wait(lock, [&](){return m_count > 0;});
			m_count -= 1;
		}

		// TODO: use std::chrono for timeout interval
		// FIXME: does not count timeout properly in case of spurious wakeups
		bool WaitFor(int milliseconds)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while (m_count == 0)
			{
				std::cv_status rc = m_condvar.wait_for(lock, std::chrono::milliseconds(milliseconds));
				if (rc == std::cv_status::timeout) return false;
			}
			m_count -= 1;
			return true;
		}

	private:
		mutable std::mutex m_mutex;
		std::condition_variable m_condvar;
		unsigned long m_count;
	};
}

#endif // BEARLIBTERMINAL_SEMAPHORE_HPP
