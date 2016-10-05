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

#ifdef __linux__

#include "X11Window.hpp"
#include "OpenGL.hpp"
#include "Log.hpp"
#include "Encoding.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"
#include <unistd.h>
#include <sys/time.h>
#include <future>
#include <iostream>
#include <limits.h>
#include <string.h>
#include <X11/Xatom.h>

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"

// X11 hack because X.h #defines these names without any regards to others
#define XlibKeyPress 2
#define XlibKeyRelease 3
#undef KeyPress
#undef KeyRelease

namespace BearLibTerminal
{
	void X11Window::InitKeymaps()
	{
		auto& keymaps = m_keymaps;

		memset(keymaps, 0, sizeof(keymaps));

		// Generic keyboard keys
		// TODO: what about non-qwerty?
		keymaps[0][0x0A] = TK_1; // First row
		keymaps[0][0x0B] = TK_2;
		keymaps[0][0x0C] = TK_3;
		keymaps[0][0x0D] = TK_4;
		keymaps[0][0x0E] = TK_5;
		keymaps[0][0x0F] = TK_6;
		keymaps[0][0x10] = TK_7;
		keymaps[0][0x11] = TK_8;
		keymaps[0][0x12] = TK_9;
		keymaps[0][0x13] = TK_0;
		keymaps[0][0x14] = TK_MINUS;
		keymaps[0][0x15] = TK_EQUALS;
		keymaps[0][0x18] = TK_Q; // Second row
		keymaps[0][0x19] = TK_W;
		keymaps[0][0x1A] = TK_E;
		keymaps[0][0x1B] = TK_R;
		keymaps[0][0x1C] = TK_T;
		keymaps[0][0x1D] = TK_Y;
		keymaps[0][0x1E] = TK_U;
		keymaps[0][0x1F] = TK_I;
		keymaps[0][0x20] = TK_O;
		keymaps[0][0x21] = TK_P;
		keymaps[0][0x22] = TK_LBRACKET;
		keymaps[0][0x23] = TK_RBRACKET;
		keymaps[0][0x26] = TK_A; // Third row
		keymaps[0][0x27] = TK_S;
		keymaps[0][0x28] = TK_D;
		keymaps[0][0x29] = TK_F;
		keymaps[0][0x2A] = TK_G;
		keymaps[0][0x2B] = TK_H;
		keymaps[0][0x2C] = TK_J;
		keymaps[0][0x2D] = TK_K;
		keymaps[0][0x2E] = TK_L;
		keymaps[0][0x2F] = TK_SEMICOLON;
		keymaps[0][0x30] = TK_APOSTROPHE;
		keymaps[0][0x31] = TK_GRAVE;
		keymaps[0][0x33] = TK_BACKSLASH;
		keymaps[0][0x34] = TK_Z; // Fourth row
		keymaps[0][0x35] = TK_X;
		keymaps[0][0x36] = TK_C;
		keymaps[0][0x37] = TK_V;
		keymaps[0][0x38] = TK_B;
		keymaps[0][0x39] = TK_N;
		keymaps[0][0x3A] = TK_M;
		keymaps[0][0x3B] = TK_COMMA;
		keymaps[0][0x3C] = TK_PERIOD;
		keymaps[0][0x3D] = TK_SLASH;
		keymaps[0][0x5B] = TK_KP_PERIOD; // Extra
		keymaps[0][0x5E] = TK_BACKSLASH;
		keymaps[0][0x77] = TK_DELETE;

		keymaps[1][XK_BackSpace&0xFF] =    TK_BACKSPACE;   // 08 // Editing
		keymaps[1][XK_Tab&0xFF] =          TK_TAB;         // 09
		keymaps[1][XK_Return&0xFF] =       TK_RETURN;      // 0d
		keymaps[1][XK_Pause&0xFF] =        TK_PAUSE;       // 13
		keymaps[1][XK_Escape&0xFF] =       TK_ESCAPE;      // 1b
		keymaps[1][XK_Delete&0xFF] =       TK_DELETE;      // ff
		keymaps[1][XK_KP_0&0xFF] =         TK_KP_0;        // b0 // Keypad 0-9: NumLock on
		keymaps[1][XK_KP_1&0xFF] =         TK_KP_1;        // b1
		keymaps[1][XK_KP_2&0xFF] =         TK_KP_2;        // b2
		keymaps[1][XK_KP_3&0xFF] =         TK_KP_3;        // b3
		keymaps[1][XK_KP_4&0xFF] =         TK_KP_4;        // b4
		keymaps[1][XK_KP_5&0xFF] =         TK_KP_5;        // b5
		keymaps[1][XK_KP_6&0xFF] =         TK_KP_6;        // b6
		keymaps[1][XK_KP_7&0xFF] =         TK_KP_7;        // b7
		keymaps[1][XK_KP_8&0xFF] =         TK_KP_8;        // b8
		keymaps[1][XK_KP_9&0xFF] =         TK_KP_9;        // b9
		keymaps[1][XK_KP_Insert&0xFF] =    TK_KP_0;        // 9e // Keypad 0-9: NumLock off
		keymaps[1][XK_KP_End&0xFF] =       TK_KP_1;        // 9c
		keymaps[1][XK_KP_Down&0xFF] =      TK_KP_2;        // 99
		keymaps[1][XK_KP_Page_Down&0xFF] = TK_KP_3;        // 9b
		keymaps[1][XK_KP_Left&0xFF] =      TK_KP_4;        // 96
		keymaps[1][XK_KP_Begin&0xFF] =     TK_KP_5;        // 9d
		keymaps[1][XK_KP_Right&0xFF] =     TK_KP_6;        // 98
		keymaps[1][XK_KP_Home&0xFF] =      TK_KP_7;        // 95
		keymaps[1][XK_KP_Up&0xFF] =        TK_KP_8;        // 97
		keymaps[1][XK_KP_Page_Up&0xFF] =   TK_KP_9;        // 9a
		keymaps[1][XK_KP_Delete&0xFF] =    TK_KP_PERIOD;   // 9f // Keypad symbols
		keymaps[1][XK_KP_Decimal&0xFF] =   TK_KP_PERIOD;   // ae
		keymaps[1][XK_KP_Divide&0xFF] =    TK_KP_DIVIDE;   // af
		keymaps[1][XK_KP_Multiply&0xFF] =  TK_KP_MULTIPLY; // aa
		keymaps[1][XK_KP_Subtract&0xFF] =  TK_KP_MINUS;    // ad
		keymaps[1][XK_KP_Add&0xFF] =       TK_KP_PLUS;     // ab
		keymaps[1][XK_KP_Enter&0xFF] =     TK_KP_ENTER;    // 8d
		keymaps[1][XK_KP_Equal&0xFF] =     TK_EQUALS;      // bd
		keymaps[1][XK_Up&0xFF] =           TK_UP;          // 52 // Navigation
		keymaps[1][XK_Down&0xFF] =         TK_DOWN;        // 54
		keymaps[1][XK_Right&0xFF] =        TK_RIGHT;       // 53
		keymaps[1][XK_Left&0xFF] =         TK_LEFT;        // 51
		keymaps[1][XK_Insert&0xFF] =       TK_INSERT;      // 63
		keymaps[1][XK_Home&0xFF] =         TK_HOME;        // 50
		keymaps[1][XK_End&0xFF] =          TK_END;         // 57
		keymaps[1][XK_Page_Up&0xFF] =      TK_PAGEUP;      // 55
		keymaps[1][XK_Page_Down&0xFF] =    TK_PAGEDOWN;    // 56
		keymaps[1][XK_F1&0xFF] =           TK_F1;          // be // Functionals
		keymaps[1][XK_F2&0xFF] =           TK_F2;          // bf
		keymaps[1][XK_F3&0xFF] =           TK_F3;          // c0
		keymaps[1][XK_F4&0xFF] =           TK_F4;          // c1
		keymaps[1][XK_F5&0xFF] =           TK_F5;          // c2
		keymaps[1][XK_F6&0xFF] =           TK_F6;          // c3
		keymaps[1][XK_F7&0xFF] =           TK_F7;          // c4
		keymaps[1][XK_F8&0xFF] =           TK_F8;          // c5
		keymaps[1][XK_F9&0xFF] =           TK_F9;          // c6
		keymaps[1][XK_F10&0xFF] =          TK_F10;         // c7
		keymaps[1][XK_F11&0xFF] =          TK_F11;         // c8
		keymaps[1][XK_F12&0xFF] =          TK_F12;         // c9
		keymaps[1][XK_Shift_R&0xFF] =      TK_SHIFT;       // e2 // Modifiers
		keymaps[1][XK_Shift_L&0xFF] =      TK_SHIFT;       // e1
		keymaps[1][XK_Control_R&0xFF] =    TK_CONTROL;     // e4
		keymaps[1][XK_Control_L&0xFF] =    TK_CONTROL;     // e3
		keymaps[1][XK_Alt_R&0xFF] =        TK_ALT;         // ea
		keymaps[1][XK_Alt_L&0xFF] =        TK_ALT;         // e9
	}

