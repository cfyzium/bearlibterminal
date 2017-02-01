/*
* BearLibTerminal
* Copyright (C) 2013-2016 Cfyz
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

#include <list>
#include <string>
#include <cstdint>
#include "Window.hpp"
#include "Platform.hpp"

// Force SDK version to XP SP3
#if !defined(WINVER)
#define WINVER 0x502
#define _WIN32_WINNT 0x502
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace BearLibTerminal
{
	// Windows Vista+ functionality, i. e. not available universally and cannot be linked against.
	const DWORD DWMWA_EXTENDED_FRAME_BOUNDS = 9;

	class WinApiWindow: public Window
	{
	public:
		WinApiWindow(EventHandler handler);
		~WinApiWindow();
		bool ValidateIcon(const std::wstring& filename);
		void SetTitle(const std::wstring& title);
		void SetIcon(const std::wstring& filename);
		void SetClientSize(const Size& size);
		void Show();
		void Hide();
		bool AcquireRC();
		bool ReleaseRC();
		void SwapBuffers();
		void SetVSync(bool enabled);
		void SetResizeable(bool resizeable);
		void SetFullscreen(bool fullscreen);
		void SetCursorVisibility(bool visible);
		Size GetActualSize();
		const char* GetClipboard();
		int PumpEvents();
	protected:
		bool Construct();
		void Destroy();
		void DestroyUnlocked();
		bool CreateWindowObject();
		bool CreateOpenGLContext();
		void DestroyWindowObject();
		void DestroyOpenGLContext();
		void Redraw();
		void ClipToScreen(int width, int height, bool center);
		static LRESULT CALLBACK SharedWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT HandleWmPaint(WPARAM wParam, LPARAM lParam);
	protected:
		std::wstring m_class_name;
		HWND m_handle;
		HDC m_device_context;
		HGLRC m_rendering_context;
		bool m_maximized;
		uint64_t m_last_mouse_click;
		int m_consecutive_mouse_clicks;
		bool m_suppress_wm_paint_once;
		bool m_mouse_cursor_enabled;
		bool m_mouse_cursor_visible;
		std::list<Event> m_events;
		bool m_resizing;
		bool m_has_been_shown;
		Module::Function<BOOL, stdcall_t, int> m_wglSwapIntervalEXT;
		Module::Function<HRESULT, stdcall_t, HWND, DWORD, PVOID, DWORD> m_DwmGetWindowAttribute;
	};
}

#endif // _WIN32

#endif // BEARLIBTERMINAL_WINAPIWINDOW_HPP
