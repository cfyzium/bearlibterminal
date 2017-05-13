#
# BearLibTerminal
# Copyright (C) 2013-2017 Cfyz
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

import sys, ctypes, atexit
from ctypes import c_int, c_uint32, c_char_p, c_wchar_p, POINTER

_version3 = sys.version_info >= (3, 0)
_integer = int if _version3 else (int, long)

def _load_library():
	import os
	
	# Figure out where the library binary should be
	places = [sys.argv[0]]
	try:
		# Try to search near the wrapper module
		places.insert(0, __file__)
	except NameError:
		# There may be no module (e. g. a standalone app)
		pass
	places = [os.path.dirname(os.path.abspath(place)) for place in places]
	
	# Construct a platform-specific name of the library binary file
	if 'win32' in sys.platform:
		name = 'BearLibTerminal.dll'
	elif 'linux' in sys.platform:
		name = 'libBearLibTerminal.so'
	elif 'darwin' in sys.platform:
		name = 'libBearLibTerminal.dylib'
	else:
		raise RuntimeError('Unsupported platform: ' + sys.platform)
	
	# Now actually try to load the library binary
	for place in places:
		try:
			return ctypes.CDLL(os.path.join(place, name))
		except OSError:
			pass
	raise RuntimeError('Cannot load BearLibTerminal library: no {} found in {}'.format(name, places))

_library = _load_library()

# wchar_t size may vary
if ctypes.sizeof(ctypes.c_wchar()) == 4:
	_wset = _library.terminal_set32
	_wprint_ext = _library.terminal_print_ext32
	_wmeasure_ext = _library.terminal_measure_ext32
	_read_wstr = _library.terminal_read_str32
	_color_from_wname = _library.color_from_name32
	_wget = _library.terminal_get32
	_wfont = _library.terminal_font32
else:
	_wset = _library.terminal_set16
	_wprint_ext = _library.terminal_print_ext16
	_wmeasure_ext = _library.terminal_measure_ext16
	_read_wstr = _library.terminal_read_str16
	_color_from_wname = _library.color_from_name16
	_wget = _library.terminal_get16
	_wfont = _library.terminal_font16

# color/bkcolor accept uint32, color_from_name returns uint32
_library.terminal_color.argtypes = [c_uint32]
_library.terminal_bkcolor.argtypes = [c_uint32]
_library.color_from_name8.restype = c_uint32
_color_from_wname.restype = c_uint32

def color_from_name(s):
	if _version3 or isinstance(s, unicode):
		return _color_from_wname(s)
	else:
		return _library.color_from_name8(s)

def open():
	if _library.terminal_open() == 0:
		return False
	set('terminal.encoding-affects-put=false')
	# Try to register a terminal ipython integration.
	try:
		from IPython.lib import inputhook
		def bearlibterminal_inputhook():
			has_input()
			while not inputhook.stdin_ready():
				delay(5)
			return 0
		inputhook.inputhook_manager.set_inputhook(bearlibterminal_inputhook)
	except:
		pass
	return True

# Try to register a kernel-based ipython/jupyter integration.
try:
    from ipykernel.eventloops import register_integration
    @register_integration('blt')
    def loop_blt(kernel):
        interval = int(kernel._poll_interval * 1000) # to milliseconds
        while True:
            kernel.do_one_iteration()
            delay(interval)
except:
    pass

close = _library.terminal_close
close.restype = None

def set(s):
	if _version3 or isinstance(s, unicode):
		return _wset(s) == 1
	else:
		return _library.terminal_set8(s) == 1

def setf(s, *args):
	return set(s.format(*args))

refresh = _library.terminal_refresh
refresh.restype = None

clear = _library.terminal_clear
clear.restype = None

clear_area = _library.terminal_clear_area
clear_area.restype = None

crop = _library.terminal_crop
crop.restype = None

layer = _library.terminal_layer
layer.restype = None

def color(v):
	if isinstance(v, _integer):
		_library.terminal_color(v)
	else:
		_library.terminal_color(color_from_name(v))

def bkcolor(v):
	if isinstance(v, _integer):
		_library.terminal_bkcolor(v)
	else:
		_library.terminal_bkcolor(color_from_name(v))

composition = _library.terminal_composition
composition.restype = None

_afont = _library.terminal_font8
_afont.restype = None
_afont.argtypes = [c_char_p]
_wfont.restype = None
_wfont.argtypes = [c_wchar_p]
def font(name):
	if _version3 or isinstance(name, unicode):
		_wfont(name)
	else:
		_afont(name)

def put(x, y, c):
	if not isinstance(c, _integer):
		c = ord(c)
	_library.terminal_put(x, y, c)

