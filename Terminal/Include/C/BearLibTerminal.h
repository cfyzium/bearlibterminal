/*
* BearLibTerminal
* Copyright (C) 2013-2017 Cfyz
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

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 1)
#pragma GCC diagnostic ignored "-Wformat-nonliteral" /* False-positive when wrapping vsnprintf. */
#endif /* __GNUC__ >= 4.1 */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#if defined(__cplusplus)
#include <sstream>
#endif

/*
 * Keyboard scancodes for events/states
 */
#define TK_A                0x04
#define TK_B                0x05
#define TK_C                0x06
#define TK_D                0x07
#define TK_E                0x08
#define TK_F                0x09
#define TK_G                0x0A
#define TK_H                0x0B
#define TK_I                0x0C
#define TK_J                0x0D
#define TK_K                0x0E
#define TK_L                0x0F
#define TK_M                0x10
#define TK_N                0x11
#define TK_O                0x12
#define TK_P                0x13
#define TK_Q                0x14
#define TK_R                0x15
#define TK_S                0x16
#define TK_T                0x17
#define TK_U                0x18
#define TK_V                0x19
#define TK_W                0x1A
#define TK_X                0x1B
#define TK_Y                0x1C
#define TK_Z                0x1D
#define TK_1                0x1E
#define TK_2                0x1F
#define TK_3                0x20
#define TK_4                0x21
#define TK_5                0x22
#define TK_6                0x23
#define TK_7                0x24
#define TK_8                0x25
#define TK_9                0x26
#define TK_0                0x27
#define TK_RETURN           0x28
#define TK_ENTER            0x28
#define TK_ESCAPE           0x29
#define TK_BACKSPACE        0x2A
#define TK_TAB              0x2B
#define TK_SPACE            0x2C
#define TK_MINUS            0x2D /*  -  */
#define TK_EQUALS           0x2E /*  =  */
#define TK_LBRACKET         0x2F /*  [  */
#define TK_RBRACKET         0x30 /*  ]  */
#define TK_BACKSLASH        0x31 /*  \  */
#define TK_SEMICOLON        0x33 /*  ;  */
#define TK_APOSTROPHE       0x34 /*  '  */
#define TK_GRAVE            0x35 /*  `  */
#define TK_COMMA            0x36 /*  ,  */
#define TK_PERIOD           0x37 /*  .  */
#define TK_SLASH            0x38 /*  /  */
#define TK_F1               0x3A
#define TK_F2               0x3B
#define TK_F3               0x3C
#define TK_F4               0x3D
#define TK_F5               0x3E
#define TK_F6               0x3F
#define TK_F7               0x40
#define TK_F8               0x41
#define TK_F9               0x42
#define TK_F10              0x43
#define TK_F11              0x44
#define TK_F12              0x45
#define TK_PAUSE            0x48 /* Pause/Break */
#define TK_INSERT           0x49
#define TK_HOME             0x4A
#define TK_PAGEUP           0x4B
#define TK_DELETE           0x4C
#define TK_END              0x4D
#define TK_PAGEDOWN         0x4E
#define TK_RIGHT            0x4F /* Right arrow */
#define TK_LEFT             0x50 /* Left arrow */
#define TK_DOWN             0x51 /* Down arrow */
#define TK_UP               0x52 /* Up arrow */
#define TK_KP_DIVIDE        0x54 /* '/' on numpad */
#define TK_KP_MULTIPLY      0x55 /* '*' on numpad */
#define TK_KP_MINUS         0x56 /* '-' on numpad */
#define TK_KP_PLUS          0x57 /* '+' on numpad */
#define TK_KP_ENTER         0x58
#define TK_KP_1             0x59
#define TK_KP_2             0x5A
#define TK_KP_3             0x5B
#define TK_KP_4             0x5C
#define TK_KP_5             0x5D
#define TK_KP_6             0x5E
#define TK_KP_7             0x5F
#define TK_KP_8             0x60
#define TK_KP_9             0x61
#define TK_KP_0             0x62
#define TK_KP_PERIOD        0x63 /* '.' on numpad */
#define TK_SHIFT            0x70
#define TK_CONTROL          0x71
#define TK_ALT              0x72

/*
 * Mouse events/states
 */
