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

#ifndef BEARLIBTERMINAL_H
#define BEARLIBTERMINAL_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/*
 * Keyboard scancodes
 */
#define VK_LBUTTON			0x01
#define VK_RBUTTON			0x02
#define VK_CLOSE			0x03 /* Same code as VK_CANCEL from winuser.h */
#define VK_BACK				0x08 /* Backspace */
#define VK_BACKSPACE		0x08 /* Backspace (alias) */
#define VK_TAB				0x09
#define VK_RETURN			0x0D /* Enter */
#define VK_SHIFT			0x10
#define VK_CONTROL			0x11
#define VK_PAUSE			0x13 /* Pause/Break */
#define VK_ESCAPE			0x1B
#define VK_SPACE			0x20
#define VK_PRIOR			0x21 /* Page Up */
#define VK_NEXT				0x22 /* Page Down */
#define VK_END				0x23
#define VK_HOME				0x24
#define VK_LEFT				0x25 /* Left arrow */
#define VK_UP				0x26 /* Up arrow */
#define VK_RIGHT			0x27 /* Right arrow */
#define VK_DOWN				0x28 /* Down arrow */
#define VK_INSERT			0x2D
#define VK_DELETE			0x2E
#define VK_0				0x30
#define VK_1				0x31
#define VK_2				0x32
#define VK_3				0x33
#define VK_4				0x34
#define VK_5				0x35
#define VK_6				0x36
#define VK_7				0x37
#define VK_8				0x38
#define VK_9				0x39
#define VK_A				0x41
#define VK_B				0x42
#define VK_C				0x43
#define VK_D				0x44
#define VK_E				0x45
#define VK_F				0x46
#define VK_G				0x47
#define VK_H				0x48
#define VK_I				0x49
#define VK_J				0x4A
#define VK_K				0x4B
#define VK_L				0x4C
#define VK_M				0x4D
#define VK_N				0x4E
#define VK_O				0x4F
#define VK_P				0x50
#define VK_Q				0x51
#define VK_R				0x52
#define VK_S				0x53
#define VK_T				0x54
#define VK_U				0x55
#define VK_V				0x56
#define VK_W				0x57
#define VK_X				0x58
#define VK_Y				0x59
#define VK_Z				0x5A
#define VK_GRAVE			0xC0 /*  `  */
#define VK_MINUS			0xBD /*  -  */
#define VK_EQUALS			0xBB /*  =  */
#define VK_BACKSLASH		0xDC /*  \  */
#define VK_LBRACKET			0xDB /*  [  */
#define VK_RBRACKET			0xDD /*  ]  */
#define VK_SEMICOLON		0xBA /*  ;  */
#define VK_APOSTROPHE		0xDE /*  '  */
#define VK_COMMA			0xBC /*  ,  */
#define VK_PERIOD			0xBE /*  .  */
#define VK_SLASH			0xBF /*  /  */
#define VK_NUMPAD0			0x60
#define VK_NUMPAD1			0x61
#define VK_NUMPAD2			0x62
#define VK_NUMPAD3			0x63
#define VK_NUMPAD4			0x64
#define VK_NUMPAD5			0x65
#define VK_NUMPAD6			0x66
#define VK_NUMPAD7			0x67
#define VK_NUMPAD8			0x68
#define VK_NUMPAD9			0x69
#define VK_MULTIPLY			0x6A /* '*' on numpad */
#define VK_ADD				0x6B /* '+' on numpad */
#define VK_SUBTRACT			0x6D /* '-' on numpad */
#define VK_DECIMAL			0x6E /* '.' on numpad */
#define VK_DIVIDE			0x6F /* '/' on numpad */
#define VK_F1				0x70
#define VK_F2				0x71
#define VK_F3				0x72
#define VK_F4				0x73
#define VK_F5				0x74
#define VK_F6				0x75
#define VK_F7				0x76
#define VK_F8				0x77
#define VK_F9				0x78
#define VK_F10				0x79
#define VK_F11				0x7A
#define VK_F12				0x7B

/*
 * If key was released instead of pressed, it's code will be OR'ed with
 * VK_FLAG_RELEASED constant, i. e. terminal_read() will return:
 * a) 0x41 for pressed 'A'
 * b) 0x41|VK_FLAG_RELEASED for released 'A' (0x141)
 *
 * NOTE: you have to set keyrelease flag in "input.events" option for
 * terminal to generate these input events. It is not set by default.
 */
#define VK_FLAG_RELEASED	0x100

/*
 * Terminal-specific input events
 */
#define VK_MOUSE_MOVE		0xD3 /* Mouse movement event */
#define VK_MOUSE_SCROLL		0xD4 /* Mouse wheel scroll event */

/*
 * Virtual key-codes for internal terminal states/variables. These can be
 * accessed via terminal_state() function.
 */