def put_ext(x, y, dx, dy, c, corners=None):
	if not isinstance(c, _integer):
		c = ord(c)
	if corners is None:
		_library.terminal_put_ext(x, y, dx, dy, c, None)
	else:
		for i in range(0, 4):
			put_ext.corners[i] = corners[i]
		_library.terminal_put_ext(x, y, dx, dy, c, ctypes.cast(put_ext.corners, ctypes.POINTER(ctypes.c_uint)))
put_ext.corners = (c_uint32 * 4)()

def pick(x, y, z = 0):
	return _library.terminal_pick(x, y, z);

def pick_color(x, y, z = 0):
	return _library.terminal_pick_color(x, y, z);

pick_bkcolor = _library.terminal_pick_bkcolor
pick_bkcolor.restype = c_uint32

_aprint_ext = _library.terminal_print_ext8
_aprint_ext.argtypes = [c_int, c_int, c_int, c_int, c_int, c_char_p, POINTER(c_int), POINTER(c_int)]
_aprint_ext.restype = None
_wprint_ext.argtypes = [c_int, c_int, c_int, c_int, c_int, c_wchar_p, POINTER(c_int), POINTER(c_int)]
_wprint_ext.restype = None
def puts(x, y, s, width=0, height=0, align=0):
	out_width = c_int()
	out_height = c_int()
	if _version3 or isinstance(s, unicode):
		_wprint_ext(x, y, width, height, align, s, ctypes.byref(out_width), ctypes.byref(out_height))
	else:
		_aprint_ext(x, y, width, height, align, s, ctypes.byref(out_width), ctypes.byref(out_height))
	return (out_width.value, out_height.value)

print_ = puts

if _version3:
	setattr(sys.modules[__name__], "print", puts)

def printf(x, y, s, *args):
	return puts(x, y, s.format(*args))

_ameasure_ext = _library.terminal_measure_ext8
_ameasure_ext.argtypes = [c_int, c_int, c_char_p, POINTER(c_int), POINTER(c_int)]
_ameasure_ext.restype = None
_wmeasure_ext.argtypes = [c_int, c_int, c_wchar_p, POINTER(c_int), POINTER(c_int)]
_wmeasure_ext.restype = None
def measure(s, width=0, height=0):
	out_width = c_int()
	out_height = c_int()
	if _version3 or isinstance(s, unicode):
		_wmeasure_ext(width, height, s, ctypes.byref(out_width), ctypes.byref(out_height))
	else:
		_ameasure_ext(width, height, s, ctypes.byref(out_width), ctypes.byref(out_height))
	return (out_width.value, out_height.value)

def measuref(s, *args):
	return measure(s.format(*args))

def has_input():
	return _library.terminal_has_input() == 1

state = _library.terminal_state

def check(state):
	return _library.terminal_state(state) > 0

read = _library.terminal_read

peek = _library.terminal_peek

def read_str(x, y, s, max):
	if _version3 or isinstance(s, unicode):
		p = ctypes.create_unicode_buffer(s, max+1)
		rc = _read_wstr(x, y, p, max)
		return rc, p.value
	else:
		p = ctypes.create_string_buffer(s, max+1)
		rc = _library.terminal_read_str8(x, y, p, max)
		return rc, p.value

delay = _library.terminal_delay
delay.restype = None

_library.terminal_get8.restype = c_char_p
_wget.restype = c_wchar_p

def get(s, default_value=None):
	if _version3:
		return _wget(s, default_value)
	elif isinstance(s, unicode):
		return unicode(_wget(s, default_value));
	else:
		return str(_library.terminal_get8(s, default_value))

def color_from_argb(a, r, g, b):
	result = a
	result = result * 256 + r
	result = result * 256 + g
	result = result * 256 + b
	return result

@atexit.register
def _cleanup():
	close()

