/*
* BearLibTerminal
* Copyright (C) 2013-2021 Cfyz
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

#include "SDL2Window.hpp"
#include "Encoding.hpp"
#include "Log.hpp"
#include "BearLibTerminal.h"
#include <unordered_map>
#include <iostream>
#include <deque>
#include <chrono>

namespace BearLibTerminal {

SDL2Window::SDL2Window(EventHandler handler):
	Window(handler),
	m_Window(nullptr),
	m_GL_Context(nullptr)
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

SDL2Window::~SDL2Window()
{
	Dispose();
}

void SDL2Window::Create()
{
	SDL_Init(SDL_INIT_VIDEO); // XXX: make it global?

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_Window = SDL_CreateWindow(
		"BearLibTerminal",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
	);

	if (!m_Window)
	{
		throw std::runtime_error(std::string("[SDL2] Failed to create a window (") + SDL_GetError() + ")");
	}

	m_GL_Context = SDL_GL_CreateContext(m_Window);
	if(!m_GL_Context)
	{
		throw std::runtime_error(std::string("[SDL2] Failed to create GL context (") + SDL_GetError() + ")");
	}

	if (SDL_GL_MakeCurrent(m_Window, m_GL_Context) < 0)
	{
		throw std::runtime_error(std::string("[SDL2] Failed to make GL context current for the window (") + SDL_GetError() + ")");
	}
}

void SDL2Window::Dispose()
{
	if (m_GL_Context)
	{
		SDL_GL_MakeCurrent(m_Window, nullptr);
		SDL_GL_DeleteContext(m_GL_Context);
		m_GL_Context = nullptr;
	}

	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}

	SDL_Quit();
}

Size SDL2Window::GetActualSize()
{
	int w, h;
	SDL_GetWindowSize(m_Window, &w, &h);
	return Size{w, h};
}

Size SDL2Window::GetDrawableSize()
{
	int w, h;
	SDL_GL_GetDrawableSize(m_Window, &w, &h);
	return Size{w, h};
}

std::wstring SDL2Window::GetClipboard()
{
	char* s = SDL_GetClipboardText();
	if (s == nullptr)
		return {};

	std::wstring result = UTF8Encoding{}.Convert(s);
	SDL_free(s);
	return result;
}

void SDL2Window::SetTitle(const std::wstring& title)
{
	SDL_SetWindowTitle(m_Window, UTF8Encoding{}.Convert(title).c_str());
}

void SDL2Window::SetIcon(const std::wstring& filename)
{
	// FIXME: NYI
	// XXX: SDL uses a single image, but a set of resolutions may be necessary (e. g. window icon and icon in taskbar)
}

void SDL2Window::SetSizeHints(Size increment, Size minimum_size)
{
	Window::SetSizeHints(increment, minimum_size);
	Size pixel_size = m_minimum_size * m_cell_size;
	SDL_SetWindowMinimumSize(m_Window, pixel_size.width, pixel_size.height);
}

void SDL2Window::SetClientSize(const Size& size)
{
	if (!m_Window)
		return;

	SDL_SetWindowSize(m_Window, size.width, size.height);
}

void SDL2Window::Show()
{
	SDL_ShowWindow(m_Window);
}

void SDL2Window::Hide()
{
	SDL_HideWindow(m_Window);
}

void SDL2Window::SwapBuffers()
{
	SDL_GL_SwapWindow(m_Window);
}

void SDL2Window::SetVSync(bool enabled)
{
	SDL_GL_SetSwapInterval(enabled? 1: 0);
}

void SDL2Window::SetResizeable(bool resizeable)
{
	SDL_SetWindowResizable(m_Window, resizeable? SDL_TRUE: SDL_FALSE);
}

void SDL2Window::SetFullscreen(bool fullscreen)
{
	if (SDL_SetWindowFullscreen(m_Window, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: 0) != 0)
	{
		LOG(Error, "Failed to set fullscreen to " << fullscreen << " (" << SDL_GetError() << ")");
	}
	// TODO: keep window size state (or does SDL keep it by itself?)
}

void SDL2Window::SetCursorVisibility(bool visible)
{
	SDL_ShowCursor(visible? SDL_TRUE: SDL_FALSE);
}

static std::unordered_map<int, int> g_keycode_map = {
	{SDLK_a, TK_A},
	{SDLK_b, TK_B},
	{SDLK_c, TK_C},
	{SDLK_d, TK_D},
	{SDLK_e, TK_E},
	{SDLK_f, TK_F},
	{SDLK_g, TK_G},
	{SDLK_h, TK_H},
	{SDLK_i, TK_I},
	{SDLK_j, TK_J},
	{SDLK_k, TK_K},
	{SDLK_l, TK_L},
	{SDLK_m, TK_M},
	{SDLK_n, TK_N},
	{SDLK_o, TK_O},
	{SDLK_p, TK_P},
	{SDLK_q, TK_Q},
	{SDLK_r, TK_R},
	{SDLK_s, TK_S},
	{SDLK_t, TK_T},
	{SDLK_u, TK_U},
	{SDLK_v, TK_V},
	{SDLK_w, TK_W},
	{SDLK_x, TK_X},
	{SDLK_y, TK_Y},
	{SDLK_z, TK_Z},
	{SDLK_1, TK_1},
	{SDLK_2, TK_2},
	{SDLK_3, TK_3},
	{SDLK_4, TK_4},
	{SDLK_5, TK_5},
	{SDLK_6, TK_6},
	{SDLK_7, TK_7},
	{SDLK_8, TK_8},
	{SDLK_9, TK_9},
	{SDLK_0, TK_0},
	{SDLK_RETURN, TK_RETURN},
	{SDLK_ESCAPE, TK_ESCAPE},
	{SDLK_BACKSPACE, TK_BACKSPACE},
	{SDLK_TAB, TK_TAB},
	{SDLK_SPACE, TK_SPACE},
	{SDLK_MINUS, TK_MINUS},
	{SDLK_EQUALS, TK_EQUALS},
	{SDLK_LEFTBRACKET, TK_LBRACKET},
	{SDLK_RIGHTBRACKET, TK_RBRACKET},
	{SDLK_BACKSLASH, TK_BACKSLASH},
	{SDLK_SEMICOLON, TK_SEMICOLON},
	{SDLK_QUOTE, TK_APOSTROPHE},
	{SDLK_BACKQUOTE, TK_GRAVE},
	{SDLK_COMMA, TK_COMMA},
	{SDLK_PERIOD, TK_PERIOD},
	{SDLK_SLASH, TK_SLASH},
	{SDLK_F1, TK_F1},
	{SDLK_F2, TK_F2},
	{SDLK_F3, TK_F3},
	{SDLK_F4, TK_F4},
	{SDLK_F5, TK_F5},
	{SDLK_F6, TK_F6},
	{SDLK_F7, TK_F7},
	{SDLK_F8, TK_F8},
	{SDLK_F9, TK_F9},
	{SDLK_F10, TK_F10},
	{SDLK_F11, TK_F11},
	{SDLK_F12, TK_F12},
	{SDLK_PAUSE, TK_PAUSE},
	{SDLK_INSERT, TK_INSERT},
	{SDLK_HOME, TK_HOME},
	{SDLK_PAGEUP, TK_PAGEUP},
	{SDLK_DELETE, TK_DELETE},
	{SDLK_END, TK_END},
	{SDLK_PAGEDOWN, TK_PAGEDOWN},
	{SDLK_RIGHT, TK_RIGHT},
	{SDLK_LEFT, TK_LEFT},
	{SDLK_DOWN, TK_DOWN},
	{SDLK_UP, TK_UP},
	{SDLK_KP_DIVIDE, TK_KP_DIVIDE},
	{SDLK_KP_MULTIPLY, TK_KP_MULTIPLY},
	{SDLK_KP_MINUS, TK_KP_MINUS},
	{SDLK_KP_PLUS, TK_KP_PLUS},
	{SDLK_KP_ENTER, TK_KP_ENTER},
	{SDLK_KP_1, TK_KP_1},
	{SDLK_KP_2, TK_KP_2},
	{SDLK_KP_3, TK_KP_3},
	{SDLK_KP_4, TK_KP_4},
	{SDLK_KP_5, TK_KP_5},
	{SDLK_KP_6, TK_KP_6},
	{SDLK_KP_7, TK_KP_7},
	{SDLK_KP_8, TK_KP_8},
	{SDLK_KP_9, TK_KP_9},
	{SDLK_KP_0, TK_KP_0},
	{SDLK_KP_PERIOD, TK_KP_PERIOD},
	{SDLK_LSHIFT, TK_SHIFT},
	{SDLK_RSHIFT, TK_SHIFT},
	{SDLK_LCTRL, TK_CONTROL},
	{SDLK_RCTRL, TK_CONTROL},
	{SDLK_LALT, TK_ALT},
	{SDLK_RALT, TK_ALT}
};

static std::unordered_map<int, int> g_button_map = {
	{SDL_BUTTON_LEFT, TK_MOUSE_LEFT},
	{SDL_BUTTON_RIGHT, TK_MOUSE_RIGHT},
	{SDL_BUTTON_MIDDLE, TK_MOUSE_MIDDLE},
	{SDL_BUTTON_X1, TK_MOUSE_X1},
	{SDL_BUTTON_X2, TK_MOUSE_X2}
};

int SDL2Window::PumpEvents()
{
	SDL_Event event = {};
	int processed = 0;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE))
		{
			m_event_handler(Event{TK_CLOSE, TK_CLOSE});
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				Event e{TK_RESIZED, TK_RESIZED};
				e[TK_WIDTH] = event.window.data1;
				e[TK_HEIGHT] = event.window.data2;
				m_event_handler(e);
			}
			else if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
			{
				m_event_handler(Event{TK_EVENT, TK_INVALIDATE});
				m_event_handler(Event{TK_EVENT, TK_REDRAW});
			}
			else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				m_event_handler(Event{TK_EVENT, TK_ACTIVATED});
			}
		}
		else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		{
			bool pressed = event.type == SDL_KEYDOWN;
			int type = pressed? TK_KEY_DOWN: TK_KEY_UP;
			auto i = g_keycode_map.find(event.key.keysym.sym);
			if (i != g_keycode_map.end())
			{
				int code = i->second;
				Event e{type, code | (pressed? 0: TK_KEY_RELEASED)};
				e[TK_KEY_CODE] = code;
				e[TK_KEY_SCANCODE] = event.key.keysym.scancode;
				e[TK_KEY_REPEAT] = event.key.repeat;
				e[code] = pressed? 1: 0;
				m_event_handler(std::move(e));
			}
			else
			{
				Event e{type, type};
				e[TK_KEY_CODE] = 0;
				e[TK_KEY_SCANCODE] = event.key.keysym.scancode;
				e[TK_KEY_REPEAT] = event.key.repeat;
				m_event_handler(std::move(e));
			}
		}
		else if (event.type == SDL_TEXTINPUT)
		{
			std::wstring s = UTF8Encoding{}.Convert(event.text.text);
			for (wchar_t c: s)
			{
				Event e{TK_TEXT, TK_TEXT};
				e[TK_WCHAR] = c;
				m_event_handler(std::move(e));
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN || event.type  == SDL_MOUSEBUTTONUP)
		{
			auto i = g_button_map.find(event.button.button);
			if (i != g_button_map.end())
			{
				bool pressed = (event.type == SDL_MOUSEBUTTONDOWN);
				Event e{pressed? TK_MOUSE_BUTTON_DOWN: TK_MOUSE_BUTTON_UP, i->second | (pressed? 0: TK_KEY_RELEASED)};
				e[TK_MOUSE_BUTTON] = i->second;
				e[TK_MOUSE_CLICKS] = pressed? event.button.clicks: 0;
				e[i->second] = pressed? 1: 0;
				m_event_handler(std::move(e));
			}
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			Event e{TK_MOUSE_MOVE, TK_MOUSE_MOVE};
			e[TK_MOUSE_PIXEL_X] = event.motion.x;
			e[TK_MOUSE_PIXEL_Y] = event.motion.y;
			m_event_handler(std::move(e));
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
			Event e{TK_MOUSE_SCROLL, TK_MOUSE_SCROLL};
			e[TK_MOUSE_WHEEL] = -event.wheel.y * (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED? -1: +1);
			m_event_handler(std::move(e));
		}

		processed += 1;
	}

	return processed;
}

} // namespace BearLibTerminal