	static XIMStyle ChooseBetterStyle(XIMStyle style1, XIMStyle style2)
	{
		XIMStyle s,t;
		XIMStyle preedit = XIMPreeditArea | XIMPreeditCallbacks | XIMPreeditPosition | XIMPreeditNothing | XIMPreeditNone;
		XIMStyle status = XIMStatusArea | XIMStatusCallbacks | XIMStatusNothing | XIMStatusNone;
		if (style1 == 0) return style2;
		if (style2 == 0) return style1;
		if ((style1 & (preedit | status)) == (style2 & (preedit | status))) return style1;
		s = style1 & preedit;
		t = style2 & preedit;
		if (s != t)
		{
			if (s | t | XIMPreeditCallbacks)
				return (s == XIMPreeditCallbacks)?style1:style2;
			else if (s | t | XIMPreeditPosition)
				return (s == XIMPreeditPosition)?style1:style2;
			else if (s | t | XIMPreeditArea)
				return (s == XIMPreeditArea)?style1:style2;
			else if (s | t | XIMPreeditNothing)
				return (s == XIMPreeditNothing)?style1:style2;
		}
		else
		{
			// if preedit flags are the same, compare status flags
			s = style1 & status;
			t = style2 & status;
			if (s | t | XIMStatusCallbacks)
				return (s == XIMStatusCallbacks)?style1:style2;
			else if (s | t | XIMStatusArea)
				return (s == XIMStatusArea)?style1:style2;
			else if (s | t | XIMStatusNothing)
				return (s == XIMStatusNothing)?style1:style2;
		}

		return 0;
	}

