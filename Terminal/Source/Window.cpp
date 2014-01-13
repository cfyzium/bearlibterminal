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

namespace BearLibTerminal
{
	Window::Window():
		m_synchronous_redraw(false),
		m_proceed(false)
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

	void Window::SetOnResize(ResizeEventHandler callback)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_on_resize = callback;
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

	void Window::SetCellSize(const Size& size)
	{
		m_cell_size = size;
	}

	Size Window::GetClientSize()
	{
		return m_client_size;
	}

	void Window::RunAsynchronous()
	{
		auto thread_function = [&](std::promise<bool> promise)
		{
			// NOTE: this lambda will be executed on a new thread
			// (so any uncaught exception becomes fatal)

			try
			{
				Construct();
				promise.set_value(true);
			}
			catch ( std::exception& e )
			{
				// TODO: Log
				promise.set_value(false);
				return;
			}

			try
			{
				ThreadFunction();
				if ( m_on_destroy ) m_on_destroy();
			}
			catch ( std::exception& e )
			{
				// TODO: Log
			}

			Destroy();
		};

		// --------------------------------------------------------------------

		try
		{
			std::promise<bool> promise;
			std::future<bool> result = promise.get_future();
			m_thread = std::thread(thread_function, std::move(promise));

			if ( !result.get() )
			{
				throw std::runtime_error("Failed to create a window");
			}
		}
		catch( std::exception& e )
		{
			LOG(Fatal, L"Window initialization routine has thrown an exception: " << e.what());
			// TODO: chain the exception
			throw;
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