#define TK_MOUSE_LEFT       0x80 /* Buttons */
#define TK_MOUSE_RIGHT      0x81
#define TK_MOUSE_MIDDLE     0x82
#define TK_MOUSE_X1         0x83
#define TK_MOUSE_X2         0x84
#define TK_MOUSE_MOVE       0x85 /* Movement event */
#define TK_MOUSE_SCROLL     0x86 /* Mouse scroll event */
#define TK_MOUSE_X          0x87 /* Cusor position in cells */
#define TK_MOUSE_Y          0x88
#define TK_MOUSE_PIXEL_X    0x89 /* Cursor position in pixels */
#define TK_MOUSE_PIXEL_Y    0x8A
#define TK_MOUSE_WHEEL      0x8B /* Scroll direction and amount */
#define TK_MOUSE_CLICKS     0x8C /* Number of consecutive clicks */

/*
 * If key was released instead of pressed, it's code will be OR'ed with TK_KEY_RELEASED:
 * a) pressed 'A': 0x04
 * b) released 'A': 0x04|VK_KEY_RELEASED = 0x104
 */
#define TK_KEY_RELEASED     0x100

/*
 * Virtual key-codes for internal terminal states/variables.
 * These can be accessed via terminal_state function.
 */
#define TK_WIDTH            0xC0 /* Terminal window size in cells */
#define TK_HEIGHT           0xC1
#define TK_CELL_WIDTH       0xC2 /* Character cell size in pixels */
#define TK_CELL_HEIGHT      0xC3
#define TK_COLOR            0xC4 /* Current foregroung color */
#define TK_BKCOLOR          0xC5 /* Current background color */
#define TK_LAYER            0xC6 /* Current layer */
#define TK_COMPOSITION      0xC7 /* Current composition state */
#define TK_CHAR             0xC8 /* Translated ANSI code of last produced character */
#define TK_WCHAR            0xC9 /* Unicode codepoint of last produced character */
#define TK_EVENT            0xCA /* Last dequeued event */
#define TK_FULLSCREEN       0xCB /* Fullscreen state */

/*
 * Other events
 */
#define TK_CLOSE            0xE0
#define TK_RESIZED          0xE1

/*
 * Generic mode enum.
 * Right now it is used for composition option only.
 */
#define TK_OFF                 0
#define TK_ON                  1

/*
 * Input result codes for terminal_read function.
 */
#define TK_INPUT_NONE          0
#define TK_INPUT_CANCELLED    -1

/*
 * Text printing alignment.
 */
#define TK_ALIGN_DEFAULT       0
#define TK_ALIGN_LEFT          1
#define TK_ALIGN_RIGHT         2
#define TK_ALIGN_CENTER        3
#define TK_ALIGN_TOP           4
#define TK_ALIGN_BOTTOM        8
#define TK_ALIGN_MIDDLE       12

/*
 * Terminal uses unsigned 32-bit value for color representation in ARGB order (0xAARRGGBB), e. g.
 * a) solid red is 0xFFFF0000
 * b) half-transparent green is 0x8000FF00
 */
typedef uint32_t color_t;

typedef struct dimensions_t_
{
	int width;
	int height;
}
dimensions_t;

