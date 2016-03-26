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

#ifdef _WIN32

#include <map>
#include <set>
#include <future>
#include <algorithm>
#include "WinApiWindow.hpp"
#include "BearLibTerminal.h"
#include "Point.hpp"
#include "Log.hpp"
#include "OpenGL.hpp"
#include "Resource.hpp"
#include "Geometry.hpp"
#include "Utility.hpp"

#include <Mmsystem.h>

#if !defined(MAPVK_VSC_TO_VK)
#define MAPVK_VSC_TO_VK 1
#endif

namespace BearLibTerminal
{
	std::wstring GetLastErrorStr(std::uint32_t rc)
	{
		const size_t buffer_size = 256;
		wchar_t buffer[buffer_size];
		DWORD n = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, rc, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), buffer, buffer_size, NULL);
		std::wstring result(buffer, n);
		if ( result.find(L"\r\n") != std::wstring::npos ) result = result.substr(0, result.length()-2);
		if ( result.length() > 0 && result.back() == '.' ) result = result.substr(0, result.length()-1);
		return result;
	}

	std::wstring GetLastErrorStr()
	{
		return GetLastErrorStr(GetLastError());
	}

	uint64_t StartTiming()
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return t.QuadPart;
	}

	uint64_t StopTiming(uint64_t start)
	{
		LARGE_INTEGER t, f;
		QueryPerformanceCounter(&t);
		QueryPerformanceFrequency(&f);
		return ((t.QuadPart-start)/(double)f.QuadPart)*1000000;
	}

	WinApiWindow::WinApiWindow(EventHandler handler):
		Window(handler),
		m_class_name(L"BearLibTerminal"),
		m_handle(nullptr),
		m_device_context(nullptr),
		m_rendering_context(nullptr),
		m_maximized(false),
		m_last_mouse_click(0),
		m_consecutive_mouse_clicks(1),
		m_suppress_wm_paint_once(false),
		m_mouse_cursor_enabled(true),
		m_mouse_cursor_visible(true),
		m_wglSwapIntervalEXT(nullptr),
		m_resizing(false)
	{
		// Raising timing resolution for Delay function.
		timeBeginPeriod(1);

		try
		{
			Construct();
		}
		catch (...)
		{
			Destroy();
			throw;
		}
	}

	WinApiWindow::~WinApiWindow()
	{
		Destroy();

		// Timing resolution restore.
		timeEndPeriod(1);
	}

	HMODULE GetCurrentModule()
	{
		// NB: XP+ solution!
		HMODULE hModule = NULL;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule, &hModule);
		return hModule;
	}

	static HICON LoadIconFromFile(const std::wstring& filename, bool small)
	{
		HICON result = NULL;

		if (filename == L"default")
		{
			result = (HICON)LoadImageW
			(
				(HINSTANCE)GetCurrentModule(),
				(LPWSTR)MAKEINTRESOURCE(100), // FIXME: magic constant
				IMAGE_ICON,
				GetSystemMetrics(small? SM_CXSMICON: SM_CXICON),
				GetSystemMetrics(small? SM_CYSMICON: SM_CYICON),
				0
			);
		}
		else
		{
			result = (HICON)LoadImageW
			(
				NULL,
				filename.c_str(),
				IMAGE_ICON,
				GetSystemMetrics(small? SM_CXSMICON: SM_CXICON),
				GetSystemMetrics(small? SM_CYSMICON: SM_CYICON),
				LR_LOADFROMFILE
			);
		}

		if ( result == NULL )
		{
			LOG(Warning, L"Failed to load " << (small? L"small": L"large") << " icon from file (" << GetLastErrorStr() << ")" );
		}

		return result;
	}

	struct IconDirectoryEntry
	{
		uint8_t width;
		uint8_t height;
		uint8_t colors;
		uint8_t reserved;
		uint16_t planes;
		uint16_t bpp;
		uint32_t size;
		uint32_t offset;
	};

	bool WinApiWindow::ValidateIcon(const std::wstring& filename)
	{
		// TODO
		return true;
	}

	void WinApiWindow::SetTitle(const std::wstring& title)
	{
		SetWindowTextW(m_handle, title.c_str());
	}

	void WinApiWindow::SetIcon(const std::wstring& filename)
	{
		if (filename.empty())
			return;

		HICON new_small_icon = LoadIconFromFile(filename, true);
		if ( new_small_icon != NULL )
		{
			HICON old_small_icon = (HICON)SetClassLongPtr(m_handle, GCLP_HICONSM, (LONG_PTR)new_small_icon);
			DWORD rc = GetLastError();
			if ( rc == 0 )
			{
				if ( old_small_icon != NULL ) DestroyIcon(old_small_icon);
			}
			else
			{
				LOG(Error, L"Failed to apply small icon (" << GetLastErrorStr() << ")");
			}
		}

		HICON new_large_icon = LoadIconFromFile(filename, false);
		if ( new_large_icon != NULL )
		{
			HICON old_large_icon = (HICON)SetClassLongPtr(m_handle, GCLP_HICON, (LONG_PTR)new_large_icon);
			DWORD rc = GetLastError();
			if ( rc == 0 )
			{
				if ( old_large_icon != NULL ) DestroyIcon(old_large_icon);
			}
			else
			{
				LOG(Error, L"Failed to apply large icon (" << GetLastErrorStr() << ")");
			}
		}
	}

	void WinApiWindow::SetClientSize(const Size& size)
	{
		if (!m_fullscreen)
		{
			DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
			RECT rectangle = {0, 0, size.width, size.height};
			AdjustWindowRect(&rectangle, style, FALSE);
			SetWindowPos
			(
				m_handle,
				HWND_NOTOPMOST,
				0, 0,
				rectangle.right-rectangle.left,
				rectangle.bottom-rectangle.top,
				SWP_NOMOVE
			);
		}

		m_client_size = size;
	}

	void WinApiWindow::SetResizeable(bool resizeable)
	{
		if (!m_handle)
			return;

		if (!m_fullscreen)
		{
			if (!resizeable)
			{
				WINDOWPLACEMENT placement;
				GetWindowPlacement(m_handle, &placement);
				if (placement.showCmd & SW_MAXIMIZE)
				{
					ShowWindow(m_handle, SW_RESTORE);
				}
			}

			DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
			DWORD flags = WS_THICKFRAME | WS_MAXIMIZEBOX;
			style = resizeable? style|flags: style^flags;
			SetWindowLongW(m_handle, GWL_STYLE, style);

			RECT rectangle = {0, 0, m_client_size.width, m_client_size.height};
			AdjustWindowRect(&rectangle, style, FALSE);
			BOOL rc = SetWindowPos
			(
				m_handle,
				HWND_NOTOPMOST,
				0, 0,
				rectangle.right-rectangle.left,
				rectangle.bottom-rectangle.top,
				SWP_NOMOVE
			);
		}

		m_resizeable = resizeable;
	}

	void WinApiWindow::SetFullscreen(bool fullscreen)
	{
		if (m_fullscreen == fullscreen)
			return;

		if (m_fullscreen)
		{
			// Leave fullscreen mode

			DWORD required_style = WS_BORDER|WS_CAPTION;
			if (m_resizeable) required_style |= WS_THICKFRAME|WS_MAXIMIZEBOX;

			DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
			style = style|required_style;
			SetWindowLongW(m_handle, GWL_STYLE, style);

			RECT rectangle = {0, 0, m_client_size.width, m_client_size.height};
			AdjustWindowRect(&rectangle, style, FALSE);

			BOOL rc = SetWindowPos
			(
				m_handle,
				HWND_TOP,//NOTOPMOST,
				m_location.x, m_location.y,
				rectangle.right-rectangle.left,
				rectangle.bottom-rectangle.top,
				0
			);
		}
		else
		{
			// Enter fullscreen mode

			RECT rect;
			GetWindowRect(m_handle, &rect);
			m_location = Point(rect.left, rect.top);

			DWORD unnecessary_style = WS_BORDER|WS_CAPTION|WS_THICKFRAME|WS_MAXIMIZEBOX;

			DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
			style &= ~unnecessary_style;
			SetWindowLongW(m_handle, GWL_STYLE, style);

			int width = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);

			// Make sure this window is placed over everyting (taskbar is especially stubborn here)
			SetWindowPos(m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);
			SetWindowPos(m_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);

			BOOL rc = SetWindowPos
			(
				m_handle,
				HWND_TOP,
				0, 0,
				width,
				height,
				0
			);

			SetForegroundWindow(m_handle);
		}

		m_fullscreen = fullscreen;//!m_fullscreen;

		// Force window update
		InvalidateRect(m_handle, NULL, FALSE);
	}

	void WinApiWindow::SetCursorVisibility(bool visible)
	{
		m_mouse_cursor_enabled = visible;
	}

	Size WinApiWindow::GetActualSize()
	{
		RECT rect;
		GetClientRect(m_handle, &rect);
		return Size(rect.right-rect.left, rect.bottom-rect.top);
	}

	void WinApiWindow::Show()
	{
		if (m_handle != nullptr)
		{
			ShowWindow(m_handle, SW_SHOW);
			SetForegroundWindow(m_handle);
		}
	}

	void WinApiWindow::Hide()
	{
		if (m_handle != nullptr)
		{
			ShowWindow(m_handle, SW_HIDE);
		}
	}

	int WinApiWindow::PumpEvents()
	{
		for(MSG msg; PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE); )
		{
			if (msg.message == WM_QUIT)
			{
				//m_proceed = false; // XXX:
				break;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		// Leave only the last TK_REDRAW event.
		int redraws = 0;
		for (auto i = m_events.rbegin(); i != m_events.rend(); i++)
		{
			if (i->code == TK_REDRAW && ++redraws > 1)
				m_events.erase(std::next(i).base()); // Witchcraft.
		}

		int count = m_events.size();
		for (auto& event: m_events)
			m_event_handler(std::move(event));
		m_events.clear();
		return count;
	}

	bool WinApiWindow::Construct()
	{
		if ( !CreateWindowObject() || !CreateOpenGLContext() )
		{
			DestroyUnlocked();
			return false;
		}

		return true;
	}

	void WinApiWindow::Destroy()
	{
		DestroyUnlocked();
	}

	void WinApiWindow::DestroyUnlocked()
	{
		DestroyOpenGLContext();
		DestroyWindowObject();
	}

	bool WinApiWindow::CreateWindowObject()
	{
		Size size(640, 480);
		std::wstring m_title(L"BearLibTerminal");

		HINSTANCE app_instance = GetModuleHandle(NULL);

		WNDCLASSW wc;
		memset(&wc, 0, sizeof(wc));
		wc.style			= /*CS_HREDRAW | CS_VREDRAW |*/ CS_OWNDC;
		wc.lpfnWndProc		= (WNDPROC)SharedWindowProc;
		wc.hInstance		= app_instance;
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName	= m_class_name.c_str();

		if ( !RegisterClassW(&wc) )
		{
			LOG(Fatal, L"Failed to register window class (" << GetLastErrorStr() << ")");
			return false; // Nothing to destroy yet
		}

		DWORD style =
			WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|
			WS_CLIPSIBLINGS|WS_CLIPCHILDREN;

		RECT rect = { 0, 0, size.width, size.height };
		AdjustWindowRect(&rect, style, FALSE);
		int width = rect.right-rect.left;
		int height = rect.bottom-rect.top;

		m_handle = CreateWindowW
			(
				m_class_name.c_str(),
				m_title.c_str(),
				style,
				CW_USEDEFAULT, CW_USEDEFAULT,
				width, height,
				NULL, NULL,
				app_instance,
				NULL
			);

		if ( m_handle == nullptr )
		{
			LOG(Fatal, L"Failed to create window (" << GetLastErrorStr() << ")");
			DestroyWindowObject();
			return false;
		}

		// Hook custom WndProc
		SetWindowLongPtrW(m_handle, GWLP_USERDATA, (LONG_PTR)(void*)this);

		return true;
	}

	bool WinApiWindow::CreateOpenGLContext()
	{
		m_device_context = GetDC(m_handle);
		if ( m_device_context == nullptr )
		{
			LOG(Fatal, L"Failed to retrieve device context (" << GetLastErrorStr() << ")");
			return false; // Nothing to destroy yet
		}

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24; // At least, driver may choose PFD with higher color depth
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixel_format = ChoosePixelFormat(m_device_context, &pfd);
		if ( pixel_format == 0 )
		{
			LOG(Fatal, L"Failed to choose suitable pixel format (" << GetLastErrorStr() << ")");
			DestroyOpenGLContext();
			return false;
		}

		if ( SetPixelFormat(m_device_context, pixel_format, &pfd) == 0 )
		{
			LOG(Fatal, L"Failed to apply chosen pixel format (" << GetLastErrorStr() << ")");
			DestroyOpenGLContext();
			return false;
		}

		m_rendering_context = wglCreateContext(m_device_context);
		if ( m_rendering_context == nullptr )
		{
			LOG(Fatal, L"Failed to create OpenGL context (" << GetLastErrorStr() << ")");
			DestroyOpenGLContext();
			return false;
		}

		if (!AcquireRC())
		{
			LOG(Fatal, L"Failed to acquire rendering context");
			DestroyOpenGLContext();
			return false;
		}
		ProbeOpenGL();

		m_wglSwapIntervalEXT = (PFN_WGLSWAPINTERVALEXT)wglGetProcAddress("wglSwapIntervalEXT");
		SetVSync(true);

		return true;
	}

	bool WinApiWindow::AcquireRC()
	{
		if (wglMakeCurrent(m_device_context, m_rendering_context) == 0)
		{
			LOG(Error, L"Failed to bind OpenGL context (" << GetLastErrorStr() << ")");
			return false;
		}

		return true;
	}

	bool WinApiWindow::ReleaseRC()
	{
		if (wglMakeCurrent(NULL, NULL) == 0)
		{
			LOG(Error, L"Failed to unbind OpenGL context (" << GetLastErrorStr() << ")");
			return false;
		}

		return true;
	}

	void WinApiWindow::SwapBuffers()
	{
		::SwapBuffers(m_device_context);
	}

	void WinApiWindow::SetVSync(bool enabled)
	{
		if (m_wglSwapIntervalEXT)
			m_wglSwapIntervalEXT(enabled? 1: 0);
	}

	void WinApiWindow::DestroyWindowObject()
	{
		if ( m_handle )
		{
			DestroyWindow(m_handle);
			m_handle = nullptr;
		}

		HINSTANCE app_instance = GetModuleHandleW(NULL);
		UnregisterClassW(m_class_name.c_str(), app_instance);
	}

	void WinApiWindow::DestroyOpenGLContext()
	{
		if ( m_rendering_context != nullptr )
		{
			ReleaseRC();
			m_rendering_context = nullptr;
		}

		if ( m_device_context )
		{
			ReleaseDC(m_handle, m_device_context);
			m_device_context = nullptr;
		}
	}

	void WinApiWindow::Redraw()
	{
		m_events.push_back(TK_REDRAW);
	}

	LRESULT CALLBACK WinApiWindow::SharedWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WinApiWindow* p = (WinApiWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

		return (p == nullptr)?
			DefWindowProc(hwnd, uMsg, wParam, lParam):
			p->WindowProc(uMsg, wParam, lParam);
	}

	/*
	LRESULT WinApiWindow::HandleWmPaint(WPARAM wParam, LPARAM lParam)
	{
		if (!m_suppress_wm_paint_once)
		{
			try
			{
				Redraw();
			}
			catch (std::exception& e)
			{
				LOG(Fatal, L"Rendering routine has thrown an exception: " << e.what());
				//m_proceed = false; // XXX:
			}
		}
		else
		{
			m_suppress_wm_paint_once = false;
		}

		// Mark window area as processed
		RECT rect;
		GetClientRect(m_handle, &rect);
		ValidateRect(m_handle, &rect);

		return FALSE;
	}
	//*/

	static int MapNavigationScancodeToNumpad(uint8_t scancode)
	{
		static std::map<uint8_t, int> mapping =
		{
			{VK_DELETE, TK_KP_PERIOD},
			{VK_INSERT, TK_KP_0},
			{VK_END,    TK_KP_1},
			{VK_DOWN,   TK_KP_2},
			{VK_NEXT,   TK_KP_3},
			{VK_LEFT,   TK_KP_4},
			{VK_CLEAR,  TK_KP_5},
			{VK_RIGHT,  TK_KP_6},
			{VK_HOME,   TK_KP_7},
			{VK_UP,     TK_KP_8},
			{VK_PRIOR,  TK_KP_9},
			{VK_RETURN, TK_KP_ENTER}
		};

		return mapping[scancode];
	}

	static wchar_t MapOEMScancodeToCharacter(uint8_t scancode)
	{
		static std::map<uint8_t, wchar_t> mapping =
		{
			{VK_OEM_1,      L';'},
			{VK_OEM_PLUS,   L'='},
			{VK_OEM_COMMA,  L','},
			{VK_OEM_MINUS,  L'-'},
			{VK_OEM_PERIOD, L'.'},
			{VK_OEM_2,      L'/'},
			{VK_OEM_3,      L'`'},
			{VK_OEM_4,      L'['},
			{VK_OEM_5,      L'\\'},
			{VK_OEM_6,      L']'},
			{VK_OEM_7,      L'\''},
			{VK_OEM_102,    L'\\'}
		};

		return mapping[scancode];
	}

	static int MapWindowsScancodeToTerminal(uint8_t scancode)
	{
		static std::map<uint8_t, int> mapping =
		{
			{'A',           TK_A},
			{'B',           TK_B},
			{'C',           TK_C},
			{'D',           TK_D},
			{'E',           TK_E},
			{'F',           TK_F},
			{'G',           TK_G},
			{'H',           TK_H},
			{'I',           TK_I},
			{'J',           TK_J},
			{'K',           TK_K},
			{'L',           TK_L},
			{'M',           TK_M},
			{'N',           TK_N},
			{'O',           TK_O},
			{'P',           TK_P},
			{'Q',           TK_Q},
			{'R',           TK_R},
			{'S',           TK_S},
			{'T',           TK_T},
			{'U',           TK_U},
			{'V',           TK_V},
			{'W',           TK_W},
			{'X',           TK_X},
			{'Y',           TK_Y},
			{'Z',           TK_Z},
			{'1',           TK_1},
			{'2',           TK_2},
			{'3',           TK_3},
			{'4',           TK_4},
			{'5',           TK_5},
			{'6',           TK_6},
			{'7',           TK_7},
			{'8',           TK_8},
			{'9',           TK_9},
			{'0',           TK_0},
			{VK_RETURN,     TK_RETURN},
			{VK_ESCAPE,     TK_ESCAPE},
			{VK_BACK,       TK_BACKSPACE},
			{VK_TAB,        TK_TAB},
			{VK_SPACE,      TK_SPACE},
			{VK_OEM_MINUS,  TK_MINUS},
			{VK_OEM_PLUS,   TK_EQUALS},
			{VK_OEM_4,      TK_LBRACKET},
			{VK_OEM_6,      TK_RBRACKET},
			{VK_OEM_5,      TK_BACKSLASH},
			{VK_OEM_1,      TK_SEMICOLON},
			{VK_OEM_7,      TK_APOSTROPHE},
			{VK_OEM_3,      TK_GRAVE},
			{VK_OEM_COMMA,  TK_COMMA},
			{VK_OEM_PERIOD, TK_PERIOD},
			{VK_OEM_2,      TK_SLASH},
			{VK_F1,         TK_F1},
			{VK_F2,         TK_F2},
			{VK_F3,         TK_F3},
			{VK_F4,         TK_F4},
			{VK_F5,         TK_F5},
			{VK_F6,         TK_F6},
			{VK_F7,         TK_F7},
			{VK_F8,         TK_F8},
			{VK_F9,         TK_F9},
			{VK_F10,        TK_F10},
			{VK_F11,        TK_F11},
			{VK_F12,        TK_F12},
			{VK_PAUSE,      TK_PAUSE},
			{VK_INSERT,     TK_INSERT},
			{VK_HOME,       TK_HOME},
			{VK_PRIOR,      TK_PAGEUP},
			{VK_DELETE,     TK_DELETE},
			{VK_END,        TK_END},
			{VK_NEXT,       TK_PAGEDOWN},
			{VK_RIGHT,      TK_RIGHT},
			{VK_LEFT,       TK_LEFT},
			{VK_DOWN,       TK_DOWN},
			{VK_UP,         TK_UP},
			{VK_SHIFT,      TK_SHIFT},
			{VK_CONTROL,    TK_CONTROL},
			{VK_NUMPAD0,    TK_KP_0},
			{VK_NUMPAD1,    TK_KP_1},
			{VK_NUMPAD2,    TK_KP_2},
			{VK_NUMPAD3,    TK_KP_3},
			{VK_NUMPAD4,    TK_KP_4},
			{VK_NUMPAD5,    TK_KP_5},
			{VK_NUMPAD6,    TK_KP_6},
			{VK_NUMPAD7,    TK_KP_7},
			{VK_NUMPAD8,    TK_KP_8},
			{VK_NUMPAD9,    TK_KP_9},
			{VK_MULTIPLY,   TK_KP_MULTIPLY},
			{VK_ADD,        TK_KP_PLUS},
			{VK_SUBTRACT,   TK_KP_MINUS},
			{VK_DECIMAL,    TK_KP_PERIOD},
			{VK_DIVIDE,     TK_KP_DIVIDE}
		};

		return mapping[scancode];
	}

	static std::map<int, std::pair<bool, int>> kMouseButtonMapping = // WM_XXX -> {is_pressed, TK_XXX}
	{
		{WM_LBUTTONDOWN, {true,  TK_MOUSE_LEFT}},
		{WM_LBUTTONUP,   {false, TK_MOUSE_LEFT}},
		{WM_RBUTTONDOWN, {true,  TK_MOUSE_RIGHT}},
		{WM_RBUTTONUP,   {false, TK_MOUSE_RIGHT}},
		{WM_MBUTTONDOWN, {true,  TK_MOUSE_MIDDLE}},
		{WM_MBUTTONUP,   {false, TK_MOUSE_MIDDLE}},
		{WM_XBUTTONDOWN, {true,  TK_MOUSE_X1}}, // It's actually either X1 or X2 depending on wParam
		{WM_XBUTTONUP,   {false, TK_MOUSE_X1}},
	};

	BOOL UnadjustWindowRect(LPRECT prc, DWORD dwStyle, BOOL fMenu)
	{
		RECT rc;
		SetRectEmpty(&rc);
		BOOL fRc = AdjustWindowRect(&rc, dwStyle, fMenu);
		if (fRc)
		{
			prc->left -= rc.left;
			prc->top -= rc.top;
			prc->right -= rc.right;
			prc->bottom -= rc.bottom;
		}

		return fRc;
	}

	BOOL UnadjustWindowRectEx(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle)
	{
		RECT rc;
		SetRectEmpty(&rc);
		BOOL fRc = AdjustWindowRectEx(&rc, dwStyle, fMenu, dwExStyle);
		if (fRc)
		{
			prc->left -= rc.left;
			prc->top -= rc.top;
			prc->right -= rc.right;
			prc->bottom -= rc.bottom;
		}

		return fRc;
	}

	LRESULT WinApiWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CLOSE)
		{
			// Emulate pressing virtual VK_CLOSE button
			m_events.push_back(TK_CLOSE);
			return FALSE;
		}
		else if (uMsg == WM_PAINT)
		{
			// Schedule a redraw.
			m_events.push_back(TK_REDRAW);

			// Mark window area as updated.
			RECT rect;
			GetClientRect(m_handle, &rect);
			ValidateRect(m_handle, &rect);

			return FALSE;
		}
		else if (uMsg == WM_ERASEBKGND)
		{
			return TRUE;
		}
		else if (uMsg == WM_CHAR)
		{
			// This handles all keys with printable characters, as well as
			// * Escape, Tab, Backspace, Enter

			wchar_t charcode = (wchar_t)wParam;
			uint8_t scancode = ((uint32_t)lParam >> 16) & 0xFF;
			bool extended = (lParam & (1<<24)) > 0;
			scancode = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);

			if (scancode == VK_INSERT ||
			    scancode == VK_DELETE ||
			   (scancode >= VK_PRIOR && scancode <= VK_DOWN) ||
			   (charcode == '/' && extended))
			{
				// Ignore, handled by WM_KEYxxx
			}
			else if (scancode == VK_ESCAPE ||
			         scancode == VK_TAB ||
			         scancode == VK_BACK ||
			         scancode == VK_RETURN)
			{
				int code = (scancode == VK_RETURN && extended)?
					TK_KP_ENTER: MapWindowsScancodeToTerminal(scancode);

				if (code > 0)
				{
					//m_event_handler(Event{code, {{code, 1}}});
					m_events.push_back(Event{code, {{code, 1}}});
				}
			}
			else
			{
				// Convert OEM scancodes (brackets, commas, etc.)
				if (scancode == VK_OEM_102 ||
				   (scancode >= VK_OEM_1 && scancode <= VK_OEM_3) ||
				   (scancode >= VK_OEM_4 && scancode <= VK_OEM_7))
				{
					//scancode = MapOEMScancodeToCharacter(scancode);
				}

				if (scancode == VK_OEM_102) scancode = VK_OEM_5;

				int code = MapWindowsScancodeToTerminal(scancode);
				if (code > 0)
				{
					Event event(code);
					event[code] = 1; // Pressed
					event[TK_WCHAR] = charcode;
					m_events.push_back(event);
				}
			}

			return FALSE;
		}
		else if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
		{
			// This handles keys not handled by WM_CHAR message
			// * Ctrl, Shift
			// * F1-F12
			// * Break
			// * Navigation: Insert, Home, PgUp, Delete, End, PgDown + Arrows
			// * Numpad
			// * Alt+... combinations

			uint8_t scancode = (uint8_t)(wParam & 0xFF);
			bool extended = (lParam & (1<<24)) > 0;
			bool pressed = (uMsg == WM_KEYDOWN) || (uMsg == WM_SYSKEYDOWN);

			if (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP)
			{
				int code = MapWindowsScancodeToTerminal(scancode);
				std::set<int> codes = {TK_F10, TK_ALT, TK_RETURN, TK_A, TK_G, TK_MINUS, TK_EQUALS};

				if (codes.count(code))
				{
					// Set Alt state
					m_events.push_back(Event{TK_ALT, {{TK_ALT, 1}}});

					Event event(code|(pressed? 0: TK_KEY_RELEASED));
					event[code] = pressed? 1: 0;
					m_events.push_back(event);

					// Clear Alt state
					m_events.push_back(Event{TK_ALT|TK_KEY_RELEASED, {{TK_ALT, 0}}});

					return FALSE;
				}
				else
				{
					return DefWindowProc(m_handle, uMsg, wParam, lParam);
				}
			}

			if (scancode == VK_CONTROL ||
			    scancode == VK_SHIFT ||
			    scancode == VK_PAUSE ||
			   (scancode >= VK_F1 && scancode <= VK_F12) || // Functional keys
			   (scancode >= VK_NUMPAD0 && scancode <= VK_DIVIDE)) // Numpad keys
			{
				int code = MapWindowsScancodeToTerminal(scancode);
				if (code > 0)
				{
					Event event(code|(pressed? 0: TK_KEY_RELEASED));
					event[code] = pressed? 1: 0;
					m_events.push_back(event);
				}
			}
			else if (scancode == VK_CLEAR ||
					 scancode == VK_INSERT ||
					 scancode == VK_DELETE ||
					(scancode >= VK_PRIOR && scancode <= VK_DOWN)) // Navigation keys
			{
				int code = MapWindowsScancodeToTerminal(scancode);
				if ( !extended ) code = MapNavigationScancodeToNumpad(scancode);
				if (code > 0)
				{
					Event event(code|(pressed? 0: TK_KEY_RELEASED));
					event[code] = pressed? 1: 0;
					m_events.push_back(event);
				}
			}
			else if (!pressed)
			{
				// Handle key release since WM_CHAR does not intercept those
				// OEM must be converted both on press and on release

				wchar_t charcode = 0;

				if (scancode == VK_OEM_102 ||
				   (scancode >= VK_OEM_1 && scancode <= VK_OEM_3) ||
				   (scancode >= VK_OEM_4 && scancode <= VK_OEM_7))
				{
					charcode = MapOEMScancodeToCharacter(scancode);
				}

				if (scancode == VK_OEM_102) scancode = VK_OEM_5;

				int code = (scancode == VK_RETURN && extended)?
					TK_KP_ENTER: MapWindowsScancodeToTerminal(scancode);

				if (code > 0)
				{
					Event event(code|TK_KEY_RELEASED);
					event[code] = 0;
					m_events.push_back(event);
				}
			}

			return FALSE;
		}
		else if (uMsg == WM_MENUCHAR)
		{
			return MNC_CLOSE << 16;
		}
		else if (uMsg == WM_MOUSEMOVE)
		{
			Point precise_position(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (precise_position.x < 0) precise_position.x = 0;
			if (precise_position.y < 0) precise_position.y = 0;

			Event event(TK_MOUSE_MOVE);
			event[TK_MOUSE_PIXEL_X] = precise_position.x;
			event[TK_MOUSE_PIXEL_Y] = precise_position.y;
			m_events.push_back(event);

			return 0;
		}
		else if (uMsg == WM_MOUSEWHEEL)
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			Event event(TK_MOUSE_SCROLL);
			event[TK_MOUSE_WHEEL] = delta > 0? -1: +1; // FIXME: multiple events in case of large delta
			m_events.push_back(event);

			return 0;
		}
		else if (kMouseButtonMapping.count(uMsg))
		{
			auto& desc = kMouseButtonMapping[uMsg];
			bool pressed = desc.first;
			int code = desc.second;

			if (code == TK_MOUSE_X1)
			{
				// Higher word of wParam if 1 for X1 and 2 for X2.
				code += (HIWORD(wParam)-1);
			}

			if (pressed) // TODO: Maybe, an option to match X11?
			{
				uint64_t now = gettime();
				uint64_t delta = now - m_last_mouse_click;
				m_last_mouse_click = now;

				if (delta < GetDoubleClickTime()*1000)
				{
					m_consecutive_mouse_clicks += 1;
				}
				else
				{
					m_consecutive_mouse_clicks = 1;
				}
			}

			Event event(code | (pressed? 0: TK_KEY_RELEASED));
			event[code] = pressed? 1: 0;
			event[TK_MOUSE_CLICKS] = pressed? m_consecutive_mouse_clicks: 0;
			m_events.push_back(event);
		}
		else if (uMsg == WM_ACTIVATE)
		{
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				m_events.push_back(TK_ACTIVATED);
			}
		}
		else if (uMsg == WM_SIZE)
		{
			if (wParam == SIZE_MAXIMIZED || (wParam == SIZE_RESTORED && m_maximized))
			{
				m_maximized = (wParam == SIZE_MAXIMIZED);

				RECT rect;
				GetClientRect(m_handle, &rect);
				m_client_size.width = rect.right - rect.left;
				m_client_size.height = rect.bottom - rect.top;

				Event event(TK_RESIZED);
				event[TK_WIDTH] = m_client_size.width;
				event[TK_HEIGHT] = m_client_size.height;
				m_events.push_back(event);
			}

			m_suppress_wm_paint_once = true;

			if (m_resizing)
			{
				// During resizing, Windows does not return control so it is necessary
				// to send events directly to the Terminal.
				m_event_handler(TK_INVALIDATE);
				m_event_handler(TK_REDRAW);
			}
			else
			{
				m_events.push_back(TK_INVALIDATE);
				m_events.push_back(TK_REDRAW);
			}

			return TRUE;
		}
		else if (uMsg == WM_ENTERSIZEMOVE && m_resizeable)
		{
			m_resizing = true;
		}
		else if (uMsg == WM_EXITSIZEMOVE && m_resizeable)
		{
			RECT rect;
			GetClientRect(m_handle, &rect);
			m_client_size.width = rect.right - rect.left;
			m_client_size.height = rect.bottom - rect.top;

			Size snapped = m_cell_size*std::floor(m_client_size/m_cell_size.As<float>());

			if (snapped != m_client_size)
			{
				m_client_size = snapped;

				DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
				RECT rectangle = {0, 0, m_client_size.width, m_client_size.height};
				AdjustWindowRect(&rectangle, style, FALSE);
				SetWindowPos
				(
					m_handle,
					HWND_NOTOPMOST,
					0, 0,
					rectangle.right-rectangle.left,
					rectangle.bottom-rectangle.top,
					SWP_NOMOVE
				);
			}

			m_suppress_wm_paint_once = true;

			m_events.push_back(Event{TK_RESIZED, {{TK_WIDTH, m_client_size.width}, {TK_HEIGHT, m_client_size.height}}});
			m_events.push_back(TK_INVALIDATE);
			m_events.push_back(TK_REDRAW);

			m_resizing = false;

			return FALSE;
		}
		else if (uMsg == WM_SIZING)
		{
			DWORD style = GetWindowLongW(m_handle, GWL_STYLE);
			auto rect = (RECT*)lParam;

			RECT client_rect = *rect;
			UnadjustWindowRect(&client_rect, style, FALSE);
			Size size(client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);

			Size minimum = m_minimum_size * m_cell_size;

			if (size.width < minimum.width)
			{
				int delta = minimum.width - size.width;

				if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
				{
					rect->left -= delta;
				}
				else if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
				{
					rect->right += delta;
				}
			}

			if (size.height < minimum.height)
			{
				int delta = minimum.height - size.height;

				if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
				{
					rect->top -= delta;
				}
				else if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
				{
					rect->bottom += delta;
				}
			}

			m_events.push_back(TK_INVALIDATE);

			return TRUE;
		}
		else if (uMsg == WM_GETMINMAXINFO)
		{
			auto info = (MINMAXINFO*)lParam;
			info->ptMaxTrackSize.x = LONG_MAX;
			info->ptMaxTrackSize.y = LONG_MAX;
			return FALSE;
		}
		else if (uMsg == WM_SETCURSOR)
		{
			WORD ht = LOWORD(lParam);
			if (ht == HTCLIENT && !m_mouse_cursor_enabled && m_mouse_cursor_visible)
			{
				m_mouse_cursor_visible = false;
				ShowCursor(false);
			}
			else if (!m_mouse_cursor_visible && (m_mouse_cursor_enabled || ht != HTCLIENT))
			{
				m_mouse_cursor_visible = true;
				ShowCursor(true);
			}
		}

		return DefWindowProc(m_handle, uMsg, wParam, lParam);
	}
}

#endif
