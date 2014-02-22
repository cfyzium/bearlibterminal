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

#ifndef BEARLIBTERMINAL_WINDOW_HPP
#define BEARLIBTERMINAL_WINDOW_HPP

#include "Log.hpp"
#include "Size.hpp"
#include "Keystroke.hpp"
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <utility>
#include <functional>

// For internal usage
#define TK_ALT 0x12

namespace BearLibTerminal
{
	using EventHandler = std::function<void()>;
	using DrawEventHandler = std::function<int()>;
	using InputEventHandler = std::function<void(Keystroke)>;

	class Window
	{
	public:
		virtual ~Window();
		void SetOnRedraw(DrawEventHandler callback);
		void SetOnInput(InputEventHandler callback);
		void SetOnDeactivate(EventHandler callback);
		void SetOnActivate(EventHandler callback);
		void SetOnDestroy(EventHandler callback);
		Size GetClientSize();
		virtual bool ValidateIcon(const std::wstring& filename) = 0;
		virtual void SetTitle(const std::wstring& title) = 0;
		virtual void SetIcon(const std::wstring& filename) = 0;
		virtual void SetSizeHints(Size increment, Size minimum_size);
		virtual void SetClientSize(const Size& size) = 0;
		virtual void Redraw() = 0;
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void Invoke(std::function<void()> func) = 0;
		virtual bool AcquireRC() = 0;
		virtual bool ReleaseRC() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetResizeable(bool resizeable) = 0;
		static std::unique_ptr<Window> Create();
	protected:
		Window();
		virtual void ThreadFunction() = 0; // noexcept(true)
		virtual bool Construct() = 0;
		virtual void Destroy() = 0; // noexcept(true)
		virtual bool PumpEvents() = 0;
		void RunAsynchronous();
		void Stop();
		DrawEventHandler m_on_redraw;
		EventHandler m_on_deactivate;
		EventHandler m_on_activate;
		EventHandler m_on_destroy;
		InputEventHandler m_on_input;
		bool m_synchronous_redraw;
		std::mutex m_lock;
		std::thread m_thread;
		std::atomic<bool> m_proceed;
		Size m_cell_size;
		Size m_minimum_size;
		Size m_client_size;
	};
}

#endif // BEARLIBTERMINAL_WINDOW_HPP
