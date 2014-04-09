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
#include <stdint.h>
#include <future>

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace BearLibTerminal
{
	namespace
	{
		typedef struct SDL_Window SDL_Window;

		//typedef struct SDL_Renderer SDL_Renderer;

		typedef void* SDL_GLContext;

		/*
		struct SDL_RendererInfo
		{
		    const char *name;             // The name of the renderer
		    uint32_t flags;               // Supported ::SDL_RendererFlags
		    uint32_t num_texture_formats; // The number of available texture formats
		    uint32_t texture_formats[16]; // The available texture formats
		    int max_texture_width;        // The maximimum texture width
		    int max_texture_height;       // The maximimum texture height
		};
		//*/

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

		/*
		typedef struct SDL_CommonEvent
		{
		    uint32_t type;
		    uint32_t timestamp;
		} SDL_CommonEvent;
		//*/

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

		/*
		typedef struct SDL_TextEditingEvent
		{
		    uint32_t type;                                // ::SDL_TEXTEDITING
		    uint32_t timestamp;
		    uint32_t windowID;                            // The window with keyboard focus, if any
		    char text[32];      // The editing text
		    int32_t start;                               // The start cursor of selected editing text
		    int32_t length;                              // The length of selected editing text
		} SDL_TextEditingEvent;

		typedef struct SDL_TextInputEvent
		{
		    uint32_t type;                              // ::SDL_TEXTINPUT
		    uint32_t timestamp;
		    uint32_t windowID;                          // The window with keyboard focus, if any
		    char text[32];//SDL_TEXTINPUTEVENT_TEXT_SIZE];  // The input text
		} SDL_TextInputEvent;
		//*/

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
		    //SDL_CommonEvent common;         // Common event data
		    SDL_WindowEvent window;         // Window event data
		    SDL_KeyboardEvent key;          // Keyboard event data
		    //SDL_TextEditingEvent edit;      /**< Text editing event data */
		    //SDL_TextInputEvent text;        /**< Text input event data */
		    SDL_MouseMotionEvent motion;    // Mouse motion event data
		    SDL_MouseButtonEvent button;    // Mouse button event data
		    SDL_MouseWheelEvent wheel;      // Mouse wheel event data
		    //SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
		    //SDL_JoyBallEvent jball;         /**< Joystick ball event data */
		    //SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
		    //SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
		    //SDL_JoyDeviceEvent jdevice;     /**< Joystick device change event data */
		    //SDL_ControllerAxisEvent caxis;      /**< Game Controller axis event data */
		    //SDL_ControllerButtonEvent cbutton;  /**< Game Controller button event data */
		    //SDL_ControllerDeviceEvent cdevice;  /**< Game Controller device event data */
		    SDL_QuitEvent quit;             // Quit request event data
		    SDL_UserEvent user;             // Custom event data
		    //SDL_SysWMEvent syswm;           /**< System dependent window event data */
		    //SDL_TouchFingerEvent tfinger;   /**< Touch finger event data */
		    //SDL_MultiGestureEvent mgesture; /**< Gesture event data */
		    //SDL_DollarGestureEvent dgesture; /**< Gesture event data */
		    //SDL_DropEvent drop;             /**< Drag and drop event data */

		    /* This is necessary for ABI compatibility between Visual C++ and GCC
		       Visual C++ will respect the push pack pragma and use 52 bytes for
		       this structure, and GCC will use the alignment of the largest datatype
		       within the union, which is 8 bytes.

		       So... we'll add padding to force the size to be 56 bytes for both.
		    */
		    uint8_t padding[56];
		};

		typedef const char* (*PFNSDLGETERROR)(void);
		typedef int (*PFNSDLINIT)(uint32_t flags);
		typedef void (*PFNSDLQUIT)(void);
		typedef SDL_Window* (*PFNSDLCREATEWINDOW)(const char *title, int x, int y, int w, int h, uint32_t flags);
		//typedef SDL_Renderer* (*PFNSDLCREATERENDERER)(SDL_Window* window, int index, uint32_t flags);
		//typedef int (*PFNSDLGETRENDERERINFO)(SDL_Renderer* renderer, SDL_RendererInfo* info);
		//typedef void (*PFNSDLRENDERPRESENT)(SDL_Renderer* renderer);
		typedef void (*PFNSDLGLSWAPWINDOW)(SDL_Window* window);
		typedef int (*PFNSDLGLSETSWAPINTERVAL)(int interval);
		typedef void (*PFNSDLSETWINDOWICON)(SDL_Window* window, SDL_Surface* icon);
		typedef void (*PFNSDLSETWINDOWMINIMUMSIZE)(SDL_Window* window, int min_w, int min_h);
		typedef void (*PFNSDLSETWINDOWSIZE)(SDL_Window* window, int w, int h);
		typedef void (*PFNSDLSETWINDOWTITLE)(SDL_Window* window, const char* title);
		typedef void (*PFNSDLSHOWWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLHIDEWINDOW)(SDL_Window* window);
		//typedef void (*PFNSDLDESTROYRENDERER)(SDL_Renderer* renderer);
		typedef void (*PFNSDLDESTROYWINDOW)(SDL_Window* window);
		typedef SDL_Surface* (*PFNSDLCREATERGBSURFACEFROM)(void* pixels, int width, int height, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
		typedef void (*PFNSDLFREESURFACE)(SDL_Surface* surface);
		typedef int (*PFNSDLPOLLEVENT)(SDL_Event* event);
		typedef int (*PFNSDLPUSHEVENT)(SDL_Event* event);
		//typedef void (*PFNSDLSTARTTEXTINPUT)(void);
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

		/*
		enum SDL_RendererFlags
		{
		    SDL_RENDERER_SOFTWARE = 0x00000001,         // The renderer is a software fallback
		    SDL_RENDERER_ACCELERATED = 0x00000002,      // The renderer uses hardware acceleration
		    SDL_RENDERER_PRESENTVSYNC = 0x00000004,     // Present is synchronized with the refresh rate
		    SDL_RENDERER_TARGETTEXTURE = 0x00000008     // The renderer supports rendering to texture
		};
		//*/
	}

	struct SDL2Window::Private
	{
		Private();

		Module libSDL2;
		PFNSDLGETERROR SDL_GetError;
		PFNSDLINIT SDL_Init;
		PFNSDLQUIT SDL_Quit;
		PFNSDLCREATEWINDOW SDL_CreateWindow;
		//PFNSDLCREATERENDERER SDL_CreateRenderer;
		//PFNSDLGETRENDERERINFO SDL_GetRendererInfo;
		//PFNSDLRENDERPRESENT SDL_RenderPresent;
		PFNSDLGLSWAPWINDOW SDL_GL_SwapWindow;
		PFNSDLGLSETSWAPINTERVAL SDL_GL_SetSwapInterval;
		PFNSDLSETWINDOWICON SDL_SetWindowIcon;
		PFNSDLSETWINDOWMINIMUMSIZE SDL_SetWindowMinimumSize;
		PFNSDLSETWINDOWSIZE SDL_SetWindowSize;
		PFNSDLSETWINDOWTITLE SDL_SetWindowTitle;
		PFNSDLSHOWWINDOW SDL_ShowWindow;
		PFNSDLHIDEWINDOW SDL_HideWindow;
		//PFNSDLDESTROYRENDERER SDL_DestroyRenderer;
		PFNSDLDESTROYWINDOW SDL_DestroyWindow;
		PFNSDLCREATERGBSURFACEFROM SDL_CreateRGBSurfaceFrom;
		PFNSDLFREESURFACE SDL_FreeSurface;
		PFNSDLPOLLEVENT SDL_PollEvent;
		PFNSDLPUSHEVENT SDL_PushEvent;
		//PFNSDLSTARTTEXTINPUT SDL_StartTextInput;
		PFNSDLGETKEYFROMSCANCODE SDL_GetKeyFromScancode;
		PFNSDLGLCREATECONTEXT SDL_GL_CreateContext;
		PFNSDLGLDELETECONTEXT SDL_GL_DeleteContext;
		PFNSDLGLMAKECURRENT SDL_GL_MakeCurrent;

		SDL_Window* window;
		//SDL_Renderer* renderer;
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
		//SDL_CreateRenderer = (PFNSDLCREATERENDERER)libSDL2["SDL_CreateRenderer"];
		//SDL_GetRendererInfo = (PFNSDLGETRENDERERINFO)libSDL2["SDL_GetRendererInfo"];
		//SDL_RenderPresent = (PFNSDLRENDERPRESENT)libSDL2["SDL_RenderPresent"];
		SDL_GL_SwapWindow = (PFNSDLGLSWAPWINDOW)libSDL2["SDL_GL_SwapWindow"];
		SDL_GL_SetSwapInterval = (PFNSDLGLSETSWAPINTERVAL)libSDL2["SDL_GL_SetSwapInterval"];
		SDL_SetWindowIcon = (PFNSDLSETWINDOWICON)libSDL2["SDL_SetWindowIcon"];
		SDL_SetWindowMinimumSize = (PFNSDLSETWINDOWMINIMUMSIZE)libSDL2["SDL_SetWindowMinimumSize"];
		SDL_SetWindowSize = (PFNSDLSETWINDOWSIZE)libSDL2["SDL_SetWindowSize"];
		SDL_SetWindowTitle = (PFNSDLSETWINDOWTITLE)libSDL2["SDL_SetWindowTitle"];
		SDL_ShowWindow = (PFNSDLSHOWWINDOW)libSDL2["SDL_ShowWindow"];
		SDL_HideWindow = (PFNSDLHIDEWINDOW)libSDL2["SDL_HideWindow"];
		//SDL_DestroyRenderer = (PFNSDLDESTROYRENDERER)libSDL2["SDL_DestroyRenderer"];
		SDL_DestroyWindow = (PFNSDLDESTROYWINDOW)libSDL2["SDL_DestroyWindow"];
		SDL_CreateRGBSurfaceFrom = (PFNSDLCREATERGBSURFACEFROM)libSDL2["SDL_CreateRGBSurfaceFrom"];
		SDL_FreeSurface = (PFNSDLFREESURFACE)libSDL2["SDL_FreeSurface"];
		SDL_PollEvent = (PFNSDLPOLLEVENT)libSDL2["SDL_PollEvent"];
		SDL_PushEvent = (PFNSDLPUSHEVENT)libSDL2["SDL_PushEvent"];
		//SDL_StartTextInput = (PFNSDLSTARTTEXTINPUT)libSDL2["SDL_StartTextInput"];
		SDL_GetKeyFromScancode = (PFNSDLGETKEYFROMSCANCODE)libSDL2["SDL_GetKeyFromScancode"];
		SDL_GL_CreateContext = (PFNSDLGLCREATECONTEXT)libSDL2["SDL_GL_CreateContext"];
		SDL_GL_DeleteContext = (PFNSDLGLDELETECONTEXT)libSDL2["SDL_GL_DeleteContext"];
		SDL_GL_MakeCurrent = (PFNSDLGLMAKECURRENT)libSDL2["SDL_GL_MakeCurrent"];

		window = nullptr;
		//renderer = nullptr;
		context = nullptr;
	}

	SDL2Window::SDL2Window():
		m_mouse_wheel(0)
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
		m_private->SDL_SetWindowSize(m_private->window, size.width, size.height);
		// TODO: update minimum size here
	}

	void SDL2Window::Redraw()
	{
		// FIXME: NYI
		// TODO: Not necessary, remove and use Invoke
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

				Keystroke::Type type = (event.type == SDL_KEYDOWN? Keystroke::KeyPress: Keystroke::KeyRelease);
				uint32_t keycode = m_private->SDL_GetKeyFromScancode(sdl_scancode);
				if (keycode <= 32 || keycode >= 0xFFFE || type != Keystroke::KeyPress) keycode = 0;
				if (keycode > 0) type |= Keystroke::Unicode;

				LOG(Error, "type = " << (event.type == SDL_KEYDOWN? "press": "release") << ", scancode = " << scancode << ", char = " << (int)keycode);

				Keystroke stroke(type, scancode, (char16_t)keycode);
				if (m_on_input) m_on_input(stroke);
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				Keystroke stroke(Keystroke::MouseMove, TK_MOUSE_MOVE, event.motion.x, event.motion.y, 0);
				if (m_on_input) m_on_input(stroke);
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
				if (m_on_input) m_on_input(stroke);
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				m_mouse_wheel += event.wheel.y;
				Keystroke stroke(Keystroke::MouseScroll, TK_MOUSE_SCROLL, 0, 0, m_mouse_wheel);
				if (m_on_input) m_on_input(stroke);
			}
			else if (event.type == SDL_WINDOWEVENT_EXPOSED)
			{
				// if (this->m_on_redraw()) m_private->SDL_GL_SwapWindow(m_private->window);
			}
			//else if (event.type == SDL_TEXTEDITING)
			//{
			//	LOG(Error, "Text editing: \"" << event.edit.text << "\"");
			//}
			//else if (event.type == SDL_TEXTINPUT)
			//{
			//	LOG(Error, "Text input: \"" << event.text.text << "\"");
			//}
			else if (event.type == SDL_USEREVENT)
			{
				auto task = static_cast<std::packaged_task<void()>*>(event.user.data1);
				(*task)();
				delete task;
			}
		}

		return false;
	}

	void SDL2Window::SetResizeable(bool resizeable)
	{
		// FIXME: NYI
		// TODO: update minimum size here
		// XXX: SDL2 does not seem to allow changing resizeability without window reconstruction
	}

	void SDL2Window::ThreadFunction()
	{
		LOG(Error, "Entering SDL2 window thread function");

		while (m_proceed)
		{
			if (!PumpEvents()) Sleep(0);//usleep(500);
		}

		// There was redraw barrier notify call

		LOG(Error, "Leaving SDL2 window thread function");
	}

	bool SDL2Window::Construct()
	{
		if (m_private->SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			LOG(Fatal, "SDL_Init failed: " << m_private->SDL_GetError());
			return false;
		}

		m_private->window = m_private->SDL_CreateWindow("BearLibTerminal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, SDL_WINDOW_HIDDEN|SDL_WINDOW_OPENGL);
		if (m_private->window == nullptr)
		{
			LOG(Fatal, "SDL_CreateWindow failed: " << m_private->SDL_GetError());
			Destroy();
			return false;
		}

		/*
		m_private->renderer = m_private->SDL_CreateRenderer(m_private->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
		if (m_private->renderer == nullptr)
		{
			LOG(Fatal, "SDL_CreateRenderer failed: " << m_private->SDL_GetError());
			Destroy();
			return false;
		}
		/*/
		m_private->context = m_private->SDL_GL_CreateContext(m_private->window);
		if (m_private->window == nullptr)
		{
			LOG(Fatal, "SDL_GL_CreateContext failed: " << m_private->SDL_GetError());
			Destroy();
			return false;
		}
		//*/

		// TODO: renderer info

		LOG(Error, "SDL2 Construction succeeded");

		m_proceed = true;

		return true;
	}

	void SDL2Window::Destroy()
	{
		/*
		if (m_private->renderer != nullptr)
		{
			m_private->SDL_DestroyRenderer(m_private->renderer);
			m_private->renderer = nullptr;
		}
		/*/
		if (m_private->context != nullptr && m_private->window != nullptr)
		{
			m_private->SDL_GL_MakeCurrent(m_private->window, nullptr);
			m_private->SDL_GL_DeleteContext(m_private->context);
			m_private->context = nullptr;
		}
		//*/

		if (m_private->window != nullptr)
		{
			m_private->SDL_DestroyWindow(m_private->window);
			m_private->window = nullptr;
		}

		m_private->SDL_Quit();
	}
}
#endif