#define VK_MOUSE_X			0xD0 /* Mouse cursor position in characters/cells */
#define VK_MOUSE_Y			0xD1
#define VK_MOUSE_PIXEL_X	0xD5 /* Mouse cursor position in pixels */
#define VK_MOUSE_PIXEL_Y	0xD6
#define VK_MOUSE_WHEEL		0xD2 /* Mouse wheel counter (absolute value) */
#define VK_CELL_WIDTH		0xD7 /* Character cell size in pixels */
#define VK_CELL_HEIGHT		0xD8
#define VK_WINDOW_WIDTH		0xD9 /* Terminal window size in characters/cells */
#define VK_WINDOW_HEIGHT	0xDA

/*
 * Composition option. If turned on it allows for placing several tiles in one cell.
 */
#define TERMINAL_COMPOSITION_OFF    0
#define TERMINAL_COMPOSITION_ON     1

/*
 * Input result codes for terminal_read, terminal_read_char and terminal_read_str.
 */
#define TERMINAL_INPUT_CALL_AGAIN  -1
#define TERMINAL_INPUT_CANCELLED   -2

/*
 * Terminal use unsigned 32-bit value for color representation in ARGB order (0xAARRGGBB), e. g.
 * a) solid red is 0xFFFF0000
 * b) half-transparent green is 0x8000FF00
 */
typedef uint32_t color_t;

#if defined(_WIN32)
#  if defined(BEARLIBTERMINAL_BUILDING_LIBRARY)
#    define TERMINAL_API __declspec(dllexport)
#  else
#    define TERMINAL_API __declspec(dllimport)
#  endif
#else
#  if defined(BEARLIBTERMINAL_BUILDING_LIBRARY) && __GNUC__ >= 4
#    define TERMINAL_API __attribute__ ((visibility ("default")))
#  else
#    define TERMINAL_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

TERMINAL_API int terminal_open();
TERMINAL_API void terminal_close();
TERMINAL_API int terminal_set8(const int8_t* value);
TERMINAL_API int terminal_set16(const int16_t* value);
TERMINAL_API int terminal_set32(const int32_t* value);
TERMINAL_API void terminal_refresh();
TERMINAL_API void terminal_clear();
TERMINAL_API void terminal_clear_area(int x, int y, int w, int h);
TERMINAL_API void terminal_layer(int index);
TERMINAL_API void terminal_color(color_t color);
TERMINAL_API void terminal_bkcolor(color_t color);
TERMINAL_API void terminal_composition(int mode);
TERMINAL_API void terminal_put(int x, int y, int code);
TERMINAL_API void terminal_put_ext(int x, int y, int dx, int dy, int code, color_t* corners);
TERMINAL_API void terminal_custom(int mode);
TERMINAL_API int terminal_print8(int x, int y, const int8_t* s);
TERMINAL_API int terminal_print16(int x, int y, const int16_t* s);
TERMINAL_API int terminal_print32(int x, int y, const int32_t* s);
TERMINAL_API int terminal_has_input();
TERMINAL_API int terminal_state(int code);
TERMINAL_API int terminal_read();
TERMINAL_API int terminal_read_char();
TERMINAL_API int terminal_read_str8(int x, int y, int8_t* buffer, int max);
TERMINAL_API int terminal_read_str16(int x, int y, int16_t* buffer, int max);
TERMINAL_API int terminal_read_str32(int x, int y, int32_t* buffer, int max);
TERMINAL_API color_t color_from_name8(const int8_t* name);
TERMINAL_API color_t color_from_name16(const int16_t* name);
TERMINAL_API color_t color_from_name32(const int32_t* name);

#ifdef __cplusplus
} /* End of extern "C" */
#endif

/*
 * Utility macro trick which allows macro-in-macro expansion
 */
#define TERMINAL_CAT(a, b) TERMINAL_PRIMITIVE_CAT(a, b)
#define TERMINAL_PRIMITIVE_CAT(a, b) a ## b

/*
 * wchar_t has different sized depending on platform. Furthermore, it's size
 * can be changed for GCC compiler.
 */
#if !defined(__GNUC__)
#  if defined(_WIN32)
#    define __SIZEOF_WCHAR_T__ 2
#  else
#    define __SIZEOF_WCHAR_T__ 4
#  endif
#endif

#if __SIZEOF_WCHAR_T__ == 2
#define TERMINAL_WCHAR_SUFFIX 16
#define TERMINAL_WCHAR_TYPE int16_t
#else // 4
#define TERMINAL_WCHAR_SUFFIX 32
#define TERMINAL_WCHAR_TYPE int32_t
#endif

/*
 * This set of inline functions define basic name substitution + type cast:
 * terminal_[w]xxx -> terminal_xxx{8|16|32}
 */

static inline int terminal_set(const char* value)
{
	return terminal_set8((const int8_t*)value);
}

