/*
* BearLibTerminal
* Copyright (C) 2013-2014 Cfyz
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
#include <wchar.h>

/*
 * Keyboard scancodes
 */
#define TK_LBUTTON			0x01
#define TK_RBUTTON			0x02
#define TK_MBUTTON			0x07
#define TK_CLOSE			0x03 /* Same code as VK_CANCEL from winuser.h */
#define TK_BACK				0x08 /* Backspace */
#define TK_BACKSPACE		0x08 /* Backspace (alias) */
#define TK_TAB				0x09
#define TK_RETURN			0x0D /* Enter */
#define TK_SHIFT			0x10
#define TK_CONTROL			0x11
#define TK_PAUSE			0x13 /* Pause/Break */
#define TK_ESCAPE			0x1B
#define TK_SPACE			0x20
#define TK_PRIOR			0x21 /* Page Up */
#define TK_NEXT				0x22 /* Page Down */
#define TK_END				0x23
#define TK_HOME				0x24
#define TK_LEFT				0x25 /* Left arrow */
#define TK_UP				0x26 /* Up arrow */
#define TK_RIGHT			0x27 /* Right arrow */
#define TK_DOWN				0x28 /* Down arrow */
#define TK_INSERT			0x2D
#define TK_DELETE			0x2E
#define TK_0				0x30
#define TK_1				0x31
#define TK_2				0x32
#define TK_3				0x33
#define TK_4				0x34
#define TK_5				0x35
#define TK_6				0x36
#define TK_7				0x37
#define TK_8				0x38
#define TK_9				0x39
#define TK_A				0x41
#define TK_B				0x42
#define TK_C				0x43
#define TK_D				0x44
#define TK_E				0x45
#define TK_F				0x46
#define TK_G				0x47
#define TK_H				0x48
#define TK_I				0x49
#define TK_J				0x4A
#define TK_K				0x4B
#define TK_L				0x4C
#define TK_M				0x4D
#define TK_N				0x4E
#define TK_O				0x4F
#define TK_P				0x50
#define TK_Q				0x51
#define TK_R				0x52
#define TK_S				0x53
#define TK_T				0x54
#define TK_U				0x55
#define TK_V				0x56
#define TK_W				0x57
#define TK_X				0x58
#define TK_Y				0x59
#define TK_Z				0x5A
#define TK_GRAVE			0xC0 /*  `  */
#define TK_MINUS			0xBD /*  -  */
#define TK_EQUALS			0xBB /*  =  */
#define TK_BACKSLASH		0xDC /*  \  */
#define TK_LBRACKET			0xDB /*  [  */
#define TK_RBRACKET			0xDD /*  ]  */
#define TK_SEMICOLON		0xBA /*  ;  */
#define TK_APOSTROPHE		0xDE /*  '  */
#define TK_COMMA			0xBC /*  ,  */
#define TK_PERIOD			0xBE /*  .  */
#define TK_SLASH			0xBF /*  /  */
#define TK_NUMPAD0			0x60
#define TK_NUMPAD1			0x61
#define TK_NUMPAD2			0x62
#define TK_NUMPAD3			0x63
#define TK_NUMPAD4			0x64
#define TK_NUMPAD5			0x65
#define TK_NUMPAD6			0x66
#define TK_NUMPAD7			0x67
#define TK_NUMPAD8			0x68
#define TK_NUMPAD9			0x69
#define TK_MULTIPLY			0x6A /* '*' on numpad */
#define TK_ADD				0x6B /* '+' on numpad */
#define TK_SUBTRACT			0x6D /* '-' on numpad */
#define TK_DECIMAL			0x6E /* '.' on numpad */
#define TK_DIVIDE			0x6F /* '/' on numpad */
#define TK_F1				0x70
#define TK_F2				0x71
#define TK_F3				0x72
#define TK_F4				0x73
#define TK_F5				0x74
#define TK_F6				0x75
#define TK_F7				0x76
#define TK_F8				0x77
#define TK_F9				0x78
#define TK_F10				0x79
#define TK_F11				0x7A
#define TK_F12				0x7B

/*
 * If key was released instead of pressed, it's code will be OR'ed with
 * VK_FLAG_RELEASED constant, i. e. terminal_read() will return:
 * a) 0x41 for pressed 'A'
 * b) 0x41|VK_FLAG_RELEASED (0x141) for released 'A'
 *
 * NOTE: you have to set keyrelease flag in "input.events" option for
 * terminal to generate these input events. It is not set by default.
 */
#define TK_FLAG_RELEASED	0x100

/*
 * Specific input events
 */
#define TK_MOUSE_MOVE       0xD3 /* Mouse movement event */
#define TK_MOUSE_SCROLL     0xD4 /* Mouse wheel scroll event */
#define TK_WINDOW_RESIZE    0xDF /* Window resize event */

/*
 * Virtual key-codes for internal terminal states/variables. These can be
 * accessed via terminal_state() function.
 */
#define TK_MOUSE_X          0xD0 /* Mouse cursor position in cells */
#define TK_MOUSE_Y          0xD1
#define TK_MOUSE_PIXEL_X    0xD5 /* Mouse cursor position in pixels */
#define TK_MOUSE_PIXEL_Y    0xD6
#define TK_MOUSE_WHEEL      0xD2 /* Mouse wheel counter (absolute value) */
#define TK_CELL_WIDTH       0xD7 /* Character cell size in pixels */
#define TK_CELL_HEIGHT      0xD8
#define TK_WIDTH            0xD9 /* Terminal window size in cells */
#define TK_HEIGHT           0xDA
#define TK_COMPOSITION      0xC1 /* Current composition state */
#define TK_COLOR			0xC2 /* Current foregroung color */
#define TK_BKCOLOR			0xC3 /* Current background color */
#define TK_LAYER			0xC4 /* Current layer */