	// ------------------------------------------------------------------------

	X11Window::X11Window(EventHandler handler):
		Window(handler),
		m_last_mouse_click(0),
		m_consecutive_mouse_clicks(1),
		m_resizeable(false),
		m_client_resize(false),
		m_display(nullptr),
		m_screen(0),
		m_window(0),
		m_colormap(0),
		m_visual(nullptr),
		m_glx(nullptr),
		m_im(nullptr),
		m_ic(nullptr),
		//m_wm_close_message(),
		m_glXSwapIntervalEXT(nullptr),
		m_glXSwapIntervalMESA(nullptr),
		m_size_hints(XAllocSizeHints()),
		m_expose_timer(0)
	{
		try
		{
			Create();
		}
		catch (...)
		{
			Dispose();
			throw;
		}
	}

	void X11Window::Create()
	{
		XInitThreads();
		setlocale(LC_ALL, "");

		InitKeymaps();

		if ((m_display = XOpenDisplay(nullptr)) == nullptr)
			throw std::runtime_error("[X11] failed to open a display");
		m_screen = DefaultScreen(m_display);

		int glx_attrs[] =
		{
			GLX_RGBA,
			GLX_DOUBLEBUFFER,
			GLX_RED_SIZE, 4,
			GLX_GREEN_SIZE, 4,
			GLX_BLUE_SIZE, 4,
			GLX_DEPTH_SIZE, 16,
			None
		};
		if ((m_visual = glXChooseVisual(m_display, m_screen, glx_attrs)) == nullptr)
			throw std::runtime_error("[X11] failed to choose a double-buffered visual");

		// Log available OpenGL version
		int major, minor;
		glXQueryVersion(m_display, &major, &minor);
		LOG(Info, "Available OpenGL version: " << major << "." << minor);

		m_colormap = XCreateColormap
			(
				m_display,
				RootWindow(m_display, m_visual->screen),
				m_visual->visual,
				AllocNone
			);

		XSetWindowAttributes attrs;
		attrs.colormap = m_colormap;
		attrs.border_pixel = 0;
		attrs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask; // XXX: overriden later

		m_client_size = Size(640, 480);
		m_window = XCreateWindow
		(
			m_display,
			RootWindow(m_display, m_screen),
			0, 0,
			m_client_size.width, m_client_size.height,
			0,
			m_visual->depth,
			InputOutput,
			m_visual->visual,
			CWBorderPixel | CWColormap | CWEventMask,
			&attrs
		);
		if (m_window == 0)
			throw std::runtime_error("[X11] failed to create an XWindow");

		if ((m_glx = glXCreateContext(m_display, m_visual, 0, GL_TRUE)) == nullptr)
			throw std::runtime_error("[X11] failed to create GLX context");

		AcquireRC();
		ProbeOpenGL();

		// GLX-specific
		std::string extensions = glXQueryExtensionsString(m_display, m_screen);
		LOG(Trace, "OpenGL: " << extensions.c_str());
		if (extensions.find("GLX_EXT_swap_control") != std::string::npos)
		{
			LOG(Trace, "OpenGL context has GLX_EXT_swap_control extension");
			m_glXSwapIntervalEXT = (PFN_GLXSWAPINTERVALEXT)glXGetProcAddress((GLubyte*)"glXSwapIntervalEXT");
		}
		else if (extensions.find("GLX_MESA_swap_control") != std::string::npos)
		{
			LOG(Trace, "OpenGL context has GLX_MESA_swap_control extension");
			m_glXSwapIntervalMESA = (PFN_GLXSWAPINTERVALMESA)glXGetProcAddress((GLubyte*)"glXSwapIntervalMESA");
		}
		SetVSync(true);

		// Continue with input
		if ((m_im = XOpenIM(m_display, nullptr, nullptr, nullptr)) == nullptr)
			throw std::runtime_error("[X11] failed to open XIM");

		// Set flags for the styles an application can support
		XIMStyle app_supported_styles =
			XIMPreeditNone | XIMPreeditNothing | XIMPreeditArea |
			XIMStatusNone | XIMStatusNothing | XIMStatusArea;

		// Figure out which styles the IM can support
		XIMStyles* im_supported_styles = nullptr;
		XGetIMValues(m_im, XNQueryInputStyle, &im_supported_styles, NULL);

		// Look at each of the IM supported styles, and choise the "best" one that we can support
		XIMStyle best_style = 0;
		for (int i=0; i < im_supported_styles->count_styles; i++)
		{
			XIMStyle style = im_supported_styles->supported_styles[i];
			if ((style & app_supported_styles) == style) // if we can handle it
				best_style = ChooseBetterStyle(style, best_style);
		}
		XFree(im_supported_styles);

		if (best_style == 0)
			throw std::runtime_error("[X11] can't find supported IM interaction style");

		// Input context
		if ((m_ic = XCreateIC(m_im, XNInputStyle, best_style, XNClientWindow, m_window, nullptr)) == nullptr)
			throw std::runtime_error("[X11] Failed to create XIC input context");

		long im_event_mask = 0;
		XGetICValues(m_ic, XNFilterEvents, &im_event_mask, NULL);
		XSetICFocus(m_ic);

		long event_mask =
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			StructureNotifyMask |
			ButtonPressMask |
			ButtonReleaseMask |
			PointerMotionMask |
			im_event_mask;

		XSelectInput(m_display, m_window, event_mask);

		m_wm_close_message = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(m_display, m_window, &m_wm_close_message, 1);
		m_wm_state = XInternAtom(m_display, "_NET_WM_STATE", False);
		m_wm_name = XInternAtom(m_display, "_NET_WM_NAME", False);
		m_wm_maximized_horz = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		m_wm_maximized_vert = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

		Atom type = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE", False);
		Atom value = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
		XChangeProperty(
			m_display,
			m_window,
			type,
			XA_ATOM,
			32,
			PropModeReplace,
			reinterpret_cast<unsigned char*>(&value),
			1
		);
	}

