/*
* BearLibTerminal
* Copyright (C) 2016-2017 Joe "ZhayTee" Toscano, Cfyz
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

package bearlibterminal

// #cgo LDFLAGS: -lBearLibTerminal
// #include <stdlib.h>
// #include <BearLibTerminal.h>
import "C"

import (
	"unsafe"
)

//
// Keyboard scancodes for events/states
//
const (
	TK_A           = 0x04
	TK_B           = 0x05
	TK_C           = 0x06
	TK_D           = 0x07
	TK_E           = 0x08
	TK_F           = 0x09
	TK_G           = 0x0A
	TK_H           = 0x0B
	TK_I           = 0x0C
	TK_J           = 0x0D
	TK_K           = 0x0E
	TK_L           = 0x0F
	TK_M           = 0x10
	TK_N           = 0x11
	TK_O           = 0x12
	TK_P           = 0x13
	TK_Q           = 0x14
	TK_R           = 0x15
	TK_S           = 0x16
	TK_T           = 0x17
	TK_U           = 0x18
	TK_V           = 0x19
	TK_W           = 0x1A
	TK_X           = 0x1B
	TK_Y           = 0x1C
	TK_Z           = 0x1D
	TK_1           = 0x1E
	TK_2           = 0x1F
	TK_3           = 0x20
	TK_4           = 0x21
	TK_5           = 0x22
	TK_6           = 0x23
	TK_7           = 0x24
	TK_8           = 0x25
	TK_9           = 0x26
	TK_0           = 0x27
	TK_RETURN      = 0x28
	TK_ENTER       = 0x28
	TK_ESCAPE      = 0x29
	TK_BACKSPACE   = 0x2A
	TK_TAB         = 0x2B
	TK_SPACE       = 0x2C
	TK_MINUS       = 0x2D /*  -  */
	TK_EQUALS      = 0x2E /*  =  */
	TK_LBRACKET    = 0x2F /*  [  */
	TK_RBRACKET    = 0x30 /*  ]  */
	TK_BACKSLASH   = 0x31 /*  \  */
	TK_SEMICOLON   = 0x33 /*  ;  */
	TK_APOSTROPHE  = 0x34 /*  '  */
	TK_GRAVE       = 0x35 /*  `  */
	TK_COMMA       = 0x36 /*  ,  */
	TK_PERIOD      = 0x37 /*  .  */
	TK_SLASH       = 0x38 /*  /  */
	TK_F1          = 0x3A
	TK_F2          = 0x3B
	TK_F3          = 0x3C
	TK_F4          = 0x3D
	TK_F5          = 0x3E
	TK_F6          = 0x3F
	TK_F7          = 0x40
	TK_F8          = 0x41
	TK_F9          = 0x42
	TK_F10         = 0x43
	TK_F11         = 0x44
	TK_F12         = 0x45
	TK_PAUSE       = 0x48 /* Pause/Break */
	TK_INSERT      = 0x49
	TK_HOME        = 0x4A
	TK_PAGEUP      = 0x4B
	TK_DELETE      = 0x4C
	TK_END         = 0x4D
	TK_PAGEDOWN    = 0x4E
	TK_RIGHT       = 0x4F /* Right arrow */
	TK_LEFT        = 0x50 /* Left arrow */
	TK_DOWN        = 0x51 /* Down arrow */
	TK_UP          = 0x52 /* Up arrow */
	TK_KP_DIVIDE   = 0x54 /* '/' on numpad */
	TK_KP_MULTIPLY = 0x55 /* '*' on numpad */
	TK_KP_MINUS    = 0x56 /* '-' on numpad */
	TK_KP_PLUS     = 0x57 /* '+' on numpad */
	TK_KP_ENTER    = 0x58
	TK_KP_1        = 0x59
	TK_KP_2        = 0x5A
	TK_KP_3        = 0x5B
	TK_KP_4        = 0x5C
	TK_KP_5        = 0x5D
	TK_KP_6        = 0x5E
	TK_KP_7        = 0x5F
	TK_KP_8        = 0x60
	TK_KP_9        = 0x61
	TK_KP_0        = 0x62
	TK_KP_PERIOD   = 0x63 /* '.' on numpad */
	TK_SHIFT       = 0x70
	TK_CONTROL     = 0x71
	TK_ALT         = 0x72
)

