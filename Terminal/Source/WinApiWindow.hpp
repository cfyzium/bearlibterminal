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

#ifndef BEARLIBTERMINAL_WINAPIWINDOW_HPP
#define BEARLIBTERMINAL_WINAPIWINDOW_HPP

#ifdef _WIN32

#include <string>
#include <cstdint>
#include "Window.hpp"
#include "Semaphore.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WindowsX.h>

namespace BearLibTerminal
{
	class WinApiWindow: public Window
	{
	public:
		WinApiWindow();
		~WinApiWindow();
		bool ValidateIcon(const std::wstring& filename) override;
		void SetTitle(const std::wstring& title) override;
		void SetIcon(const std::wstring& filename) override;
		void SetClientSize(const Size& size) override;
		void Redraw() override;
		void Show() override;
		void Hide() override;
		void Invoke(std::function<void()> func) override;
		bool AcquireRC() override;
		bool ReleaseRC() override;
		void SwapBuffers() override;
	protected:
		void ThreadFunction() override;
		bool Construct() override;
		void Destroy() override;
		void DestroyUnlocked();
		bool CreateWindowObject();
		bool CreateOpenGLContext();
		void DestroyWindowObject();
		void DestroyOpenGLContext();
		static LRESULT CALLBACK SharedWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT HandleWmPaint(WPARAM wParam, LPARAM lParam);
		void ReportInput(const Keystroke& keystroke);
	protected:
		std::wstring m_class_name;
		HWND m_handle;
		HDC m_device_context;
		HGLRC m_rendering_context;
		Semaphore m_redraw_barrier;
		int m_mouse_wheel;
	};
}

#endif

#endif // BEARLIBTERMINAL_WINAPIWINDOW_HPP
