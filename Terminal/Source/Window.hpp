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
#include "Point.hpp"
#include "Keystroke.hpp"
#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <future>
#include <utility>
#include <functional>

// For internal usage
#define TK_ALT           0x72
#define TK_REDRAW        0x1001
#define TK_INVALIDATE    0x1002
#define TK_DESTROY       0x1003
#define TK_ACTIVATED     0x1004
#define TK_DEACTIVATED   0x1005
#define TK_STATE_UPDATE  0x1006

namespace BearLibTerminal
{
	class Window
	{
	public:
		typedef std::function<int(Event)> EventHandler;
		virtual ~Window();
		virtual Size GetActualSize() = 0; // XXX: GetClientSize?
		virtual void SetTitle(const std::wstring& title) = 0;
		virtual void SetIcon(const std::wstring& filename) = 0;
		virtual void SetSizeHints(Size increment, Size minimum_size);
		virtual void SetClientSize(const Size& size) = 0; // XXX: const& wtf
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetResizeable(bool resizeable) = 0;
		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual void SetCursorVisibility(bool visible) = 0;
		bool IsFullscreen() const;
		virtual int PumpEvents() = 0;
		static std::unique_ptr<Window> Create(EventHandler handler);
	protected:
		Window(EventHandler handler);
		EventHandler m_event_handler;
		Size m_cell_size;
		Size m_minimum_size;
		Point m_location;
		Size m_client_size;
		bool m_fullscreen;
		bool m_resizeable;
	};
}

#endif // BEARLIBTERMINAL_WINDOW_HPP