//
// Mouse events/states
//
const (
	TK_MOUSE_LEFT    = 0x80 /* Buttons */
	TK_MOUSE_RIGHT   = 0x81
	TK_MOUSE_MIDDLE  = 0x82
	TK_MOUSE_X1      = 0x83
	TK_MOUSE_X2      = 0x84
	TK_MOUSE_MOVE    = 0x85 /* Movement event */
	TK_MOUSE_SCROLL  = 0x86 /* Mouse scroll event */
	TK_MOUSE_X       = 0x87 /* Cusor position in cells */
	TK_MOUSE_Y       = 0x88
	TK_MOUSE_PIXEL_X = 0x89 /* Cursor position in pixels */
	TK_MOUSE_PIXEL_Y = 0x8A
	TK_MOUSE_WHEEL   = 0x8B /* Scroll direction and amount */
	TK_MOUSE_CLICKS  = 0x8C /* Number of consecutive clicks */
)

//
// If key was released instead of pressed, it's code will be OR'ed with TK_KEY_RELEASED:
// a) pressed 'A': 0x04
// b) released 'A': 0x04|VK_KEY_RELEASED = 0x104
//
const TK_KEY_RELEASED = 0x100

//
// Virtual key-codes for internal terminal states/variables.
// These can be accessed via terminal_state function.
//
const (
	TK_WIDTH       = 0xC0 /* Terminal window size in cells */
	TK_HEIGHT      = 0xC1
	TK_CELL_WIDTH  = 0xC2 /* Character cell size in pixels */
	TK_CELL_HEIGHT = 0xC3
	TK_COLOR       = 0xC4 /* Current foregroung color */
	TK_BKCOLOR     = 0xC5 /* Current background color */
	TK_LAYER       = 0xC6 /* Current layer */
	TK_COMPOSITION = 0xC7 /* Current composition state */
	TK_CHAR        = 0xC8 /* Translated ANSI code of last produced character */
	TK_WCHAR       = 0xC9 /* Unicode codepoint of last produced character */
	TK_EVENT       = 0xCA /* Last dequeued event */
	TK_FULLSCREEN  = 0xCB /* Fullscreen state */
)

//
// Other events
//
const (
	TK_CLOSE   = 0xE0
	TK_RESIZED = 0xE1
)

//
// Generic mode enum.
// Right now it is used for composition option only.
//
const (
	TK_OFF = 0
	TK_ON  = 1
)

//
// Input result codes for terminal_read function.
//
const (
	TK_INPUT_NONE      = 0
	TK_INPUT_CANCELLED = -1
)

const (
	TK_ALIGN_DEFAULT = 0
	TK_ALIGN_LEFT    = 1
	TK_ALIGN_RIGHT   = 2
	TK_ALIGN_CENTER  = 3
	TK_ALIGN_TOP     = 4
	TK_ALIGN_BOTTOM  = 8
	TK_ALIGN_MIDDLE  = 12
)

//
// Initialization and configuration
//

func Open() int {
	return int(C.terminal_open())
}

func Close() {
	C.terminal_close()
}

func Set(value string) int {
	cstring := C.CString(value)
	defer C.free(unsafe.Pointer(cstring))
	return int(C.terminal_set(cstring))
}

//
// Output state
//

func Color(color uint32) {
	C.terminal_color(C.color_t(color))
}

func BkColor(color uint32) {
	C.terminal_bkcolor(C.color_t(color))
}

func Composition(mode int) {
	C.terminal_composition(C.int(mode))
}

