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

#ifndef BEARLIBTERMINAL_X11WINDOW3_HPP
#define BEARLIBTERMINAL_X11WINDOW3_HPP

#ifdef __linux__

#include <memory>
#include "Window.hpp"
#include "Semaphore.hpp"
#include "Point.hpp"

namespace BearLibTerminal
{
	class X11Window: public Window
	{
	public:
		X11Window();
		~X11Window();
		bool ValidateIcon(const std::wstring& filename);
		void SetTitle(const std::wstring& title);
		void SetIcon(const std::wstring& filename);
		void SetClientSize(const Size& size);
		void Redraw();
		void Show();
		void Hide();
		std::future<void> Post(std::function<void()> func);
		bool AcquireRC();
		bool ReleaseRC();
		void SwapBuffers();
		void SetVSync(bool enabled);
		bool PumpEvents();
		void SetResizeable(bool resizeable);
		Size GetActualSize();
	protected:
		void ThreadFunction();
		bool Construct();
		void Destroy();
		void DestroyUnlocked();
		bool CreateWindowObject();
		void DestroyWindowObject();
		//void ReportInput(Event event);
		void HandleRepaint();
		void UpdateSizeHints();
	protected:
		struct Private;
		std::unique_ptr<Private> m_private;
		Semaphore m_redraw_barrier;
		Point m_mouse_position;
		int m_mouse_wheel;
		bool m_resizeable;
	};
}

#endif

#endif // BEARLIBTERMINAL_X11WINDOW3_HPP
