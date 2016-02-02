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
#include "Utility.hpp"

namespace BearLibTerminal
{
	Window::Window(EventHandler handler):
		m_event_handler(handler),
		m_minimum_size(1, 1),
		m_fullscreen(false),
		m_resizeable(false)
	{ }

	Window::~Window()
	{ }

	void Window::SetSizeHints(Size increment, Size minimum_size)
	{
		m_cell_size = increment;
		m_minimum_size = minimum_size;

		if (m_minimum_size.width < 1) m_minimum_size.width = 1;
		if (m_minimum_size.height < 1) m_minimum_size.height = 1;
	}

	void Window::ToggleFullscreen()
	{ }

	bool Window::IsFullscreen() const
	{
		return m_fullscreen;
	}

	std::unique_ptr<Window> Window::Create(EventHandler handler)
	{
#if defined(__linux)
		return std::make_unique<X11Window>(handler);
#endif
#if defined(_WIN32)
		return std::make_unique<WinApiWindow>(handler);
#endif
	}
}