static inline int terminal_wset(const wchar_t* value)
{
	return TERMINAL_CAT(terminal_set, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)value);
}

static inline int terminal_print(int x, int y, const char* s)
{
	return terminal_print8(x, y, (const int8_t*)s);
}

static inline int terminal_wprint(int x, int y, const wchar_t* s)
{
	return TERMINAL_CAT(terminal_print, TERMINAL_WCHAR_SUFFIX)(x, y, (const TERMINAL_WCHAR_TYPE*)s);
}

static inline int terminal_read_str(int x, int y, wchar_t* buffer, int max)
{
	return terminal_read_str8(x, y, (int8_t*)buffer, max);
}

static inline int terminal_read_wstr(int x, int y, wchar_t* buffer, int max)
{
	return TERMINAL_CAT(terminal_read_str, TERMINAL_WCHAR_SUFFIX)(x, y, (TERMINAL_WCHAR_TYPE*)buffer, max);
}

static inline color_t color_from_name(const char* name)
{
	return color_from_name8((const int8_t*)name);
}

static inline color_t color_from_wname(const wchar_t* name)
{
	return TERMINAL_CAT(color_from_name, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)name);
}

/*
 * These inline functions provide formatted versions for textual API:
 * terminal_[w]setf and terminal_[w]printf
 *
 * NOTE: inlining these is okay because
 * a) huge temporary buffer is static and won't mess the stack;
 * b) base functions are heavy by themselves.
 *
 * NOTE: using static termporary buffer is okay because terminal API is not
 * required to be multiple-thread safe by design.
 */

#define TERMINAL_DEFINE_FORMATTED(type, func, proto, impl)\
	static inline int proto\
	{\
		const size_t buffer_size = 1024;\
		static type buffer[buffer_size] = {0};\
		va_list argptr;\
		int rc = 0;\
		if (s == NULL) return 0;\
		va_start(argptr, s);\
		rc = func(buffer, buffer_size-1, s, argptr);\
		if (rc >= 0) rc = impl;\
		va_end(argptr);\
		return rc;\
	}

#define TERMINAL_DEFINE_FORMATTED_ANSI(proto, impl) TERMINAL_DEFINE_FORMATTED(char, vsnprintf, proto, impl)
#define TERMINAL_DEFINE_FORMATTED_WIDE(proto, impl) TERMINAL_DEFINE_FORMATTED(wchar_t, vswprintf, proto, impl)
TERMINAL_DEFINE_FORMATTED_ANSI(terminal_setf(const char* s, ...), terminal_set(buffer))
TERMINAL_DEFINE_FORMATTED_ANSI(terminal_printf(int x, int y, const char* s, ...), terminal_print(x, y, buffer))
TERMINAL_DEFINE_FORMATTED_WIDE(terminal_wsetf(const wchar_t* s, ...), terminal_wset(buffer))
TERMINAL_DEFINE_FORMATTED_WIDE(terminal_wprintf(int x, int y, const wchar_t* s, ...), terminal_wprint(x, y, buffer))

/*
 * WinMain entry point handling macro. This allows easier entry point definition.
 */
#if defined(_WIN32)

/*
 * WinMain probe macro. It will expand to either X or X_WINDOWS_ depending on
 * Windows.h header inclusion.
 */
#define TERMINAL_TAKE_CARE_OF_WINMAIN TERMINAL_WINMAIN_PROBE_IMPL(_WINDOWS_)
#define TERMINAL_WINMAIN_PROBE_IMPL(DEF) TERMINAL_PRIMITIVE_CAT(TERMINAL_WINMAIN_IMPL, DEF)

/*
 * Trivial no-arguments WinMain implementation. It just calls main.
 */
#define TERMINAL_WINMAIN_IMPL_BASE(INSTANCE_T, STRING_T)\
	extern "C" int main();\
	extern "C" int __stdcall WinMain(INSTANCE_T hInstance, INSTANCE_T hPrevInstance, STRING_T lpCmdLine, int nCmdShow)\
	{\
		return main();\
	}

/*
 * Macro expands to empty string. Windows.h is included thus code MUST use
 * predefined types or else MSVC will complain.
 */
#define TERMINAL_WINMAIN_IMPL TERMINAL_WINMAIN_IMPL_BASE(HINSTANCE, LPSTR)

/*
 * Macro expands to macro name. Windows.h wasn't included, so WinMain will be
 * defined with fundamental types (enough for linker to find it).
 */
#define TERMINAL_WINMAIN_IMPL_WINDOWS_ TERMINAL_WINMAIN_IMPL_BASE(void*, char*)

#else

/*
 * Only Windows has WinMain but macro still must be defined for cross-platform
 * applications.
 */
#define TERMINAL_TAKE_CARE_OF_WINMAIN

#endif

#endif // BEARLIBTERMINAL_H