#if defined(BEARLIBTERMINAL_STATIC_BUILD)
#  define TERMINAL_API
#elif defined(_WIN32)
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
TERMINAL_API void terminal_crop(int x, int y, int w, int h);
TERMINAL_API void terminal_layer(int index);
TERMINAL_API void terminal_color(color_t color);
TERMINAL_API void terminal_bkcolor(color_t color);
TERMINAL_API void terminal_composition(int mode);
TERMINAL_API void terminal_font8(const int8_t* name);
TERMINAL_API void terminal_font16(const int16_t* name);
TERMINAL_API void terminal_font32(const int32_t* name);
TERMINAL_API void terminal_put(int x, int y, int code);
TERMINAL_API void terminal_put_ext(int x, int y, int dx, int dy, int code, color_t* corners);
TERMINAL_API int terminal_pick(int x, int y, int index);
TERMINAL_API color_t terminal_pick_color(int x, int y, int index);
TERMINAL_API color_t terminal_pick_bkcolor(int x, int y);
TERMINAL_API void terminal_print_ext8(int x, int y, int w, int h, int align, const int8_t* s, int* out_w, int* out_h);
TERMINAL_API void terminal_print_ext16(int x, int y, int w, int h, int align, const int16_t* s, int* out_w, int* out_h);
TERMINAL_API void terminal_print_ext32(int x, int y, int w, int h, int align, const int32_t* s, int* out_w, int* out_h);
TERMINAL_API void terminal_measure_ext8(int w, int h, const int8_t* s, int* out_w, int* out_h);
TERMINAL_API void terminal_measure_ext16(int w, int h, const int16_t* s, int* out_w, int* out_h);
TERMINAL_API void terminal_measure_ext32(int w, int h, const int32_t* s, int* out_w, int* out_h);
TERMINAL_API int terminal_has_input();
TERMINAL_API int terminal_state(int code);
TERMINAL_API int terminal_read();
TERMINAL_API int terminal_read_str8(int x, int y, int8_t* buffer, int max);
TERMINAL_API int terminal_read_str16(int x, int y, int16_t* buffer, int max);
TERMINAL_API int terminal_read_str32(int x, int y, int32_t* buffer, int max);
TERMINAL_API int terminal_peek();
TERMINAL_API void terminal_delay(int period);
TERMINAL_API const int8_t* terminal_get8(const int8_t* key, const int8_t* default_);
TERMINAL_API const int16_t* terminal_get16(const int16_t* key, const int16_t* default_);
TERMINAL_API const int32_t* terminal_get32(const int32_t* key, const int32_t* default_);
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

#if defined(__cplusplus)
#define TERMINAL_INLINE inline
#define TERMINAL_DEFAULT(value) = value
#else
#define TERMINAL_INLINE static inline
#define TERMINAL_DEFAULT(value)
#endif

/*
 * These functions provide inline string formatting support
 * for terminal_setf, terminal_printf, etc.
 *
 * Using static termporary buffer is okay because terminal API is not
 * required to be multiple-thread safe by design.
 */

#define TERMINAL_VSPRINTF_MAXIMUM_BUFFER_SIZE 65536

TERMINAL_INLINE const char* terminal_vsprintf(const char* s, va_list args)
{
	static int buffer_size = 512;
	static char* buffer = NULL;
	int rc = 0;

	if (!s)
		return NULL;
	else if (!buffer)
		buffer = (char*)malloc(buffer_size);

	while (1)
	{
		buffer[buffer_size-1] = '\0';
		rc = vsnprintf(buffer, buffer_size, s, args);
		if (rc >= buffer_size || buffer[buffer_size-1] != '\0')
		{
			if (buffer_size >= TERMINAL_VSPRINTF_MAXIMUM_BUFFER_SIZE)
				return NULL;

			buffer_size *= 2;
			buffer = (char*)realloc(buffer, buffer_size);
		}
		else
		{
			break;
		}
	}

	return rc >= 0? buffer: NULL;
}

TERMINAL_INLINE const wchar_t* terminal_vswprintf(const wchar_t* s, va_list args)
{
	static int buffer_size = 512;
	static wchar_t* buffer = NULL;
	int rc = 0;

	if (!s)
		return NULL;
	else if (!buffer)
		buffer = (wchar_t*)malloc(buffer_size * sizeof(wchar_t));

	while (1)
	{
		buffer[buffer_size-1] = L'\0';
#if defined(_WIN32)
		rc = _vsnwprintf(buffer, buffer_size, s, args);
#else
		rc = vswprintf(buffer, buffer_size, s, args);
#endif
		if (rc >= buffer_size || buffer[buffer_size-1] != L'\0')
		{
			if (buffer_size >= TERMINAL_VSPRINTF_MAXIMUM_BUFFER_SIZE)
				return NULL;

			buffer_size *= 2;
			buffer = (wchar_t*)realloc(buffer, buffer_size * sizeof(wchar_t));
		}
		else
		{
			break;
		}
	}

	return rc >= 0? buffer: NULL;
}

#define TERMINAL_FORMATTED_WRAP(type, call) \
	type ret; \
	va_list args; \
	va_start(args, s); \
	ret = call; \
	va_end(args); \
	return ret;

#define TERMINAL_FORMATTED_WRAP_V(call) \
	va_list args; \
	va_start(args, s); \
	call; \
	va_end(args);

/*
 * This set of inline functions define basic name substitution + type cast:
 * terminal_[w]xxxx -> terminal_xxxx{8|16|32}
 */