# Keyboard scancodes for events/states.
TK_A                = 0x04
TK_B                = 0x05
TK_C                = 0x06
TK_D                = 0x07
TK_E                = 0x08
TK_F                = 0x09
TK_G                = 0x0A
TK_H                = 0x0B
TK_I                = 0x0C
TK_J                = 0x0D
TK_K                = 0x0E
TK_L                = 0x0F
TK_M                = 0x10
TK_N                = 0x11
TK_O                = 0x12
TK_P                = 0x13
TK_Q                = 0x14
TK_R                = 0x15
TK_S                = 0x16
TK_T                = 0x17
TK_U                = 0x18
TK_V                = 0x19
TK_W                = 0x1A
TK_X                = 0x1B
TK_Y                = 0x1C
TK_Z                = 0x1D
TK_1                = 0x1E
TK_2                = 0x1F
TK_3                = 0x20
TK_4                = 0x21
TK_5                = 0x22
TK_6                = 0x23
TK_7                = 0x24
TK_8                = 0x25
TK_9                = 0x26
TK_0                = 0x27
TK_RETURN           = 0x28
TK_ENTER            = 0x28
TK_ESCAPE           = 0x29
TK_BACKSPACE        = 0x2A
TK_TAB              = 0x2B
TK_SPACE            = 0x2C
TK_MINUS            = 0x2D
TK_EQUALS           = 0x2E
TK_LBRACKET         = 0x2F
TK_RBRACKET         = 0x30
TK_BACKSLASH        = 0x31
TK_SEMICOLON        = 0x33
TK_APOSTROPHE       = 0x34
TK_GRAVE            = 0x35
TK_COMMA            = 0x36
TK_PERIOD           = 0x37
TK_SLASH            = 0x38
TK_F1               = 0x3A
TK_F2               = 0x3B
TK_F3               = 0x3C
TK_F4               = 0x3D
TK_F5               = 0x3E
TK_F6               = 0x3F
TK_F7               = 0x40
TK_F8               = 0x41
TK_F9               = 0x42
TK_F10              = 0x43
TK_F11              = 0x44
TK_F12              = 0x45
TK_PAUSE            = 0x48
TK_INSERT           = 0x49
TK_HOME             = 0x4A
TK_PAGEUP           = 0x4B
TK_DELETE           = 0x4C
TK_END              = 0x4D
TK_PAGEDOWN         = 0x4E
TK_RIGHT            = 0x4F
TK_LEFT             = 0x50
TK_DOWN             = 0x51
TK_UP               = 0x52
TK_KP_DIVIDE        = 0x54
TK_KP_MULTIPLY      = 0x55
TK_KP_MINUS         = 0x56
TK_KP_PLUS          = 0x57
TK_KP_ENTER         = 0x58
TK_KP_1             = 0x59
TK_KP_2             = 0x5A
TK_KP_3             = 0x5B
TK_KP_4             = 0x5C
TK_KP_5             = 0x5D
TK_KP_6             = 0x5E
TK_KP_7             = 0x5F
TK_KP_8             = 0x60
TK_KP_9             = 0x61
TK_KP_0             = 0x62
TK_KP_PERIOD        = 0x63
TK_SHIFT            = 0x70
TK_CONTROL          = 0x71
TK_ALT              = 0x72

# Mouse events/states.
TK_MOUSE_LEFT       = 0x80 # Buttons
TK_MOUSE_RIGHT      = 0x81
TK_MOUSE_MIDDLE     = 0x82
TK_MOUSE_X1         = 0x83
TK_MOUSE_X2         = 0x84
TK_MOUSE_MOVE       = 0x85 # Movement event
TK_MOUSE_SCROLL     = 0x86 # Mouse scroll event
TK_MOUSE_X          = 0x87 # Cusor position in cells
TK_MOUSE_Y          = 0x88
TK_MOUSE_PIXEL_X    = 0x89 # Cursor position in pixels
TK_MOUSE_PIXEL_Y    = 0x8A
TK_MOUSE_WHEEL      = 0x8B # Scroll direction and amount
TK_MOUSE_CLICKS     = 0x8C # Number of consecutive clicks

# If key was released instead of pressed, it's code will be OR'ed with VK_KEY_RELEASED.
TK_KEY_RELEASED     = 0x100

# Virtual key-codes for internal terminal states/variables.
# These can be accessed via terminal_state function.
TK_WIDTH            = 0xC0 # Terminal width in cells
TK_HEIGHT           = 0xC1 # Terminal height in cells
TK_CELL_WIDTH       = 0xC2 # Cell width in pixels
TK_CELL_HEIGHT      = 0xC3 # Cell height in pixels
TK_COLOR            = 0xC4 # Current foregroung color
TK_BKCOLOR          = 0xC5 # Current background color
TK_LAYER            = 0xC6 # Current layer
TK_COMPOSITION      = 0xC7 # Current composition state
TK_CHAR             = 0xC8 # Translated ANSI code of last produced character
TK_WCHAR            = 0xC9 # Unicode codepoint of last produced character
TK_EVENT            = 0xCA # Last dequeued event
TK_FULLSCREEN       = 0xCB # Fullscreen state

# Other events.
TK_CLOSE            = 0xE0
TK_RESIZED          = 0xE1

# Generic mode enum. Used in Terminal.composition call only.
TK_OFF              =    0
TK_ON               =    1

# Input result codes for terminal_read_str function.
TK_INPUT_NONE       =    0
TK_INPUT_CANCELLED  =   -1

# Text printing alignment.
TK_ALIGN_DEFAULT    =    0
TK_ALIGN_LEFT       =    1
TK_ALIGN_RIGHT      =    2
TK_ALIGN_CENTER     =    3
TK_ALIGN_TOP        =    4
TK_ALIGN_BOTTOM     =    8
TK_ALIGN_MIDDLE     =   12
