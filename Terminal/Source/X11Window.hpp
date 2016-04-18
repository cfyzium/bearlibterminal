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

#ifndef BEARLIBTERMINAL_X11WINDOW3_HPP
#define BEARLIBTERMINAL_X11WINDOW3_HPP

#ifdef __linux__

#include <memory>
#include "Window.hpp"
#include "Point.hpp"
#include <X11/Xlib.h>
#include <X11/Xmu/Xmu.h>
#include <GL/glx.h>

namespace BearLibTerminal
{
	class X11Window: public Window
	{
	public:
		X11Window(EventHandler handler);
		~X11Window();
		void SetTitle(const std::wstring& title);
		void SetIcon(const std::wstring& filename);
		void SetClientSize(const Size& size);
		void Show();
		void Hide();
		void AcquireRC();
		void ReleaseRC();
		void SwapBuffers();
		void SetVSync(bool enabled);
		int PumpEvents();
		void SetResizeable(bool resizeable);
		Size GetActualSize();
		void SetFullscreen(bool fullscreen);
		void SetCursorVisibility(bool visible);
	protected:
		void Create();
		void Dispose();
		void UpdateSizeHints(Size size=Size());
		void Demaximize();
		void InitKeymaps();
		int TranslateKeycode(KeyCode kc);
	protected:
		uint64_t m_last_mouse_click;
		int m_consecutive_mouse_clicks;
		bool m_resizeable;
		bool m_client_resize;

	private:
		typedef void (*PFN_GLXSWAPINTERVALEXT)(Display *dpy, GLXDrawable drawable, int interval);
		typedef int (*PFN_GLXSWAPINTERVALMESA)(int interval);

		Display* m_display;
		int m_screen;
		::Window m_window;
		Colormap m_colormap;
		XVisualInfo* m_visual;
		GLXContext m_glx;
		XIM m_im;
		XIC m_ic;
		Atom m_wm_close_message;
		Atom m_wm_invoke_message;
		Atom m_wm_state;
		Atom m_wm_name;
		Atom m_wm_maximized_horz;
		Atom m_wm_maximized_vert;
		XSizeHints* m_size_hints;
		int m_keymaps[2][256]; // TODO: static
		PFN_GLXSWAPINTERVALEXT m_glXSwapIntervalEXT;
		PFN_GLXSWAPINTERVALMESA m_glXSwapIntervalMESA;
		uint64_t m_expose_timer;
	};
}

#endif

#endif // BEARLIBTERMINAL_X11WINDOW3_HPP
