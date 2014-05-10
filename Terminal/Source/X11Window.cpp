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

#ifdef __linux__

#include "X11Window.hpp"
#include "OpenGL.hpp"
#include "Log.hpp"
#include "Encoding.hpp"
#include <X11/Xlib.h>
#include <X11/Xmu/Xmu.h>
#include <GL/glx.h>
#include <unistd.h>
#include <sys/time.h>
#include "BearLibTerminal.h"
#include <future>

// X11 hack because X.h #defines these names without any regards to others
#define XlibKeyPress 2
#define XlibKeyRelease 3
#undef KeyPress
#undef KeyRelease

namespace BearLibTerminal
{
	struct X11Window::Private
	{
		Private();
		~Private();

		Display* display;
		::Window window;
		int screen;
		XVisualInfo* visual;
		GLXContext glx;
		XIM im;
		XIC ic;
		Atom close_message;
		Atom invoke_message;
		XSizeHints* size_hints;

		typedef void (*PFN_GLXSWAPINTERVALEXT)(Display *dpy, GLXDrawable drawable, int interval);
		typedef int (*PFN_GLXSWAPINTERVALMESA)(int interval);
		PFN_GLXSWAPINTERVALEXT glXSwapIntervalEXT;
		PFN_GLXSWAPINTERVALMESA glXSwapIntervalMESA;
	};

	struct InvokationSentry2
	{
		std::packaged_task<void()> task;

		InvokationSentry2(std::function<void()> func):
			task(func)
		{ }
	};

	X11Window::Private::Private():
		display(NULL),
		window(0),
		screen(0),
		visual(NULL),
		glx(NULL),
		im(NULL),
		ic(NULL),
		close_message(),
		glXSwapIntervalEXT(nullptr),
		glXSwapIntervalMESA(nullptr),
		size_hints(nullptr)
	{
		size_hints = XAllocSizeHints();
	}

	X11Window::Private::~Private()
	{
		if (size_hints) XFree(size_hints);
	}

	// XXX: Salvaged from somewhere, format it
	// ------------------------------------------------------------------------

	// attributes for a single buffered visual in RGBA format with at least
	// 4 bits per color and a 16 bit depth buffer
	static int glx_single_buffered_attrs[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 4,
		GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4,
		GLX_DEPTH_SIZE, 16,
		None
	};

	// attributes for a double buffered visual in RGBA format with at least
	// 4 bits per color and a 16 bit depth buffer
	static int glx_double_buffered_attrs[] =
	{
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 4,
		GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4,
		GLX_DEPTH_SIZE, 16,
		None
	};

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

	X11Window::X11Window():
		m_private(new Private()),
		m_mouse_wheel(0),
		m_resizeable(false)
	{ }

	X11Window::~X11Window()
	{
		LOG(Trace, "~X11Window");
		Stop();
		LOG(Trace, "Done with window object (X11)");
	}

	bool X11Window::ValidateIcon(const std::wstring& filename)
	{
		// TODO: Learn about icons in Linux
		return true;
	}

	void X11Window::SetIcon(const std::wstring& filename)
	{
		// TODO: Learn about icons in Linux
	}