	X11Window::~X11Window()
	{
		Dispose();
	}

	void X11Window::Dispose()
	{
		// FIXME: release XIC

		// FIXME: release XIM

		ReleaseRC();

		if (m_glx != nullptr)
			glXDestroyContext(m_display, m_glx);

		if (m_window != 0)
			XDestroyWindow(m_display, m_window);

		if (m_colormap != 0)
			XFreeColormap(m_display, m_colormap);

		if (m_visual != nullptr)
			XFree(m_visual);

		if (m_display != nullptr)
			XCloseDisplay(m_display);
	}

	void X11Window::SetIcon(const std::wstring& filename)
	{
		// TODO: Learn about icons in Linux
	}

	void X11Window::SetTitle(const std::wstring& title)
	{
		if (m_window == 0)
			return;

		std::string u8 = UTF8Encoding().Convert(title);
		XChangeProperty
		(
			m_display,
			m_window,
			m_wm_name,
			XInternAtom(m_display, "UTF8_STRING",  false), // XXX: cache?
			8,
			PropModeReplace,
			(const unsigned char*)u8.c_str(),
			u8.size()
		);
	}

	void X11Window::UpdateSizeHints(Size size) // FIXME: wtf
	{
		if (size.Area() == 0) size = m_client_size;

		auto hints = m_size_hints;

		if (m_resizeable)
		{
			hints->flags = PMinSize | PResizeInc;
			hints->width_inc = m_cell_size.width;
			hints->height_inc = m_cell_size.height;
			hints->min_width = m_minimum_size.width * m_cell_size.width;
			hints->min_height = m_minimum_size.height * m_cell_size.height;
		}
		else
		{
			hints->flags = PMinSize | PMaxSize;
			//hints->min_width = hints->max_width = m_client_size.width;
			//hints->min_height = hints->max_height = m_client_size.height;
			hints->min_width = hints->max_width = size.width;
			hints->min_height = hints->max_height = size.height;
		}

		XSetWMNormalHints(m_display, m_window, hints);
	}

