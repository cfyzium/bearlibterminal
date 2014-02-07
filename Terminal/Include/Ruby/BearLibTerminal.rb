#
# BearLibTerminal
# Copyright (C) 2014 Cfyz
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
# Release date: 2014-01-17

require 'fiddle'
require 'rbconfig'

module Terminal
	OS ||=
	(
		host_os = RbConfig::CONFIG['host_os']
		case host_os
			when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
				:windows
			when /darwin|mac os/
				:macosx
			when /linux/
				:linux
			when /solaris|bsd/
				:unix
		else
        	raise RuntimeError, "Unknown OS: #{host_os.inspect}"
      	end
    )

	case OS
		when /linux/
			Libname = "./libBearLibTerminal.so"
		when /windows/
			Libname = "BearLibTerminal.dll"
	else
		raise RuntimeError, "Unsupported OS: #{OS}"
	end
	
	if RUBY_VERSION >= '2.0.0'
		Lib = Fiddle.dlopen(Libname)
		Ptr = Fiddle::Pointer
	else
		Lib = DL.dlopen(Libname)
		Ptr = DL::CPtr
	end

	# Raw API entries
	Open = Fiddle::Function.new(Lib['terminal_open'], [], Fiddle::TYPE_INT)
	Close = Fiddle::Function.new(Lib['terminal_close'], [], Fiddle::TYPE_VOID)
	Set = Fiddle::Function.new(Lib['terminal_set8'], [Fiddle::TYPE_VOIDP], Fiddle::TYPE_INT)
	Refresh = Fiddle::Function.new(Lib['terminal_refresh'], [], Fiddle::TYPE_VOID)
	Clear = Fiddle::Function.new(Lib['terminal_clear'], [], Fiddle::TYPE_VOID)
	ClearArea = Fiddle::Function.new(Lib['terminal_clear_area'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Layer = Fiddle::Function.new(Lib['terminal_layer'], [Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Color = Fiddle::Function.new(Lib['terminal_color'], [-Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	BkColor = Fiddle::Function.new(Lib['terminal_bkcolor'], [-Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Composition = Fiddle::Function.new(Lib['terminal_composition'], [Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Put = Fiddle::Function.new(Lib['terminal_put'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	PutExt = Fiddle::Function.new(Lib['terminal_put_ext'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
	Print = Fiddle::Function.new(Lib['terminal_print8'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP], Fiddle::TYPE_INT)
	HasInput = Fiddle::Function.new(Lib['terminal_has_input'], [], Fiddle::TYPE_INT)
	State = Fiddle::Function.new(Lib['terminal_state'], [Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	Read = Fiddle::Function.new(Lib['terminal_read'], [], Fiddle::TYPE_INT)
	ReadExt = Fiddle::Function.new(Lib['terminal_read_ext'], [Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	ReadStr = Fiddle::Function.new(Lib['terminal_read_str8'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP, Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	ColorFromName = Fiddle::Function.new(Lib['color_from_name8'], [Fiddle::TYPE_VOIDP], -Fiddle::TYPE_INT)
	
	# Wrappers
	def self.open?; return Open.call == 1; end
	def self.open; open?; end
	def self.close; Close.call; end
	def self.set? s; return Set.call(Ptr[s]) == 1; end
	def self.set s; set? s; end
	def self.refresh; Refresh.call; end
	def self.clear; Clear.call; end
	def self.clear_area x, y, w, h; ClearArea.call x, y, w, h; end
	def self.layer index; Layer.call index; end
	def self.color value
		Color.call (value.is_a? String)? (ColorFromName.call Ptr[value]): value
	end
	def self.bkcolor value
		BkColor.call (value.is_a? String)? (ColorFromName.call Ptr[value]): value
	end
	def self.composition mode; Composition.call mode; end
	def self.put x, y, code; Put.call x, y, code; end
	def self.put_ext x, y, dx, dy, code, corners=nil
		if corners.nil?
			PutExt.call x, y, dx, dy, code, 0
		else
			packed = corners.pack "l4"
			ptr = Ptr[packed]
			PutExt.call x, y, dx, dy, code, ptr
		end
	end
	def self.print x, y, s; return Print.call x, y, Ptr[s]; end
	def self.has_input?; return HasInput.call == 1; end
	def self.state code; return State.call code; end
	def self.state? code; return state(code) == 1; end
	def self.read; return Read.call; end
	def self.read_ext flags; return ReadExt.call flags; end
	def self.read_str x, y, s, max
		p = Ptr.malloc max*3+1
		ps = Ptr[s]
		for i in 0..ps.size
			p[i] = ps[i]
		end
		rc = ReadStr.call x, y, p, max
		s.replace p.to_s
		return rc
	end
	def self.color_from_name name; return ColorFromName.call Ptr[name]; end
	
	module Constants
		# Mouse buttons
		TK_LBUTTON          = 0x01
		TK_RBUTTON          = 0x02
		TK_MBUTTON          = 0x07
		
		# Terminal is closed
		TK_CLOSE            = 0x03
		
		# Keyboard
		TK_BACK             = 0x08 # Backspace
		TK_BACKSPACE        = 0x08 # Backspace (alias)
		TK_TAB              = 0x09
		TK_RETURN           = 0x0D # Enter
		TK_SHIFT            = 0x10
		TK_CONTROL          = 0x11
		TK_PAUSE            = 0x13 # Pause/Break
		TK_ESCAPE           = 0x1B
		TK_SPACE            = 0x20
		TK_PRIOR            = 0x21 # Page Up
		TK_NEXT             = 0x22 # Page Down
		TK_END              = 0x23
		TK_HOME             = 0x24
		TK_LEFT             = 0x25 # Left arrow
		TK_UP               = 0x26 # Up arrow
		TK_RIGHT            = 0x27 # Right arrow
		TK_DOWN             = 0x28 # Down arrow
		TK_INSERT           = 0x2D
		TK_DELETE           = 0x2E
		TK_0                = 0x30
		TK_1                = 0x31
		TK_2                = 0x32
		TK_3                = 0x33
		TK_4                = 0x34
		TK_5                = 0x35
		TK_6                = 0x36
		TK_7                = 0x37
		TK_8                = 0x38
		TK_9                = 0x39
		TK_A                = 0x41
		TK_B                = 0x42
		TK_C                = 0x43
		TK_D                = 0x44
		TK_E                = 0x45
		TK_F                = 0x46
		TK_G                = 0x47
		TK_H                = 0x48
		TK_I                = 0x49
		TK_J                = 0x4A
		TK_K                = 0x4B
		TK_L                = 0x4C
		TK_M                = 0x4D
		TK_N                = 0x4E
		TK_O                = 0x4F
		TK_P                = 0x50
		TK_Q                = 0x51
		TK_R                = 0x52
		TK_S                = 0x53
		TK_T                = 0x54
		TK_U                = 0x55
		TK_V                = 0x56
		TK_W                = 0x57
		TK_X                = 0x58
		TK_Y                = 0x59
		TK_Z                = 0x5A
		TK_GRAVE            = 0xC0 # `
		TK_MINUS            = 0xBD # -
		TK_EQUALS           = 0xBB # =
		TK_BACKSLASH        = 0xDC # \
		TK_LBRACKET         = 0xDB # [
		TK_RBRACKET         = 0xDD # ]
		TK_SEMICOLON        = 0xBA # ;
		TK_APOSTROPHE       = 0xDE # '
		TK_COMMA            = 0xBC # ,
		TK_PERIOD           = 0xBE # .
		TK_SLASH            = 0xBF # /
		TK_NUMPAD0          = 0x60
		TK_NUMPAD1          = 0x61
		TK_NUMPAD2          = 0x62
		TK_NUMPAD3          = 0x63
		TK_NUMPAD4          = 0x64
		TK_NUMPAD5          = 0x65
		TK_NUMPAD6          = 0x66
		TK_NUMPAD7          = 0x67
		TK_NUMPAD8          = 0x68
		TK_NUMPAD9          = 0x69
		TK_MULTIPLY         = 0x6A # * on numpad
		TK_ADD              = 0x6B # + on numpad
		TK_SUBTRACT         = 0x6D # - on numpad
		TK_DECIMAL          = 0x6E # . on numpad
		TK_DIVIDE           = 0x6F # / on numpad
		TK_F1               = 0x70
		TK_F2               = 0x71
		TK_F3               = 0x72
		TK_F4               = 0x73
		TK_F5               = 0x74
		TK_F6               = 0x75
		TK_F7               = 0x76
		TK_F8               = 0x77
		TK_F9               = 0x78
		TK_F10              = 0x79
		TK_F11              = 0x7A
		TK_F12              = 0x7B

		# If key was released instead of pressed, it's code will be
		# OR'ed with VK_FLAG_RELEASED constant
		TK_FLAG_RELEASED    = 0x100

		# Specific input events
		TK_MOUSE_MOVE       = 0xD3 # Mouse movement
		TK_MOUSE_SCROLL     = 0xD4 # Mouse wheel scroll
		TK_WINDOW_RESIZE    = 0xDF # Window resize event

		# Virtual key-codes for internal terminal states/variables.
		# These can be accessed via terminal_state() function.
		TK_MOUSE_X          = 0xD0 # Mouse cursor position in cells
		TK_MOUSE_Y          = 0xD1
		TK_MOUSE_PIXEL_X    = 0xD5 # Mouse cursor position in pixels
		TK_MOUSE_PIXEL_Y    = 0xD6
		TK_MOUSE_WHEEL      = 0xD2 # Mouse wheel counter (absolute value)
		TK_CELL_WIDTH       = 0xD7 # Character cell size in pixels
		TK_CELL_HEIGHT      = 0xD8
		TK_WIDTH            = 0xD9 # Terminal window size in cells
		TK_HEIGHT           = 0xDA
		TK_COMPOSITION      = 0xC1 # Current composition state
		TK_COLOR            = 0xC2 # Current foregroung color
		TK_BKCOLOR          = 0xC3 # Current background color
		TK_LAYER            = 0xC4 # Current layer

		# Composition option. If turned on it allows for placing several tiles in one cell.
		TK_COMPOSITION_OFF  = 0
		TK_COMPOSITION_ON   = 1

		# Input result codes for terminal_read, terminal_read_char and terminal_read_str.
		TK_INPUT_NONE       = 0
		TK_INPUT_CANCELLED  = -1
		TK_INPUT_CALL_AGAIN	= -2
	
		# Extended reading flags
		TK_READ_CHAR        = 1 # Read an Unicode character, not a virtual key code
		TK_READ_NOREMOVE    = 2 # Do not remove the event from input queue
	end
	
	include Constants
end

# Mixin simplifying loading sprites from memory
# Retrieves an address of array of integer color codes suitable for library
class Array
	def to_addr_s
		@to_s_addr_cache = self.pack "l"+self.length.to_s
		return "0x"+Terminal::Ptr[@to_s_addr_cache].to_i.to_s(16)
	end
end

# Mixin simplifying put/char methods
# Retrieves a UTF-16 code of the first character in a string
class String
	def to_u16_i
		return self.encode("utf-16le").unpack("S")[0]
	end
end