	void X11Window::SetTitle(const std::wstring& title)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if ( m_private->window == 0 ) return;
		std::string u8 = UTF8->Convert(title);
		XChangeProperty
		(
			m_private->display,
			m_private->window,
			XInternAtom(m_private->display, "_NET_WM_NAME", false),
			XInternAtom(m_private->display, "UTF8_STRING",  false),
			8,
			PropModeReplace,
			(const unsigned char*)u8.c_str(),
			u8.size()
		);
	}

	void X11Window::UpdateSizeHints()
	{
		auto hints = m_private->size_hints;

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
			hints->min_width = hints->max_width = m_client_size.width;
			hints->min_height = hints->max_height = m_client_size.height;
		}

		XSetWMNormalHints(m_private->display, m_private->window, hints);
	}

	void X11Window::SetClientSize(const Size& size)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if ( m_private->window == 0 ) return;
		m_client_size = size;
		UpdateSizeHints();
		XResizeWindow(m_private->display, m_private->window, size.width, size.height);
	}

	void X11Window::SetResizeable(bool resizeable)
	{
		if (m_resizeable && !resizeable)
		{
			// Try to demaximize just in case
			XEvent xev;
			memset(&xev, 0, sizeof(xev));
			Atom wm_state  =  XInternAtom(m_private->display, "_NET_WM_STATE", False);
			Atom max_horz  =  XInternAtom(m_private->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
			Atom max_vert  =  XInternAtom(m_private->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

			memset(&xev, 0, sizeof(xev));
			xev.type = ClientMessage;
			xev.xclient.window = m_private->window;
			xev.xclient.message_type = wm_state;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 0;//_NET_WM_STATE_REMOVE;
			xev.xclient.data.l[1] = max_horz;
			xev.xclient.data.l[2] = max_vert;

			Status st = XSendEvent(m_private->display, DefaultRootWindow(m_private->display), False, SubstructureNotifyMask, &xev);
		}

		m_resizeable = resizeable;
		UpdateSizeHints();
	}

	void X11Window::Redraw()
	{
		std::unique_lock<std::mutex> guard(m_lock);

		int retries = 5;
		m_redraw_barrier.SetValue(0);
		do
		{
			XClearArea
			(
				m_private->display,
				m_private->window,
				0,
				0,
				m_client_size.width,
				m_client_size.height,
				True
			);

			guard.unlock();
			if (m_redraw_barrier.WaitFor(50))
			{
				break;
			}
			guard.lock();
		}
		while (retries --> 0);
	}

	void X11Window::Show()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if ( m_private->window != 0 ) XMapWindow(m_private->display, m_private->window);
	}

	void X11Window::Hide()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if ( m_private->window != 0 ) XUnmapWindow(m_private->display, m_private->window);
	}

	void X11Window::HandleRepaint()
	{
		int rc = 0;

		try
		{
			if (m_on_redraw && m_on_redraw() > 0) SwapBuffers();
		}
		catch (std::exception& e)
		{
			LOG(Fatal, L"Rendering routine has thrown an exception: " << e.what());
			m_proceed = false;
		}

		// Open barrier
		m_redraw_barrier.NotifyAtMost(1);
	}

	static int DEF_keymap[256];
	static int ODD_keymap[256];
	static int MISC_keymap[256];

	void X11_InitKeymap(void)
	{
		for (auto& i: ODD_keymap) i = 0;
		for (auto& i: MISC_keymap) i = 0;

		// These X keysyms have 0xFF as the high byte
		MISC_keymap[XK_BackSpace&0xFF] = TK_BACKSPACE;
		MISC_keymap[XK_Tab&0xFF] = TK_TAB;
		//MISC_keymap[XK_Clear&0xFF] = VK_CLEAR; // TODO
		MISC_keymap[XK_Return&0xFF] = TK_RETURN;
		MISC_keymap[XK_Pause&0xFF] = TK_PAUSE;
		MISC_keymap[XK_Escape&0xFF] = TK_ESCAPE;
		MISC_keymap[XK_Delete&0xFF] = TK_DELETE;

		MISC_keymap[XK_KP_0&0xFF] = TK_NUMPAD0;		// Keypad 0-9
		MISC_keymap[XK_KP_1&0xFF] = TK_NUMPAD1;
		MISC_keymap[XK_KP_2&0xFF] = TK_NUMPAD2;
		MISC_keymap[XK_KP_3&0xFF] = TK_NUMPAD3;
		MISC_keymap[XK_KP_4&0xFF] = TK_NUMPAD4;
		MISC_keymap[XK_KP_5&0xFF] = TK_NUMPAD5;
		MISC_keymap[XK_KP_6&0xFF] = TK_NUMPAD6;
		MISC_keymap[XK_KP_7&0xFF] = TK_NUMPAD7;
		MISC_keymap[XK_KP_8&0xFF] = TK_NUMPAD8;
		MISC_keymap[XK_KP_9&0xFF] = TK_NUMPAD9;
		MISC_keymap[XK_KP_Insert&0xFF] = TK_NUMPAD0;
		MISC_keymap[XK_KP_End&0xFF] = TK_NUMPAD1;
		MISC_keymap[XK_KP_Down&0xFF] = TK_NUMPAD2;
		MISC_keymap[XK_KP_Page_Down&0xFF] = TK_NUMPAD3;
		MISC_keymap[XK_KP_Left&0xFF] = TK_NUMPAD4;
		MISC_keymap[XK_KP_Begin&0xFF] = TK_NUMPAD5;
		MISC_keymap[XK_KP_Right&0xFF] = TK_NUMPAD6;
		MISC_keymap[XK_KP_Home&0xFF] = TK_NUMPAD7;
		MISC_keymap[XK_KP_Up&0xFF] = TK_NUMPAD8;
		MISC_keymap[XK_KP_Page_Up&0xFF] = TK_NUMPAD9;
		MISC_keymap[XK_KP_Delete&0xFF] = TK_DECIMAL;//SDLK_KP_PERIOD;
		MISC_keymap[XK_KP_Decimal&0xFF] = TK_DECIMAL;//SDLK_KP_PERIOD;
		MISC_keymap[XK_KP_Divide&0xFF] = TK_DIVIDE;//SDLK_KP_DIVIDE;
		MISC_keymap[XK_KP_Multiply&0xFF] = TK_MULTIPLY;//SDLK_KP_MULTIPLY;
		MISC_keymap[XK_KP_Subtract&0xFF] = TK_SUBTRACT;//SDLK_KP_MINUS;
		MISC_keymap[XK_KP_Add&0xFF] = TK_ADD;//SDLK_KP_PLUS;
		MISC_keymap[XK_KP_Enter&0xFF] = TK_RETURN;//SDLK_KP_ENTER;
		MISC_keymap[XK_KP_Equal&0xFF] = TK_EQUALS;//SDLK_KP_EQUALS;

		MISC_keymap[XK_Up&0xFF] = TK_UP;
		MISC_keymap[XK_Down&0xFF] = TK_DOWN;
		MISC_keymap[XK_Right&0xFF] = TK_RIGHT;
		MISC_keymap[XK_Left&0xFF] = TK_LEFT;
		MISC_keymap[XK_Insert&0xFF] = TK_INSERT;
		MISC_keymap[XK_Home&0xFF] = TK_HOME;
		MISC_keymap[XK_End&0xFF] = TK_END;
		MISC_keymap[XK_Page_Up&0xFF] = TK_PRIOR;//SDLK_PAGEUP;
		MISC_keymap[XK_Page_Down&0xFF] = TK_NEXT;//SDLK_PAGEDOWN;

		MISC_keymap[XK_F1&0xFF] = TK_F1;
		MISC_keymap[XK_F2&0xFF] = TK_F2;
		MISC_keymap[XK_F3&0xFF] = TK_F3;
		MISC_keymap[XK_F4&0xFF] = TK_F4;
		MISC_keymap[XK_F5&0xFF] = TK_F5;
		MISC_keymap[XK_F6&0xFF] = TK_F6;
		MISC_keymap[XK_F7&0xFF] = TK_F7;
		MISC_keymap[XK_F8&0xFF] = TK_F8;
		MISC_keymap[XK_F9&0xFF] = TK_F9;
		MISC_keymap[XK_F10&0xFF] = TK_F10;
		MISC_keymap[XK_F11&0xFF] = TK_F11;
		MISC_keymap[XK_F12&0xFF] = TK_F12;
		MISC_keymap[XK_Shift_R&0xFF] = TK_SHIFT;//SDLK_RSHIFT;
		MISC_keymap[XK_Shift_L&0xFF] = TK_SHIFT;//SDLK_LSHIFT;
		MISC_keymap[XK_Control_R&0xFF] = TK_CONTROL;//SDLK_RCTRL;
		MISC_keymap[XK_Control_L&0xFF] = TK_CONTROL;//SDLK_LCTRL;

		memset(DEF_keymap, 0, sizeof(DEF_keymap));
		DEF_keymap[0x3C] = TK_PERIOD;
		DEF_keymap[0x77] = TK_DELETE;
		DEF_keymap[0x5B] = TK_DECIMAL;
		DEF_keymap[0x3B] = TK_COMMA;
		DEF_keymap[0x31] = TK_GRAVE;
		DEF_keymap[0x14] = TK_MINUS;
		DEF_keymap[0x15] = TK_EQUALS;
		DEF_keymap[0x5E] = TK_BACKSLASH;
		DEF_keymap[0x22] = TK_LBRACKET;
		DEF_keymap[0x23] = TK_RBRACKET;
		DEF_keymap[0x3D] = TK_SLASH;
		DEF_keymap[0x33] = TK_BACKSLASH;
		DEF_keymap[0x30] = TK_APOSTROPHE;
		DEF_keymap[0x2F] = TK_SEMICOLON;
		DEF_keymap[0x26] = TK_A;
		DEF_keymap[0x38] = TK_B;
		DEF_keymap[0x36] = TK_C;
		DEF_keymap[0x28] = TK_D;
		DEF_keymap[0x1A] = TK_E;
		DEF_keymap[0x29] = TK_F;
		DEF_keymap[0x2A] = TK_G;
		DEF_keymap[0x2B] = TK_H;
		DEF_keymap[0x1F] = TK_I;
		DEF_keymap[0x2C] = TK_J;
		DEF_keymap[0x2D] = TK_K;
		DEF_keymap[0x2E] = TK_L;
		DEF_keymap[0x3A] = TK_M;
		DEF_keymap[0x39] = TK_N;
		DEF_keymap[0x20] = TK_O;
		DEF_keymap[0x21] = TK_P;
		DEF_keymap[0x18] = TK_Q;
		DEF_keymap[0x1B] = TK_R;
		DEF_keymap[0x27] = TK_S;
		DEF_keymap[0x1C] = TK_T;
		DEF_keymap[0x1E] = TK_U;
		DEF_keymap[0x37] = TK_V;
		DEF_keymap[0x19] = TK_W;
		DEF_keymap[0x35] = TK_X;
		DEF_keymap[0x1D] = TK_Y;
		DEF_keymap[0x34] = TK_Z;
	}

	int X11_TranslateKeycode(Display *display, KeyCode kc)
	{
		if (kc >= 0 && kc <= 0xFF && DEF_keymap[kc])
		{
			return DEF_keymap[kc];
		}

		int key = 0;

		KeySym xsym = XKeycodeToKeysym(display, kc, 0);
		if (xsym)
		{
			switch (xsym>>8)
			{
				case 0x1005FF:
					break;
				case 0x00:	// Latin 1
					key = (int)(xsym & 0xFF);
					break;
				case 0x01:	// Latin 2
				case 0x02:	// Latin 3
				case 0x03:	// Latin 4
				case 0x04:	// Katakana
				case 0x05:	// Arabic
				case 0x06:	// Cyrillic
				case 0x07:	// Greek
				case 0x08:	// Technical
				case 0x0A:	// Publishing
				case 0x0C:	// Hebrew
				case 0x0D:	// Thai
					// These are wrong, but it's better than nothing
					key = (int)(xsym & 0xFF);
					break;
				case 0xFE:
					key = ODD_keymap[xsym&0xFF];
					break;
				case 0xFF:
					key = MISC_keymap[xsym&0xFF];
					break;
				default:
				break;
			}
		}
		return key;
	}

	std::future<void> X11Window::Post(std::function<void()> func)
	{
		auto sentry = std::make_shared<InvokationSentry2>(func);
		std::future<void> future = sentry->task.get_future();

		XClientMessageEvent event;
		memset(&event, 0, sizeof(XClientMessageEvent));
		event.type = ClientMessage;
		event.window = m_private->window;
		event.format = 32;

		// XXX: bitness-agnostic event marking hack
		event.data.l[0] = (long)m_private->invoke_message;

		uint64_t p = (uint64_t)&sentry;
		event.data.l[1] = p & 0xFFFFFFFF; // low
		event.data.l[2] = (p >> 32) & 0xFFFFFFFF; // high

		XSendEvent(m_private->display, m_private->window, 0, 0, (XEvent*)&event);
		XFlush(m_private->display);

		return std::move(future);
	}

	bool X11Window::PumpEvents()
	{
		int events_processed = 0;
		XEvent e;

		while (XPending(m_private->display))
		{
			XNextEvent(m_private->display, &e);
			events_processed += 1;

			if (e.type == Expose && e.xexpose.count == 0)
			{
				HandleRepaint();
			}
			else if (e.type == XlibKeyPress || e.type == XlibKeyRelease)
			{
				wchar_t buffer[255] = {0};
				KeySym key;
				Status status;

				int rc = XwcLookupString(m_private->ic, &e.xkey, buffer, 255, &key, &status);

				unsigned int keycode = e.xkey.keycode;
				int code = X11_TranslateKeycode(m_private->display, keycode);

                std::uint32_t mask = (e.type == XlibKeyPress)? Keystroke::KeyPress: Keystroke::KeyRelease;
				Keystroke stroke(mask, code, buffer[0]);
				if (code != TK_SPACE && ((code >= TK_LBUTTON && code <= TK_DELETE) || rc == 0))
				{
					stroke.character = 0;
				}
                else if (mask == Keystroke::KeyPress && buffer[0] > 0)
                {
                    stroke.type |= Keystroke::Unicode;
                }

				std::lock_guard<std::mutex> guard(m_lock);
				if (m_on_input) m_on_input(stroke);
			}
			else if (e.type == ConfigureNotify)
			{
				// OnResize
				Size new_size(e.xconfigure.width, e.xconfigure.height);
				if (new_size.width != m_client_size.width || new_size.height != m_client_size.height)
				{
					m_client_size = new_size;
					if (m_on_input)
					{
						Keystroke stroke(Keystroke::KeyPress, TK_WINDOW_RESIZE, m_client_size.width, m_client_size.height, 0);
						m_on_input(stroke);
					}
				}
			}
			else if (e.type == MotionNotify)
			{
				// OnMouseMove
				m_mouse_position.x = e.xmotion.x;
				m_mouse_position.y = e.xmotion.y;

				Keystroke stroke(Keystroke::MouseMove, TK_MOUSE_MOVE, m_mouse_position.x, m_mouse_position.y, 0);
				ReportInput(stroke);
			}
			else if (e.type == ButtonPress || e.type == ButtonRelease)
			{
				// OnMousePress/Release
				Keystroke stroke(e.type == ButtonPress? Keystroke::KeyPress: Keystroke::KeyRelease, 0);
				if (e.xbutton.button == 1)
				{
					// LMB
					stroke.scancode = TK_LBUTTON;
				}
				else if (e.xbutton.button == 3)
				{
					// RMB
					stroke.scancode = TK_RBUTTON;
				}
				else if (e.xbutton.button == 4 && e.type == ButtonPress)
				{
					m_mouse_wheel -= 1;
					stroke.scancode = TK_MOUSE_SCROLL;
				}
				else if (e.xbutton.button == 5 && e.type == ButtonPress)
				{
					m_mouse_wheel += 1;
					stroke.scancode = TK_MOUSE_SCROLL;
				}
				else
				{
					// Not supported
					continue;
				}
				stroke.x = m_mouse_position.x;
				stroke.y = m_mouse_position.y;
				stroke.z = m_mouse_wheel;
				ReportInput(stroke);
			}
			else if (e.type == ClientMessage && e.xclient.format == 32 && e.xclient.data.l[0] == (long)m_private->invoke_message)
			{
				uint64_t low = e.xclient.data.l[1] & 0xFFFFFFFF;
				uint64_t high = e.xclient.data.l[2] & 0xFFFFFFFF;
				uint64_t p = (high << 32) | low;
				auto sentry = *(std::shared_ptr<InvokationSentry2>*)p;
				sentry->task();
			}
			else if (e.type == ClientMessage && e.xclient.data.l[0] == (long)m_private->close_message)
			{
				Keystroke stroke(Keystroke::KeyPress, TK_CLOSE);
				ReportInput(stroke);
			}
		}

		return events_processed > 0;
	}

	void X11Window::ThreadFunction()
	{
		LOG(Trace, "Entering X11-specific window thread function");

		while (m_proceed)
		{
			if (!PumpEvents()) usleep(500);
		}

		// Notify possible pending refreshes (TODO: duplicate? there is one in Destroy too)
		m_redraw_barrier.Notify();

		LOG(Trace, "Leaving X11-specific window thread function");
	}

	bool X11Window::Construct()
	{
		std::lock_guard<std::mutex> guard(m_lock);

		if (!CreateWindowObject())
		{
			DestroyUnlocked();
			return false;
		}

		m_proceed = true;
		return true;
	}

	void X11Window::DestroyUnlocked()
	{
		DestroyWindowObject();
	}

	void X11Window::Destroy()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		DestroyUnlocked();

		// Unblock possibly waiting client thread
		m_redraw_barrier.Notify();
	}

	bool X11Window::CreateWindowObject()
	{
		XInitThreads();
		setlocale(LC_ALL, "");

		m_private->display = XOpenDisplay(NULL);
		if ( m_private->display == NULL )
		{
			// TODO: LOG
			return false;
		}

		m_private->screen = DefaultScreen(m_private->display);

		m_private->visual = glXChooseVisual(m_private->display, m_private->screen, glx_double_buffered_attrs);

		if ( m_private->visual == NULL )
		{
			// TODO: LOG
			// Try fall back to single buffered (WTF?)
			m_private->visual = glXChooseVisual(m_private->display, m_private->screen, glx_single_buffered_attrs);
		}

		if ( m_private->visual == NULL )
		{
			// TODO: LOG
			return false;
		}

		// Log available OpenGL version
		int major, minor;
		glXQueryVersion(m_private->display, &major, &minor);
		LOG(Info, "Available OpenGL version: " << major << "." << minor);

		Colormap colormap = XCreateColormap
		(
			m_private->display,
			RootWindow(m_private->display, m_private->visual->screen),
			m_private->visual->visual,
			AllocNone
		);

		XSetWindowAttributes attrs;
		attrs.colormap = colormap;
		attrs.border_pixel = 0;
		attrs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask; // XXX: overriden later

		m_client_size = Size(640, 480);
		m_private->window = XCreateWindow
		(
			m_private->display,
			RootWindow(m_private->display, m_private->screen),
			0, 0,
			m_client_size.width, m_client_size.height,
			0,
			m_private->visual->depth,
			InputOutput,
			m_private->visual->visual,
			CWBorderPixel | CWColormap | CWEventMask,
			&attrs
		);

		// Continue with GL
		m_private->glx = glXCreateContext(m_private->display, m_private->visual, 0, GL_TRUE);
		AcquireRC();
		ProbeOpenGL();

		// GLX-specific
		std::string extensions = glXQueryExtensionsString(m_private->display, m_private->screen);
		LOG(Trace, "OpenGL: " << extensions.c_str());
		if (extensions.find("GLX_EXT_swap_control") != std::string::npos)
		{
			LOG(Trace, "OpenGL context has GLX_EXT_swap_control extension");
			m_private->glXSwapIntervalEXT = (Private::PFN_GLXSWAPINTERVALEXT)glXGetProcAddressARB((GLubyte*)"glXSwapIntervalEXT");
		}
		else if (extensions.find("GLX_MESA_swap_control") != std::string::npos)
		{
			LOG(Trace, "OpenGL context has GLX_MESA_swap_control extension");
			m_private->glXSwapIntervalMESA = (Private::PFN_GLXSWAPINTERVALMESA)glXGetProcAddressARB((GLubyte*)"glXSwapIntervalMESA");
		}
		SetVSync(true);

		// Continue with input
		if ( (m_private->im = XOpenIM(m_private->display, NULL, NULL, NULL)) == NULL )
		{
			LOG(Fatal, "Failed to open IM");
			return false;
		}

		XIMStyles *im_supported_styles;
		XIMStyle app_supported_styles;
		XIMStyle style;
		XIMStyle best_style;

		// Set flags for the styles an application can support
		app_supported_styles =
			XIMPreeditNone | XIMPreeditNothing | XIMPreeditArea |
			XIMStatusNone | XIMStatusNothing | XIMStatusArea;

		// Figure out which styles the IM can support
		XGetIMValues(m_private->im, XNQueryInputStyle, &im_supported_styles, NULL);

		// Look at each of the IM supported styles, and choise the "best" one that we can support
		best_style = 0;
		for( int i=0; i < im_supported_styles->count_styles; i++ )
		{
			style = im_supported_styles->supported_styles[i];
			if ( (style & app_supported_styles) == style ) // if we can handle it
			{
				best_style = ChooseBetterStyle(style, best_style);
			}
		}
		if (best_style == 0)
		{
			LOG(Fatal, "Can't find supported IM interaction style");
			return false;
		}

		XFree(im_supported_styles);

		// Input context
		m_private->ic = XCreateIC(m_private->im, XNInputStyle, best_style, XNClientWindow, m_private->window, NULL);
		if ( m_private->ic == NULL )
		{
			LOG(Fatal, "Failed to create input context");
			return false;
		}

		long im_event_mask;
		XGetICValues(m_private->ic, XNFilterEvents, &im_event_mask, NULL);
		XSetICFocus(m_private->ic);

		long event_mask =
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			StructureNotifyMask |
			ButtonPressMask |
			ButtonReleaseMask |
			PointerMotionMask |
			im_event_mask;
		XSelectInput(m_private->display, m_private->window, event_mask);

		m_private->close_message = XInternAtom(m_private->display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(m_private->display, m_private->window, &m_private->close_message, 1);

		m_private->invoke_message = XInternAtom(m_private->display, "WM_CUSTOM_INVOKE", False);

		X11_InitKeymap();

		return true;
	}

	void X11Window::DestroyWindowObject()
	{
		// TODO: Release IC

		// TODO: Release IM

		//glXMakeCurrent(m_private->display, None, NULL);
		ReleaseRC();
		glXDestroyContext(m_private->display, m_private->glx);
		XDestroyWindow(m_private->display, m_private->window);

		if ( m_private->display != NULL )
		{
			XCloseDisplay(m_private->display);
			m_private->display = NULL;
		}
	}

	bool X11Window::AcquireRC()
	{
		return glXMakeCurrent(m_private->display, m_private->window, m_private->glx);
	}

	bool X11Window::ReleaseRC()
	{
		glXMakeCurrent(m_private->display, None, NULL);
		return true;
	}

	void X11Window::SwapBuffers()
	{
		glXSwapBuffers(m_private->display, m_private->window);
	}

	void X11Window::SetVSync(bool enabled)
	{
		int interval = enabled? 1: 0;
		if (m_private->glXSwapIntervalEXT)
		{
			m_private->glXSwapIntervalEXT(m_private->display, m_private->window, interval);
		}
		else if (m_private->glXSwapIntervalMESA)
		{
			m_private->glXSwapIntervalMESA(interval);
		}
	}

	void X11Window::ReportInput(const Keystroke& keystroke)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (m_on_input) m_on_input(keystroke);
	}
}

#endif

