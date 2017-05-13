#
# BearLibTerminal
# Copyright (C) 2014-2017 Cfyz
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
		when /macosx/
			Libname = "./libBearLibTerminal.dylib"
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
	Crop = Fiddle::Function.new(Lib['terminal_crop'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Layer = Fiddle::Function.new(Lib['terminal_layer'], [Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Color = Fiddle::Function.new(Lib['terminal_color'], [-Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	BkColor = Fiddle::Function.new(Lib['terminal_bkcolor'], [-Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Composition = Fiddle::Function.new(Lib['terminal_composition'], [Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Font = Fiddle::Function.new(Lib['terminal_font8'], [Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
	Put = Fiddle::Function.new(Lib['terminal_put'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	PutExt = Fiddle::Function.new(Lib['terminal_put_ext'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
	Pick = Fiddle::Function.new(Lib['terminal_pick'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	PickColor = Fiddle::Function.new(Lib['terminal_pick_color'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT], -Fiddle::TYPE_INT)
	PickBkColor = Fiddle::Function.new(Lib['terminal_pick_bkcolor'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT], -Fiddle::TYPE_INT)
	PrintExt = Fiddle::Function.new(Lib['terminal_print_ext8'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
	MeasureExt = Fiddle::Function.new(Lib['terminal_measure_ext8'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
	HasInput = Fiddle::Function.new(Lib['terminal_has_input'], [], Fiddle::TYPE_INT)
	State = Fiddle::Function.new(Lib['terminal_state'], [Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	Read = Fiddle::Function.new(Lib['terminal_read'], [], Fiddle::TYPE_INT)
	Peek = Fiddle::Function.new(Lib['terminal_peek'], [], Fiddle::TYPE_INT)
	ReadStr = Fiddle::Function.new(Lib['terminal_read_str8'], [Fiddle::TYPE_INT, Fiddle::TYPE_INT, Fiddle::TYPE_VOIDP, Fiddle::TYPE_INT], Fiddle::TYPE_INT)
	Delay = Fiddle::Function.new(Lib['terminal_delay'], [Fiddle::TYPE_INT], Fiddle::TYPE_VOID)
	Get = Fiddle::Function.new(Lib['terminal_get8'], [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOIDP)
	ColorFromName = Fiddle::Function.new(Lib['color_from_name8'], [Fiddle::TYPE_VOIDP], -Fiddle::TYPE_INT)
	
	# Temporary buffer for by-pointer integer arguments.
	Out = Fiddle::Pointer.malloc(Fiddle::SIZEOF_INT * 2)
	
	# Wrappers
	def self.open; return Open.call == 1; end
	def self.close; Close.call; end
	def self.set s; return Set.call(Ptr[s]) == 1; end
	def self.refresh; Refresh.call; end
	def self.clear; Clear.call; end
	def self.clear_area x, y, w, h; ClearArea.call x, y, w, h; end
	def self.crop x, y, w, h; Crop.call x, y, w, h; end
	def self.layer index; Layer.call index; end
	def self.color value
		v = (value.is_a? String)? (ColorFromName.call Ptr[value]): value
		if v > 2**31-1 then v -= 2**32 end
		Color.call v
	end
	def self.bkcolor value
		v = (value.is_a? String)? (ColorFromName.call Ptr[value]): value
		if v > 2**31-1 then v -= 2**32 end
		BkColor.call v
	end
	def self.composition mode; Composition.call mode; end
	def self.font name; Font.call(Ptr[name]); end
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
	def self.pick x, y, z=0; Pick.call x, y, z; end
	def self.pick_color x, y, z=0; PickColor.call x, y, z; end
	def self.pick_bkcolor x, y; PickBkColor.call x, y; end
	def self.print_ext x, y, w, h, a, s;
		PrintExt.call x, y, w, h, a, Ptr[s], Out+0, Out+Fiddle::SIZEOF_INT
		return Out[0], Out[Fiddle::SIZEOF_INT]
	end
	def self.print x, y, s; return print_ext(x, y, 0, 0, 0, s); end
	def self.measure_ext w, h, s;
		MeasureExt.call w, h, Ptr[s], Out+0, Out+Fiddle::SIZEOF_INT
		return Out[0], Out[Fiddle::SIZEOF_INT]
	end
    def self.measure s;
    	return measure_ext(0, 0, s)
    end
	def self.has_input?; return HasInput.call == 1; end
	def self.state code; return State.call code; end
	def self.check? code; return state(code) > 0; end
	def self.read; return Read.call; end
	def self.peek; return Peek.call; end
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
	def self.delay period; Delay.call period; end
	def self.get s, default_value = ""
		return (Get.call Ptr[s], Ptr[default_value]).to_s
	end
	def self.color_from_name name; return ColorFromName.call Ptr[name]; end
	def self.color_from_argb a, r, g, b
		return (a << 24) | (r << 16) | (g << 8) | b
	end
	
	module Constants
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
	end
	
	include Constants
end

# Mixin simplifying loading sprites from memory. Retrieves an address of array of integer color
# codes suitable for library:
# > pixels = [...]
# > Terminal.set "0xE000: #{pixels.to_addr_s}, size=2x2"
class Array
	def to_addr_s
		@to_s_addr_cache = self.pack "l"+self.length.to_s
		return "0x"+Terminal::Ptr[@to_s_addr_cache].to_i.to_s(16)
	end
end

# Mixin simplifying putting chars. Retrieves a UTF-16 code of the first character in a string:
# > Terminal.put_ext x, y, dx, dy, "#".to_u16_i
class String
	def to_u16_i
		return self.encode("utf-16le").unpack("S")[0]
	end
end