/*
 * Composition option. If turned on it allows for placing several tiles in one cell.
 */
#define TK_COMPOSITION_OFF    0
#define TK_COMPOSITION_ON     1

/*
 * Input result codes for terminal_read, terminal_read_char and terminal_read_str.
 */
#define TK_INPUT_NONE         0
#define TK_INPUT_CANCELLED   -1
//#define TK_INPUT_CALL_AGAIN  -2

/*
 * Extended reading flags
 */
#define TK_READ_CHAR          1 /* Read an Unicode character, not a virtual key code */
#define TK_READ_NOREMOVE      2 /* Do not remove the event from input queue */
#define TK_READ_NOBLOCK       4 /* Do not block execution if input is not ready */

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
TERMINAL_API int terminal_print8(int x, int y, const int8_t* s);
TERMINAL_API int terminal_print16(int x, int y, const int16_t* s);
TERMINAL_API int terminal_print32(int x, int y, const int32_t* s);
TERMINAL_API int terminal_has_input();
TERMINAL_API int terminal_state(int code);
TERMINAL_API int terminal_read();
TERMINAL_API int terminal_read_ext(int flags);
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
#if !defined(__SIZEOF_WCHAR_T__)
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
 * terminal_[w]xxxx -> terminal_xxxx{8|16|32}
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

static inline int terminal_read_str(int x, int y, char* buffer, int max)
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
 * b) printing functions are heavy by themselves.
 *
 * NOTE: using static termporary buffer is okay because terminal API is not
 * required to be multiple-thread safe by design.
 */

#if !defined(TERMINAL_FORMAT_BUFFER_SIZE)
#define TERMINAL_FORMAT_BUFFER_SIZE 1024
#endif

#define TERMINAL_FORMATTED_VA(type, name, sign, func, call_sign)\
	static inline int terminal_v##name##f sign\
	{\
		static type buffer[TERMINAL_FORMAT_BUFFER_SIZE] = {0};\
		int rc = 0;\
		if (s == NULL) return 0;\
		rc = func(buffer, TERMINAL_FORMAT_BUFFER_SIZE-1, s, args);\
		if (rc > 0) rc = terminal_##name call_sign;\
		return rc;\
	}

TERMINAL_FORMATTED_VA(char, set, (const char* s, va_list args), vsnprintf, (buffer))
TERMINAL_FORMATTED_VA(wchar_t, wset, (const wchar_t* s, va_list args), vswprintf, (buffer))
TERMINAL_FORMATTED_VA(char, print, (int x, int y, const char* s, va_list args), vsnprintf, (x, y, buffer))
TERMINAL_FORMATTED_VA(wchar_t, wprint, (int x, int y, const wchar_t* s, va_list args), vswprintf, (x, y, buffer))

#define TERMINAL_FORMATTED(outer, inner)\
	static inline int terminal_##outer\
	{\
		va_list args;\
		int rc = 0;\
		va_start(args, s);\
		rc = terminal_v##inner;\
		va_end(args);\
		return rc;\
	}

TERMINAL_FORMATTED(setf(const char* s, ...), setf(s, args))
TERMINAL_FORMATTED(wsetf(const wchar_t* s, ...), wsetf(s, args))
TERMINAL_FORMATTED(printf(int x, int y, const char* s, ...), printf(x, y, s, args))
TERMINAL_FORMATTED(wprintf(int x, int y, const wchar_t* s, ...), wprintf(x, y, s, args))

#ifdef __cplusplus
/*
 * C++ supports function overloading, should take advantage of it.
 */

static inline int terminal_set(const wchar_t* s)
{
	return terminal_wset(s);
}

static inline void terminal_color(const char* name)
{
	terminal_color(color_from_name(name));
}

static inline void terminal_color(const wchar_t* name)
{
	terminal_color(color_from_wname(name));
}

static inline void terminal_bkcolor(const char* name)
{
	terminal_bkcolor(color_from_name(name));
}

static inline void terminal_bkcolor(const wchar_t* name)
{
	terminal_bkcolor(color_from_wname(name));
}

TERMINAL_FORMATTED(setf(const wchar_t* s, ...), wsetf(s, args))

static inline int terminal_print(int x, int y, const wchar_t* s)
{
	return terminal_wprint(x, y, s);
}

TERMINAL_FORMATTED(printf(int x, int y, const wchar_t* s, ...), wprintf(x, y, s, args))

static inline int terminal_read_str(int x, int y, wchar_t* buffer, int max)
{
	return terminal_read_wstr(x, y, buffer, max);
}

static inline color_t color_from_name(const wchar_t* name)
{
	return color_from_wname(name);
}
#endif

/*
 * Color routines
 */
static inline color_t color_from_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return ((color_t)a << 24) | (r << 16) | (g << 8) | b;
}

/*
 * Other functional sugar
 */
static inline int terminal_check(int code)
{
	return terminal_state(code) == 1;
}

/*
 * WinMain entry point handling macro. This allows easier entry point definition.
 * The macro will expand to proper WinMain stub regardless of header include order.
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
