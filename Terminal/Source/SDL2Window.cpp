/*
* BearLibTerminal
* Copyright (C) 2014 Cfyz
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

#if defined(USE_SDL)
#include "Encoding.hpp"
#include "Platform.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"
#include <stdint.h>
#include <future>

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"

namespace BearLibTerminal
{
	namespace
	{
		typedef struct SDL_Window SDL_Window;

		typedef void* SDL_GLContext;

		typedef struct SDL_Surface SDL_Surface;

		enum SDL_EventType
		{
		    SDL_FIRSTEVENT     = 0,       // Unused (do not remove)
		    SDL_QUIT           = 0x100,   // User-requested quit
		    SDL_WINDOWEVENT    = 0x200,   // Window state change
		    SDL_KEYDOWN        = 0x300,   // Key pressed
		    SDL_KEYUP,                    // Key released
		    SDL_TEXTEDITING,			  // Keyboard text editing (composition)
		    SDL_TEXTINPUT,                // Keyboard text input
		    SDL_MOUSEMOTION    = 0x400,   // Mouse moved
		    SDL_MOUSEBUTTONDOWN,          // Mouse button pressed
		    SDL_MOUSEBUTTONUP,            // Mouse button released
		    SDL_MOUSEWHEEL,               // Mouse wheel motion
		    SDL_USEREVENT      = 0x8000,
		};

		struct SDL_WindowEvent
		{
		    uint32_t type;                // ::SDL_WINDOWEVENT
		    uint32_t timestamp;
		    uint32_t windowID;            // The associated window
		    uint8_t event;                // ::SDL_WindowEventID
		    uint8_t padding1;
		    uint8_t padding2;
		    uint8_t padding3;
		    int32_t data1;                // event dependent data
		    int32_t data2;                // event dependent data
		};

		typedef int32_t SDL_Scancode;     // NOTE: it was an enum in SDL2 headers
		typedef int32_t SDL_Keycode;

		static std::vector<int> ScancodeMapping =
		{
			0,             // 0
			0,             // 1
			0,             // 2
			0,             // 3
			TK_A,          // 4
			TK_B,          // 5
			TK_C,          // 6
			TK_D,          // 7
			TK_E,          // 8
			TK_F,          // 9
			TK_G,          // 10
			TK_H,          // 11
			TK_I,          // 12
			TK_J,          // 13
			TK_K,          // 14
			TK_L,          // 15
			TK_M,          // 16
			TK_N,          // 17
			TK_O,          // 18
			TK_P,          // 19
			TK_Q,          // 20
			TK_R,          // 21
			TK_S,          // 22
			TK_T,          // 23
			TK_U,          // 24
			TK_V,          // 25
			TK_W,          // 26
			TK_X,          // 27
			TK_Y,          // 28
			TK_Z,          // 29
			TK_1,          // 30
			TK_2,          // 31
			TK_3,          // 32
			TK_4,          // 33
			TK_5,          // 34
			TK_6,          // 35
			TK_7,          // 36
			TK_8,          // 37
			TK_9,          // 38
			TK_0,          // 39
			TK_RETURN,     // 40
			TK_ESCAPE,     // 41
			TK_BACKSPACE,  // 42
			TK_TAB,        // 43
			TK_SPACE,      // 44
			TK_MINUS,      // 45
			TK_EQUALS,     // 46
			TK_LBRACKET,   // 47
			TK_RBRACKET,   // 48
			TK_BACKSLASH,  // 49
			TK_BACKSLASH,  // 50 Non-US slash
			TK_SEMICOLON,  // 51
			TK_APOSTROPHE, // 52
			TK_GRAVE,      // 53
			TK_COMMA,      // 54
			TK_PERIOD,     // 55
			TK_SLASH,      // 56
			0,             // 57 Caps Lock
			TK_F1,         // 58
			TK_F2,         // 59
			TK_F3,         // 60
			TK_F4,         // 61
			TK_F5,         // 62
			TK_F6,         // 63
			TK_F7,         // 64
			TK_F8,         // 65
			TK_F9,         // 66
			TK_F10,        // 67
			TK_F11,        // 68
			TK_F12,        // 69
			0,             // 70 Print Screen
			0,             // 71 Scroll Lock
			TK_PAUSE,      // 72
			TK_INSERT,     // 73
			TK_HOME,       // 74
			TK_PRIOR,      // 75
			TK_DELETE,     // 76
			TK_END,        // 77
			TK_NEXT,       // 78
			TK_RIGHT,      // 79
			TK_LEFT,       // 80
			TK_DOWN,       // 81
			TK_UP,         // 82
			0,             // 83 Num Lock Clear
			TK_DIVIDE,     // 84
			TK_MULTIPLY,   // 85
			TK_SUBTRACT,   // 86
			TK_ADD,        // 87
			TK_RETURN,     // 88 Numpad Enter
			TK_NUMPAD1,    // 89
			TK_NUMPAD2,    // 90
			TK_NUMPAD3,    // 91
			TK_NUMPAD4,    // 92
			TK_NUMPAD5,    // 93
			TK_NUMPAD6,    // 94
			TK_NUMPAD7,    // 95
			TK_NUMPAD8,    // 96
			TK_NUMPAD9,    // 97
			TK_NUMPAD0,    // 98
			TK_DECIMAL,    // 99
			0,             // 100 Non-US backslash
			0,             // 101 Application, context menu
			0,             // 102 Power
			TK_EQUALS,     // 103 '=' on keypad
			0,             // 104
			0,             // 105
			0,             // 106
			0,             // 107
			0,             // 108
			0,             // 109
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 110-119
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 120-129
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 130-139
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 140-149
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 150-159
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 160-169
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 170-179
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 180-189
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 190-199
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 200-209
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 210-219
			0,             // 220
			0,             // 221
			0,             // 222
			0,             // 223
			TK_CONTROL,    // 224 Left Control
			TK_SHIFT,      // 225 Left Shift
			0,             // 226 Left Alt
			0,             // 227 Left GUI
			TK_CONTROL,    // 228 Right Control
			TK_SHIFT,      // 229 Right Shift
			0,             // 230 Right Alt
			0,             // 231 Right GUI
			0              // 232
		};

		struct SDL_Keysym
		{
		    SDL_Scancode scancode;      // SDL physical key code - see ::SDL_Scancode for details
		    SDL_Keycode sym;            // SDL virtual key code - see ::SDL_Keycode for details
		    uint16_t mod;               // current key modifiers
		    uint32_t unused;
		};

		struct SDL_KeyboardEvent
		{
		    uint32_t type;        // ::SDL_KEYDOWN or ::SDL_KEYUP
		    uint32_t timestamp;
		    uint32_t windowID;    // The window with keyboard focus, if any
		    uint8_t state;        // ::SDL_PRESSED or ::SDL_RELEASED
		    uint8_t repeat;       // Non-zero if this is a key repeat
		    uint8_t padding2;
		    uint8_t padding3;
		    SDL_Keysym keysym;    // The key that was pressed or released
		};

		struct SDL_TextInputEvent
		{
		    uint32_t type;                              // ::SDL_TEXTINPUT
		    uint32_t timestamp;
		    uint32_t windowID;                          // The window with keyboard focus, if any
		    char text[32];//SDL_TEXTINPUTEVENT_TEXT_SIZE];  // The input text
		};

		struct SDL_MouseMotionEvent
		{
		    uint32_t type;        // ::SDL_MOUSEMOTION
		    uint32_t timestamp;
		    uint32_t windowID;    // The window with mouse focus, if any
		    uint32_t which;       // The mouse instance id, or SDL_TOUCH_MOUSEID
		    uint32_t state;       // The current button state
		    int32_t x;            // X coordinate, relative to window
		    int32_t y;            // Y coordinate, relative to window
		    int32_t xrel;         // The relative motion in the X direction
		    int32_t yrel;         // The relative motion in the Y direction
		};

		struct SDL_MouseButtonEvent
		{
		    uint32_t type;        // ::SDL_MOUSEBUTTONDOWN or ::SDL_MOUSEBUTTONUP
		    uint32_t timestamp;
		    uint32_t windowID;    // The window with mouse focus, if any
		    uint32_t which;       // The mouse instance id, or SDL_TOUCH_MOUSEID
		    uint8_t button;       // The mouse button index
		    uint8_t state;        // ::SDL_PRESSED or ::SDL_RELEASED
		    uint8_t clicks;       // 1 for single-click, 2 for double-click, etc.
		    uint8_t padding1;
		    int32_t x;            // X coordinate, relative to window
		    int32_t y;            // Y coordinate, relative to window
		};

		struct SDL_MouseWheelEvent
		{
		    uint32_t type;        // ::SDL_MOUSEWHEEL
		    uint32_t timestamp;
		    uint32_t windowID;    // The window with mouse focus, if any
		    uint32_t which;       // The mouse instance id, or SDL_TOUCH_MOUSEID
		    int32_t x;            // The amount scrolled horizontally, positive to the right and negative to the left
		    int32_t y;            // The amount scrolled vertically, positive away from the user and negative toward the user
		};

		struct SDL_QuitEvent
		{
		    uint32_t type;        // ::SDL_QUIT
		    uint32_t timestamp;
		};

		struct SDL_UserEvent
		{
		    uint32_t type;        // ::SDL_USEREVENT through ::SDL_LASTEVENT-1
		    uint32_t timestamp;
		    uint32_t windowID;    // The associated window if any
		    int32_t code;         // User defined event code
		    void *data1;          // User defined data pointer
		    void *data2;          // User defined data pointer
		};

		union SDL_Event
		{
		    uint32_t type;                  // Event type, shared with all events
		    SDL_WindowEvent window;         // Window event data
		    SDL_KeyboardEvent key;          // Keyboard event data
		    SDL_TextInputEvent text;        // Text input event data
		    SDL_MouseMotionEvent motion;    // Mouse motion event data
		    SDL_MouseButtonEvent button;    // Mouse button event data
		    SDL_MouseWheelEvent wheel;      // Mouse wheel event data
		    SDL_QuitEvent quit;             // Quit request event data
		    SDL_UserEvent user;             // Custom event data
		    uint8_t padding[56];
		};

		typedef const char* (*PFNSDLGETERROR)(void);
		typedef int (*PFNSDLINIT)(uint32_t flags);
		typedef void (*PFNSDLQUIT)(void);
		typedef SDL_Window* (*PFNSDLCREATEWINDOW)(const char *title, int x, int y, int w, int h, uint32_t flags);
		typedef void (*PFNSDLGLSWAPWINDOW)(SDL_Window* window);
		typedef int (*PFNSDLGLSETSWAPINTERVAL)(int interval);
		typedef void (*PFNSDLSETWINDOWICON)(SDL_Window* window, SDL_Surface* icon);
		typedef void (*PFNSDLSETWINDOWMINIMUMSIZE)(SDL_Window* window, int min_w, int min_h);
		typedef void (*PFNSDLSETWINDOWMAXIMUMSIZE)(SDL_Window* window, int min_w, int min_h);
		typedef void (*PFNSDLSETWINDOWSIZE)(SDL_Window* window, int w, int h);
		typedef void (*PFNSDLSETWINDOWTITLE)(SDL_Window* window, const char* title);
		typedef void (*PFNSDLSHOWWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLHIDEWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLRESTOREWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLDESTROYWINDOW)(SDL_Window* window);
		typedef SDL_Surface* (*PFNSDLCREATERGBSURFACEFROM)(void* pixels, int width, int height, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
		typedef void (*PFNSDLFREESURFACE)(SDL_Surface* surface);
		typedef int (*PFNSDLPOLLEVENT)(SDL_Event* event);
		typedef int (*PFNSDLPUSHEVENT)(SDL_Event* event);
		typedef void (*PFNSDLSTARTTEXTINPUT)(void);
		typedef void (*PFNSDLSTOPTEXTINPUT)(void);
		typedef SDL_Keycode (*PFNSDLGETKEYFROMSCANCODE)(SDL_Scancode scancode);
		typedef SDL_GLContext (*PFNSDLGLCREATECONTEXT)(SDL_Window* window);
		typedef void (*PFNSDLGLDELETECONTEXT)(SDL_GLContext context);
		typedef int (*PFNSDLGLMAKECURRENT)(SDL_Window* window, SDL_GLContext context);

		#define SDL_INIT_VIDEO          0x00000020
		#define SDL_WINDOWPOS_UNDEFINED 0x1FFF0000
		#define SDL_BUTTON_LEFT         1
		#define SDL_BUTTON_MIDDLE       2
		#define SDL_BUTTON_RIGHT        3

		enum SDL_WindowFlags
		{
		    SDL_WINDOW_FULLSCREEN = 0x00000001,         // fullscreen window
		    SDL_WINDOW_OPENGL = 0x00000002,             // window usable with OpenGL context
		    SDL_WINDOW_SHOWN = 0x00000004,              // window is visible
		    SDL_WINDOW_HIDDEN = 0x00000008,             // window is not visible
		    SDL_WINDOW_BORDERLESS = 0x00000010,         // no window decoration
		    SDL_WINDOW_RESIZABLE = 0x00000020,          // window can be resized
		    SDL_WINDOW_MINIMIZED = 0x00000040,          // window is minimized
		    SDL_WINDOW_MAXIMIZED = 0x00000080,          // window is maximized
		    SDL_WINDOW_INPUT_GRABBED = 0x00000100,      // window has grabbed input focus
		    SDL_WINDOW_INPUT_FOCUS = 0x00000200,        // window has input focus
		    SDL_WINDOW_MOUSE_FOCUS = 0x00000400,        // window has mouse focus
		    SDL_WINDOW_FULLSCREEN_DESKTOP = ( SDL_WINDOW_FULLSCREEN | 0x00001000 ),
		    SDL_WINDOW_FOREIGN = 0x00000800,            // window not created by SDL
		    SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000       // window should be created in high-DPI mode if supported
		};

		enum SDL_WindowEventID
		{
		    SDL_WINDOWEVENT_NONE,                       // Never used
		    SDL_WINDOWEVENT_SHOWN,                      // Window has been shown
		    SDL_WINDOWEVENT_HIDDEN,                     // Window has been hidden
		    SDL_WINDOWEVENT_EXPOSED,                    // Window has been exposed and should be redrawn
		    SDL_WINDOWEVENT_MOVED,                      // Window has been moved to data1, data2
		    SDL_WINDOWEVENT_RESIZED,                    // Window has been resized to data1xdata2
		    SDL_WINDOWEVENT_SIZE_CHANGED,               // The window size has changed, either as a result of an API call or through the system or user changing the window size.
		    SDL_WINDOWEVENT_MINIMIZED,                  // Window has been minimized
		    SDL_WINDOWEVENT_MAXIMIZED,                  // Window has been maximized
		    SDL_WINDOWEVENT_RESTORED,                   // Window has been restored to normal size and position
		    SDL_WINDOWEVENT_ENTER,                      // Window has gained mouse focus
		    SDL_WINDOWEVENT_LEAVE,                      // Window has lost mouse focus
		    SDL_WINDOWEVENT_FOCUS_GAINED,               // Window has gained keyboard focus
		    SDL_WINDOWEVENT_FOCUS_LOST,                 // Window has lost keyboard focus
		    SDL_WINDOWEVENT_CLOSE                       // The window manager requests that the window be closed
		};
	}

	struct SDL2Window::Private
	{
		Private();

		Module libSDL2;
		PFNSDLGETERROR SDL_GetError;
		PFNSDLINIT SDL_Init;
		PFNSDLQUIT SDL_Quit;
		PFNSDLCREATEWINDOW SDL_CreateWindow;
		PFNSDLGLSWAPWINDOW SDL_GL_SwapWindow;
		PFNSDLGLSETSWAPINTERVAL SDL_GL_SetSwapInterval;
		PFNSDLSETWINDOWICON SDL_SetWindowIcon;
		PFNSDLSETWINDOWMINIMUMSIZE SDL_SetWindowMinimumSize;
		PFNSDLSETWINDOWMAXIMUMSIZE SDL_SetWindowMaximumSize;
		PFNSDLSETWINDOWSIZE SDL_SetWindowSize;
		PFNSDLSETWINDOWTITLE SDL_SetWindowTitle;
		PFNSDLSHOWWINDOW SDL_ShowWindow;
		PFNSDLHIDEWINDOW SDL_HideWindow;
		PFNSDLRESTOREWINDOW SDL_RestoreWindow;
		PFNSDLDESTROYWINDOW SDL_DestroyWindow;
		PFNSDLCREATERGBSURFACEFROM SDL_CreateRGBSurfaceFrom;
		PFNSDLFREESURFACE SDL_FreeSurface;
		PFNSDLPOLLEVENT SDL_PollEvent;
		PFNSDLPUSHEVENT SDL_PushEvent;
		PFNSDLSTARTTEXTINPUT SDL_StartTextInput;
		PFNSDLSTOPTEXTINPUT SDL_StopTextInput;
		PFNSDLGETKEYFROMSCANCODE SDL_GetKeyFromScancode;
		PFNSDLGLCREATECONTEXT SDL_GL_CreateContext;
		PFNSDLGLDELETECONTEXT SDL_GL_DeleteContext;
		PFNSDLGLMAKECURRENT SDL_GL_MakeCurrent;

		SDL_Window* window;
		SDL_GLContext context;
	};

	SDL2Window::Private::Private()
	{
#if defined (__linux__)
		libSDL2 = Module(L"libSDL2-2.0.so.0"); // TODO: comment about soname
#else
		libSDL2 = Module(L"SDL2.dll");
#endif

		SDL_GetError = (PFNSDLGETERROR)libSDL2["SDL_GetError"];
		SDL_Init = (PFNSDLINIT)libSDL2["SDL_Init"];
		SDL_Quit = (PFNSDLQUIT)libSDL2["SDL_Quit"];
		SDL_CreateWindow = (PFNSDLCREATEWINDOW )libSDL2["SDL_CreateWindow"];
		SDL_GL_SwapWindow = (PFNSDLGLSWAPWINDOW)libSDL2["SDL_GL_SwapWindow"];
		SDL_GL_SetSwapInterval = (PFNSDLGLSETSWAPINTERVAL)libSDL2["SDL_GL_SetSwapInterval"];
		SDL_SetWindowIcon = (PFNSDLSETWINDOWICON)libSDL2["SDL_SetWindowIcon"];
		SDL_SetWindowMinimumSize = (PFNSDLSETWINDOWMINIMUMSIZE)libSDL2["SDL_SetWindowMinimumSize"];
		SDL_SetWindowMaximumSize = (PFNSDLSETWINDOWMAXIMUMSIZE)libSDL2["SDL_SetWindowMaximumSize"];
		SDL_SetWindowSize = (PFNSDLSETWINDOWSIZE)libSDL2["SDL_SetWindowSize"];
		SDL_SetWindowTitle = (PFNSDLSETWINDOWTITLE)libSDL2["SDL_SetWindowTitle"];
		SDL_ShowWindow = (PFNSDLSHOWWINDOW)libSDL2["SDL_ShowWindow"];
		SDL_HideWindow = (PFNSDLHIDEWINDOW)libSDL2["SDL_HideWindow"];
		SDL_RestoreWindow = (PFNSDLRESTOREWINDOW)libSDL2["SDL_RestoreWindow"];
		SDL_DestroyWindow = (PFNSDLDESTROYWINDOW)libSDL2["SDL_DestroyWindow"];
		SDL_CreateRGBSurfaceFrom = (PFNSDLCREATERGBSURFACEFROM)libSDL2["SDL_CreateRGBSurfaceFrom"];
		SDL_FreeSurface = (PFNSDLFREESURFACE)libSDL2["SDL_FreeSurface"];
		SDL_PollEvent = (PFNSDLPOLLEVENT)libSDL2["SDL_PollEvent"];
		SDL_PushEvent = (PFNSDLPUSHEVENT)libSDL2["SDL_PushEvent"];
		SDL_StartTextInput = (PFNSDLSTARTTEXTINPUT)libSDL2["SDL_StartTextInput"];
		SDL_StopTextInput = (PFNSDLSTOPTEXTINPUT)libSDL2["SDL_StopTextInput"];
		SDL_GetKeyFromScancode = (PFNSDLGETKEYFROMSCANCODE)libSDL2["SDL_GetKeyFromScancode"];
		SDL_GL_CreateContext = (PFNSDLGLCREATECONTEXT)libSDL2["SDL_GL_CreateContext"];
		SDL_GL_DeleteContext = (PFNSDLGLDELETECONTEXT)libSDL2["SDL_GL_DeleteContext"];
		SDL_GL_MakeCurrent = (PFNSDLGLMAKECURRENT)libSDL2["SDL_GL_MakeCurrent"];

		window = nullptr;
		context = nullptr;
	}

	SDL2Window::SDL2Window():
		m_mouse_wheel(0),
		m_pending_stroke(Keystroke::KeyPress, 0, 0),
		m_pending_stroke_time(0),
		m_resizeable(false),
		m_maximized(false)
	{
		try
		{
			m_private = std::make_unique<Private>();
		}
		catch (std::exception& e)
		{
			LOG(Fatal, "Loading SDL2 failed: " << e.what());
			throw std::runtime_error("SDL2Window construction failed");
		}
	}

	SDL2Window::~SDL2Window()
	{
		Stop();
	}

	bool SDL2Window::ValidateIcon(const std::wstring& filename)
	{
		return true; // FIXME: NYI
	}

	void SDL2Window::SetTitle(const std::wstring& title)
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_SetWindowTitle(m_private->window, UTF8->Convert(title).c_str());
	}

	void SDL2Window::SetIcon(const std::wstring& filename)
	{
		// FIXME: NYI
	}

	void SDL2Window::SetClientSize(const Size& size)
	{
		if (m_private->window == nullptr) return;

		if (m_maximized)
		{
			m_private->SDL_RestoreWindow(m_private->window);
			m_maximized = false;
		}

		m_private->SDL_SetWindowSize(m_private->window, size.width, size.height);
		m_client_size = size;

		UpdateSizeHints();
	}

	void SDL2Window::SetResizeable(bool resizeable)
	{
		m_resizeable = resizeable;
		UpdateSizeHints();
	}

	void SDL2Window::UpdateSizeHints()
	{
		if (m_private->window == nullptr) return;

		Size minimum = m_resizeable? m_cell_size*m_minimum_size: m_client_size;
		m_private->SDL_SetWindowMinimumSize(m_private->window, minimum.width, minimum.height);

		Size maximum = m_resizeable? Size(1E+6, 1E+6): minimum;
		m_private->SDL_SetWindowMaximumSize(m_private->window, maximum.width, maximum.height);
	}

	void SDL2Window::Redraw()
	{
		// TODO: should not be necessary, remove and use Invoke
	}

	void SDL2Window::Show()
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_ShowWindow(m_private->window);
	}

	void SDL2Window::Hide()
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_HideWindow(m_private->window);
	}

	void SDL2Window::Invoke(std::function<void()> func)
	{
		if (m_private->window == nullptr) return;

		auto task = new std::packaged_task<void()>(func);
		auto future = task->get_future();

		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.data1 = (void*)task;

		m_private->SDL_PushEvent(&event);
		future.wait();
	}

	void SDL2Window::SwapBuffers()
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_GL_SwapWindow(m_private->window);
	}

	void SDL2Window::SetVSync(bool enabled)
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_GL_SetSwapInterval(enabled? 1: 0);
	}

	bool SDL2Window::PumpEvents()
	{
		SDL_Event event;
		while (m_private->SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				Keystroke stroke(Keystroke::KeyPress, TK_CLOSE);
				if (m_on_input) m_on_input(stroke);
			}
			else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			{
				int sdl_scancode = event.key.keysym.scancode;
				if (sdl_scancode < 0 || sdl_scancode >= (int)ScancodeMapping.size())
				{
					// Not in acceptable scancode range
					continue;
				}

				int scancode = ScancodeMapping[sdl_scancode];
				if (scancode == 0)
				{
					// Does not map on terminal scancode
					continue;
				}

				if (m_pending_stroke.scancode > 0)
				{
					m_pending_stroke.type = Keystroke::KeyPress;
					m_on_input(m_pending_stroke);
					m_pending_stroke.scancode = 0;
				}

				if (event.type == SDL_KEYDOWN)
				{
					m_pending_stroke.scancode = scancode;
					m_pending_stroke.character = 0;
					m_pending_stroke_time = gettime();
				}
				else
				{
					Keystroke stroke(Keystroke::KeyRelease, scancode);
					m_on_input(stroke);
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				Keystroke stroke(Keystroke::MouseMove, TK_MOUSE_MOVE, event.motion.x, event.motion.y, 0);
				m_on_input(stroke);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
			{
				Keystroke::Type type = (event.type == SDL_MOUSEBUTTONDOWN? Keystroke::KeyPress: Keystroke::KeyRelease);
				int scancodes[] = {0, TK_LBUTTON, TK_MBUTTON, TK_RBUTTON};
				if (event.button.button < 1 || event.button.button > 3)
				{
					// Out of acceptable range
					continue;
				}

				Keystroke stroke(type, scancodes[event.button.button]);
				m_on_input(stroke);
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				m_mouse_wheel += event.wheel.y;
				Keystroke stroke(Keystroke::MouseScroll, TK_MOUSE_SCROLL, 0, 0, m_mouse_wheel);
				m_on_input(stroke);
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
				{
					//if (this->m_on_redraw())
					//{
					//	m_private->SDL_GL_SwapWindow(m_private->window
					//);
				}
				else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					m_client_size = Size(event.window.data1, event.window.data2);
					Keystroke stroke(Keystroke::KeyPress, TK_WINDOW_RESIZE, m_client_size.width, m_client_size.height, 0);
					m_on_input(stroke);
				}
				else if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
				{
					m_maximized = true;
				}
				else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED || event.window.event == SDL_WINDOWEVENT_RESTORED)
				{
					m_maximized = false;
				}
			}
			else if (event.type == SDL_TEXTINPUT)
			{
				if (m_pending_stroke.scancode > 0)
				{
					std::wstring text = UTF8->Convert(event.text.text);
					if (text.length() > 0)
					{
						m_pending_stroke.character = (char16_t)text[0];
						m_pending_stroke.type = Keystroke::KeyPress | Keystroke::Unicode;
					}
					m_on_input(m_pending_stroke);
					m_pending_stroke.scancode = 0;
				}
			}
			else if (event.type == SDL_USEREVENT)
			{
				auto task = static_cast<std::packaged_task<void()>*>(event.user.data1);
				(*task)();
				delete task;
			}
		}

		if (m_pending_stroke.scancode && gettime() > m_pending_stroke_time+2500)
		{
			m_pending_stroke.type = Keystroke::KeyPress;
			m_on_input(m_pending_stroke);
			m_pending_stroke.scancode = 0;
		}

		return false;
	}

	void SDL2Window::ThreadFunction()
	{
		while (m_proceed)
		{
			if (!PumpEvents())
			{
				int delay = m_pending_stroke.scancode? 1: 5;
				std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			}
		}

		// There was redraw barrier notify call
	}

	bool SDL2Window::Construct()
	{
		if (m_private->SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			LOG(Fatal, "SDL_Init failed: " << m_private->SDL_GetError());
			return false;
		}

		m_private->window = m_private->SDL_CreateWindow
		(
			"BearLibTerminal",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			640,
			400,
			SDL_WINDOW_HIDDEN|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
		);
		if (m_private->window == nullptr)
		{
			LOG(Fatal, "SDL_CreateWindow failed: " << m_private->SDL_GetError());
			Destroy();
			return false;
		}

		m_private->context = m_private->SDL_GL_CreateContext(m_private->window);
		if (m_private->window == nullptr)
		{
			LOG(Fatal, "SDL_GL_CreateContext failed: " << m_private->SDL_GetError());
			Destroy();
			return false;
		}

		m_proceed = true;

		return true;
	}

	void SDL2Window::Destroy()
	{
		if (m_private->context != nullptr && m_private->window != nullptr)
		{
			m_private->SDL_GL_MakeCurrent(m_private->window, nullptr);
			m_private->SDL_GL_DeleteContext(m_private->context);
			m_private->context = nullptr;
		}

		if (m_private->window != nullptr)
		{
			m_private->SDL_DestroyWindow(m_private->window);
			m_private->window = nullptr;
		}

		m_private->SDL_Quit();
	}
}
#endif