TERMINAL_INLINE int terminal_set(const char* s)
{
	return terminal_set8((const int8_t*)s);
}

TERMINAL_INLINE int terminal_setf(const char* s, ...)
{
	TERMINAL_FORMATTED_WRAP(int, terminal_set(terminal_vsprintf(s, args)))
}

TERMINAL_INLINE int terminal_wset(const wchar_t* s)
{
	return TERMINAL_CAT(terminal_set, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)s);
}

TERMINAL_INLINE int terminal_wsetf(const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(int, terminal_wset(terminal_vswprintf(s, args)))
}

TERMINAL_INLINE void terminal_font(const char* name)
{
	terminal_font8((const int8_t*)name);
}

TERMINAL_INLINE void terminal_wfont(const wchar_t* name)
{
	TERMINAL_CAT(terminal_font, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)name);
}

TERMINAL_INLINE dimensions_t terminal_print(int x, int y, const char* s)
{
	dimensions_t ret;
	terminal_print_ext8(x, y, 0, 0, TK_ALIGN_DEFAULT, (const int8_t*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_printf(int x, int y, const char* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_print(x, y, terminal_vsprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_wprint(int x, int y, const wchar_t* s)
{
	dimensions_t ret;
	TERMINAL_CAT(terminal_print_ext, TERMINAL_WCHAR_SUFFIX)(x, y, 0, 0, TK_ALIGN_DEFAULT, (const TERMINAL_WCHAR_TYPE*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_wprintf(int x, int y, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wprint(x, y, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_print_ext(int x, int y, int w, int h, int align, const char* s)
{
	dimensions_t ret;
	terminal_print_ext8(x, y, w, h, align, (const int8_t*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_printf_ext(int x, int y, int w, int h, int align, const char* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_print_ext(x, y, w, h, align, terminal_vsprintf(s, args)));
}

TERMINAL_INLINE dimensions_t terminal_wprint_ext(int x, int y, int w, int h, int align, const wchar_t* s)
{
	dimensions_t ret;
	TERMINAL_CAT(terminal_print_ext, TERMINAL_WCHAR_SUFFIX)(x, y, w, h, align, (const TERMINAL_WCHAR_TYPE*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_wprintf_ext(int x, int y, int w, int h, int align, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wprint_ext(x, y, w, h, align, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_measure(const char* s)
{
	dimensions_t ret;
	terminal_measure_ext8(0, 0, (const int8_t*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_measuref(const char* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_measure(terminal_vsprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_wmeasure(const wchar_t* s)
{
	dimensions_t ret;
	TERMINAL_CAT(terminal_measure_ext, TERMINAL_WCHAR_SUFFIX)(0, 0, (const TERMINAL_WCHAR_TYPE*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_wmeasuref(const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wmeasure(terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_measure_ext(int w, int h, const char* s)
{
	dimensions_t ret;
	terminal_measure_ext8(w, h, (const int8_t*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_measuref_ext(int w, int h, const char* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_measure_ext(w, h, terminal_vsprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_wmeasure_ext(int w, int h, const wchar_t* s)
{
	dimensions_t ret;
	TERMINAL_CAT(terminal_measure_ext, TERMINAL_WCHAR_SUFFIX)(w, h, (const TERMINAL_WCHAR_TYPE*)s, &ret.width, &ret.height);
	return ret;
}

TERMINAL_INLINE dimensions_t terminal_wmeasuref_ext(int w, int h, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wmeasure_ext(w, h, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE int terminal_read_str(int x, int y, char* buffer, int max)
{
	return terminal_read_str8(x, y, (int8_t*)buffer, max);
}

TERMINAL_INLINE int terminal_read_wstr(int x, int y, wchar_t* buffer, int max)
{
	return TERMINAL_CAT(terminal_read_str, TERMINAL_WCHAR_SUFFIX)(x, y, (TERMINAL_WCHAR_TYPE*)buffer, max);
}

TERMINAL_INLINE const char* terminal_get(const char* key, const char* default_ TERMINAL_DEFAULT((const char*)0))
{
	return (const char*)terminal_get8((const int8_t*)key, (const int8_t*)default_);
}

TERMINAL_INLINE const wchar_t* terminal_wget(const wchar_t* key, const wchar_t* default_ TERMINAL_DEFAULT((const wchar_t*)0))
{
	return (const wchar_t*)TERMINAL_CAT(terminal_get, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)key, (const TERMINAL_WCHAR_TYPE*)default_);
}

TERMINAL_INLINE color_t color_from_name(const char* name)
{
	return color_from_name8((const int8_t*)name);
}

TERMINAL_INLINE color_t color_from_wname(const wchar_t* name)
{
	return TERMINAL_CAT(color_from_name, TERMINAL_WCHAR_SUFFIX)((const TERMINAL_WCHAR_TYPE*)name);
}

#ifdef __cplusplus
/*
 * C++ supports function overloading, should take advantage of it.
 */

TERMINAL_INLINE int terminal_set(const wchar_t* s)
{
	return terminal_wset(s);
}

TERMINAL_INLINE int terminal_setf(const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(int, terminal_wset(terminal_vswprintf(s, args)));
}

TERMINAL_INLINE void terminal_color(const char* name)
{
	terminal_color(color_from_name(name));
}

TERMINAL_INLINE void terminal_color(const wchar_t* name)
{
	terminal_color(color_from_wname(name));
}

TERMINAL_INLINE void terminal_bkcolor(const char* name)
{
	terminal_bkcolor(color_from_name(name));
}

TERMINAL_INLINE void terminal_bkcolor(const wchar_t* name)
{
	terminal_bkcolor(color_from_wname(name));
}

TERMINAL_INLINE void terminal_font(const wchar_t* name)
{
	terminal_wfont(name);
}

TERMINAL_INLINE void terminal_put_ext(int x, int y, int dx, int dy, int code)
{
	terminal_put_ext(x, y, dx, dy, code, 0);
}

TERMINAL_INLINE dimensions_t terminal_print(int x, int y, const wchar_t* s)
{
	return terminal_wprint(x, y, s);
}

TERMINAL_INLINE dimensions_t terminal_printf(int x, int y, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wprint(x, y, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_print_ext(int x, int y, int w, int h, int align, const wchar_t* s)
{
	return terminal_wprint_ext(x, y, w, h, align, s);
}

TERMINAL_INLINE dimensions_t terminal_printf_ext(int x, int y, int w, int h, int align, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wprint_ext(x, y, w, h, align, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_measure(const wchar_t* s)
{
	return terminal_wmeasure(s);
}

TERMINAL_INLINE dimensions_t terminal_measuref(const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wmeasure(terminal_vswprintf(s, args)))
}

TERMINAL_INLINE dimensions_t terminal_measure_ext(int w, int h, const wchar_t* s)
{
	return terminal_wmeasure_ext(w, h, s);
}

TERMINAL_INLINE dimensions_t terminal_measuref_ext(int w, int h, const wchar_t* s, ...)
{
	TERMINAL_FORMATTED_WRAP(dimensions_t, terminal_wmeasure_ext(w, h, terminal_vswprintf(s, args)))
}

TERMINAL_INLINE int terminal_read_str(int x, int y, wchar_t* buffer, int max)
{
	return terminal_read_wstr(x, y, buffer, max);
}

TERMINAL_INLINE color_t color_from_name(const wchar_t* name)
{
	return color_from_wname(name);
}

TERMINAL_INLINE int terminal_pick(int x, int y)
{
	return terminal_pick(x, y, 0);
}

TERMINAL_INLINE color_t terminal_pick_color(int x, int y)
{
	return terminal_pick_color(x, y, 0);
}

TERMINAL_INLINE const wchar_t* terminal_get(const wchar_t* key, const wchar_t* default_ = (const wchar_t*)0)
{
	return terminal_wget(key, default_);
}

template<typename T, typename C> T terminal_get(const C* key, const T& default_ = T())
{
	const C* result_str = terminal_get(key, (const C*)0);
	if (result_str[0] == C(0))
		return default_;
	T result;
	return (bool)(std::basic_istringstream<C>(result_str) >> result)? result: default_;
}

#endif /* __cplusplus */

/*
 * Color routines
 */
TERMINAL_INLINE color_t color_from_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return ((color_t)a << 24) | (r << 16) | (g << 8) | b;
}

/*
 * Other functional sugar
 */
TERMINAL_INLINE int terminal_check(int code)
{
	return terminal_state(code) > 0;
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