func Layer(index int) {
	C.terminal_layer(C.int(index))
}

func Font(name string) {
	cstring := C.CString(name)
	defer C.free(unsafe.Pointer(cstring))
	C.terminal_font(cstring)
}

//
// Output
//

func Clear() {
	C.terminal_clear()
}

func ClearArea(x, y, w, h int) {
	C.terminal_clear_area(C.int(x), C.int(y), C.int(w), C.int(h))
}

func Crop(x, y, w, h int) {
	C.terminal_crop(C.int(x), C.int(y), C.int(w), C.int(h))
}

func Refresh() {
	C.terminal_refresh()
}

func Put(x, y, code int) {
	C.terminal_put(C.int(x), C.int(y), C.int(code))
}

func Pick(x, y, index int) int {
	return int(C.terminal_pick(C.int(x), C.int(y), C.int(index)))
}

func PickColor(x, y, index int) uint32 {
	val := C.terminal_pick_color(C.int(x), C.int(y), C.int(index))
	return uint32(val)
}

func PickBkColor(x, y int) uint32 {
	val := C.terminal_pick_bkcolor(C.int(x), C.int(y))
	return uint32(val)
}

func PutExt(x, y, dx, dy, code int, corners [4]uint32) {
	var cornerColors [4]C.color_t
	for i := 0; i < 4; i++ {
		cornerColors[i] = C.color_t(corners[i])
	}

	C.terminal_put_ext(C.int(x), C.int(y), C.int(dx), C.int(dy), C.int(code), &cornerColors[0])
}

func PrintExt(x, y, w, h, alignment int, s string) (width, height int) {
	cstring := C.CString(s)
	defer C.free(unsafe.Pointer(cstring))
	sz := C.terminal_print_ext(C.int(x), C.int(y), C.int(w), C.int(h), C.int(alignment), cstring)
	return int(sz.width), int(sz.height)
}

func Print(x, y int, s string) (width, height int) {
	return PrintExt(x, y, 0, 0, TK_ALIGN_DEFAULT, s)
}

func MeasureExt(w, h int, s string) (width, height int) {
	cstring := C.CString(s)
	defer C.free(unsafe.Pointer(cstring))
	sz := C.terminal_measure_ext(C.int(w), C.int(h), cstring)
	return int(sz.width), int(sz.height)
}

func Measure(s string) (width, height int) {
	return MeasureExt(0, 0, s)
}

//
// Input
//

func State(code int) int {
	return int(C.terminal_state(C.int(code)))
}

func Check(code int) int {
	return int(C.terminal_check(C.int(code)))
}

func HasInput() bool {
	result := int(C.terminal_has_input())
	return result > 0
}

func Read() int {
	return int(C.terminal_read())
}

func Peek() int {
	return int(C.terminal_peek())
}

func ReadStr(x, y int, max int) (int, string) {
	cstring := C.CString("")
	defer C.free(unsafe.Pointer(cstring))
	result := int(C.terminal_read_str(C.int(x), C.int(y), cstring, C.int(max)))
	return result, C.GoString(cstring)
}

func Delay(period int) {
	C.terminal_delay(C.int(period))
}

func Get(key, defaultValue string) string {
	cstringKey := C.CString(key)
	defer C.free(unsafe.Pointer(cstringKey))
	cstringDefaultValue := C.CString(defaultValue)
	defer C.free(unsafe.Pointer(cstringDefaultValue))
	result := C.terminal_get(cstringKey, cstringDefaultValue)
	return C.GoString(result)
}

func ColorFromName(name string) uint32 {
	cstring := C.CString(name)
	defer C.free(unsafe.Pointer(cstring))
	val := C.color_from_name(cstring)
	return uint32(val)
}

func ColorFromARGB(a, r, g, b uint8) uint32 {
	val := C.color_from_argb(C.uint8_t(a), C.uint8_t(r), C.uint8_t(g), C.uint8_t(b))
	return uint32(val)
}
