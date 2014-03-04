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

#include "Window.hpp"
#if defined(__linux)
#include "X11Window.hpp"
#endif
#if defined(_WIN32)
#include "WinApiWindow.hpp"
#endif
#include "Log.hpp"
#include <future>
#include <stdexcept>

namespace BearLibTerminal
{
	Window::Window():
		m_synchronous_redraw(false),
		m_proceed(false),
		m_minimum_size(1, 1)
	{ }

	Window::~Window()
	{ }

	void Window::SetOnRedraw(DrawEventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_redraw = callback;
	}

	void Window::SetOnInput(InputEventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_input = callback;
	}

	void Window::SetOnDeactivate(EventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_deactivate = callback;
	}

	void Window::SetOnActivate(EventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_activate = callback;
	}

	void Window::SetOnDestroy(EventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_destroy = callback;
	}

	void Window::SetSizeHints(Size increment, Size minimum_size)
	{
		m_cell_size = increment;
		m_minimum_size = minimum_size;

		if (m_minimum_size.width < 1) m_minimum_size.width = 1;
		if (m_minimum_size.height < 1) m_minimum_size.height = 1;
	}

	Size Window::GetClientSize()
	{
		return m_client_size;
	}

	void Window::RunAsynchronous()
	{
		auto thread_function = [&](std::shared_ptr<std::promise<bool>> promise)
		{
			try
			{
				try
				{
					Construct();
					promise->set_value(true);
				}
				catch(...)
				{
					promise->set_exception(std::current_exception());
					return;
				}

				ThreadFunction();
				if (m_on_destroy) m_on_destroy();
			}
			catch (std::exception& e)
			{
				// By the time execution falls here, Run method has already finished.
				// So there is no one to relay exception to.
				LOG(Error, "Window thread has thrown an exception: " << e.what());
			}

			Destroy();
		};

		// --------------------------------------------------------------------

		try
		{
			auto promise = std::make_shared<std::promise<bool>>();
			auto result = promise->get_future();
			m_thread = std::thread(thread_function, promise);
			result.get(); // This will either get 'true' or throw an exception
		}
		catch(std::exception& e)
		{
			throw std::runtime_error(std::string("window initialization has failed: ") + e.what());
		}
	}

	void Window::Stop()
	{
		m_proceed = false;

		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	std::unique_ptr<Window> Window::Create()
	{
		std::unique_ptr<Window> result;

#if defined(__linux)
		result.reset(new X11Window());
#endif
#if defined(_WIN32)
		result.reset(new WinApiWindow());
#endif

		result->RunAsynchronous();
		return std::move(result);
	}
}