	void X11Window::SetClientSize(const Size& size)
	{
		if (m_window == 0)
			return;

		if (m_fullscreen)
		{
			m_client_size = size;
		}
		else
		{
			Demaximize();
			UpdateSizeHints(size);
			XResizeWindow(m_display, m_window, size.width, size.height);
		}
	}

	void X11Window::SetResizeable(bool resizeable)
	{
		if (m_resizeable && !resizeable)
		{
			Demaximize();
		}

		m_resizeable = resizeable;
		UpdateSizeHints();
	}

	void X11Window::Demaximize()
	{
		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m_window;
		xev.xclient.message_type = m_wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 0; // FIXME: _NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = m_wm_maximized_horz;
		xev.xclient.data.l[2] = m_wm_maximized_vert;
		XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureNotifyMask, &xev);
	}

	void X11Window::SetFullscreen(bool fullscreen)
	{
		if (fullscreen == m_fullscreen)
			return;

		if (!m_resizeable)
		{
			XSizeHints *sizehints = XAllocSizeHints();
			long flags = 0;
			XGetWMNormalHints(m_display, m_window, sizehints, &flags);
			if (fullscreen)
			{
				// Entering fullscreen mode
				sizehints->flags &= ~(PMinSize | PMaxSize);
			}
			else
			{
				// Leaving fullscreen mode
				sizehints->flags |= PMinSize | PMaxSize;
				sizehints->min_width = sizehints->max_width = m_client_size.width;
				sizehints->min_height = sizehints->max_height = m_client_size.height;
			}

			XSetWMNormalHints(m_display, m_window, sizehints);
			XFree(sizehints);
		}

		XEvent e;
		memset(&e, 0, sizeof(e));
		e.xclient.type = ClientMessage;
		e.xclient.window = m_window;
		e.xclient.message_type = m_wm_state;
		e.xclient.format = 32;
		e.xclient.data.l[0] = fullscreen? 1: 0;
		e.xclient.data.l[1] = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);
		XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);

		m_event_handler(TK_INVALIDATE);

		m_fullscreen = fullscreen;
	}

	void X11Window::SetCursorVisibility(bool visible)
	{
		if (visible)
		{
			XUndefineCursor(m_display, m_window);
		}
		else
		{
			Cursor cursor;
			Pixmap dummy;
			XColor black;
			static char data[] = {0, 0, 0, 0, 0, 0, 0, 0};
			black.red = black.green = black.blue = 0;

			dummy = XCreateBitmapFromData(m_display, m_window, data, 8, 8);
			cursor = XCreatePixmapCursor(m_display, dummy, dummy, &black, &black, 0, 0);
			XDefineCursor(m_display, m_window, cursor);
			XFreeCursor(m_display, cursor);
			XFreePixmap(m_display, dummy);
		}
	}

	void X11Window::Show()
	{
		if (m_window != 0) // XXX: unnecessary check?
			XMapWindow(m_display, m_window);
	}

	void X11Window::Hide()
	{
		if (m_window != 0)
			XUnmapWindow(m_display, m_window);
	}

	Size X11Window::GetActualSize()
	{
		::Window root;
		int x, y;
		unsigned int width, height, border, depth;

		XGetGeometry
		(
			m_display,
			m_window,
			&root,
			&x, &y,
			&width, &height,
			&border,
			&depth
		);

		return Size(width, height);
	}

	int X11Window::TranslateKeycode(KeyCode kc)
	{
		if (kc >= 0 && kc <= 0xFF && m_keymaps[0][kc])
		{
			return m_keymaps[0][kc];
		}

		// Try to map keycode to some keyboard layout
		int count = 0;
		KeySym sym, *mapped = XGetKeyboardMapping(m_display, kc, 1, &count);
		if (count > 0 && mapped != nullptr && mapped[0] != NoSymbol)
		{
			sym = mapped[0];
		}
		XFree(mapped);

		if (sym == 0)
		{
			return 0;
		}

		int block = sym >> 8;

		if (block == 0xFF)
		{
			return m_keymaps[1][sym & 0xFF];
		}
		else if (block == 0 && sym == 32)
		{
			return TK_SPACE;
		}
		else
		{
			return 0;
		}
	}

	int X11Window::PumpEvents()
	{
		XEvent e;
		int processed = 0;

		while (XPending(m_display))
		{
			processed += 1;

			XNextEvent(m_display, &e);

			if (e.type == Expose && e.xexpose.count == 0)
			{
				if (!m_expose_timer)
					m_expose_timer = gettime();
			}
			else if (e.type == XlibKeyPress || e.type == XlibKeyRelease)
			{
				bool pressed = e.type == XlibKeyPress;

				if (!pressed && XPending(m_display))
				{
					XEvent next;
					XPeekEvent(m_display, &next);

					if (next.type == XlibKeyPress && next.xkey.keycode == e.xkey.keycode && next.xkey.time == e.xkey.time)
					{
						// Pseudo-release in auto-repeat mode, ignore.
						continue;
					}
				}

				int code = TranslateKeycode(e.xkey.keycode);
				if (code == 0)
				{
					continue;
				}

				wchar_t buffer[255] = {0};
				KeySym key;
				Status status;
				int rc = XwcLookupString(m_ic, &e.xkey, buffer, 255, &key, &status);

				if (rc <= 0)
				{
					buffer[0] = (wchar_t)0;
				}
				else if ((code >= TK_RETURN && code < TK_SPACE) || code >= TK_F1)
				{
					// These keys do not produce textual input
					buffer[0] = (wchar_t)0;
				}
				else if ((int)buffer[0] < 32)
				{
					// Ctrl+? keys, produce ASCII control codes
					buffer[0] = (wchar_t)0;
				}

				try
				{
					Event event(code|(pressed? 0: TK_KEY_RELEASED));
					event[code] = pressed? 1: 0;
					event[TK_WCHAR] = (int)buffer[0];
					m_event_handler(std::move(event));
				}
				catch (std::exception& e)
				{
					LOG(Warning, "Error processing keyboard event: " << e.what());
					continue;
				}
			}
			else if (e.type == ConfigureNotify)
			{
				// Ignore 'synthetic' events, they interfere with resizeable fullscreen operatrion.
				if (e.xconfigure.send_event)
					continue;

				Size new_size(e.xconfigure.width, e.xconfigure.height);
				if (new_size.width != m_client_size.width || new_size.height != m_client_size.height)
				{
					m_event_handler(TK_INVALIDATE);

					if (!m_fullscreen)
					{
						// X11 spams ConfigureNotify events with different sizes
						m_client_size = new_size;
						Event event(TK_RESIZED);
						event[TK_WIDTH] = new_size.width;
						event[TK_HEIGHT] = new_size.height;
						m_event_handler(event);
					}

					//HandleRepaint();
					m_event_handler(TK_INVALIDATE); // XXX: EXPOSE
				}
			}
			else if (e.type == MotionNotify)
			{
				// OnMouseMove
				Event event(TK_MOUSE_MOVE);
				event[TK_MOUSE_PIXEL_X] = e.xmotion.x;
				event[TK_MOUSE_PIXEL_Y] = e.xmotion.y;
				m_event_handler(std::move(event));
			}
			else if (e.type == ButtonPress || e.type == ButtonRelease)
			{
				// OnMousePress/Release
				bool pressed = e.type == ButtonPress;

				bool is_button =
					(e.xbutton.button >= 1 && e.xbutton.button <= 3) ||
					(e.xbutton.button >= 6 && e.xbutton.button <= 7);

				if (is_button && pressed)
				{
					uint64_t now = gettime();
					uint64_t delta = now - m_last_mouse_click;
					m_last_mouse_click = now;

					if (delta < 250000) // FIXME: harcode, make an input.mouse-click-speed option.
					{
						m_consecutive_mouse_clicks += 1;
					}
					else
					{
						m_consecutive_mouse_clicks = 1;
					}
				}

				if (is_button)
				{
					static int mapping[] =
					{
						0,               // None
						TK_MOUSE_LEFT,   // LMB
						TK_MOUSE_MIDDLE, // MMB
						TK_MOUSE_RIGHT,  // RMB
						0,               // Wheel up
						0,               // Wheel down
						TK_MOUSE_X1,     // X1
						TK_MOUSE_X2      // X2
					};

					int code = mapping[e.xbutton.button];
					Event event(code | (pressed? 0: TK_KEY_RELEASED));
					event[code] = pressed? 1: 0;
					event[TK_MOUSE_CLICKS] = pressed? m_consecutive_mouse_clicks: 0;
					m_event_handler(event);
				}
				else if (e.xbutton.button == 4 && e.type == ButtonPress)
				{
					m_event_handler(Event(TK_MOUSE_SCROLL, {{TK_MOUSE_WHEEL, -1}}));
				}
				else if (e.xbutton.button == 5 && e.type == ButtonPress)
				{
					m_event_handler(Event(TK_MOUSE_SCROLL, {{TK_MOUSE_WHEEL, +1}}));
				}
				else
				{
					// Not supported
					continue;
				}
			}
			else if (e.type == ClientMessage && e.xclient.data.l[0] == (long)m_wm_close_message)
			{
				m_event_handler(TK_CLOSE);
			}
		}

		if (m_expose_timer > 0 && (gettime() - m_expose_timer) > (1000/30))
		{
			m_event_handler(TK_REDRAW);
			m_expose_timer = 0;
		}

		return processed;
	}

	void X11Window::AcquireRC()
	{
		glXMakeCurrent(m_display, m_window, m_glx);
	}

	void X11Window::ReleaseRC()
	{
		glXMakeCurrent(m_display, None, nullptr);
	}

	void X11Window::SwapBuffers()
	{
		glXSwapBuffers(m_display, m_window);
	}

	void X11Window::SetVSync(bool enabled)
	{
		int interval = enabled? 1: 0;
		if (m_glXSwapIntervalEXT)
		{
			m_glXSwapIntervalEXT(m_display, m_window, interval);
		}
		else if (m_glXSwapIntervalMESA)
		{
			m_glXSwapIntervalMESA(interval);
		}
	}
}

#endif

