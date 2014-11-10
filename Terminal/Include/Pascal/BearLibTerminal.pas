{*
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
*
* Release date: 2014-11-10
*}

{$H+}

unit BeaRLibTerminal;

interface  

uses ctypes;

const
  // Keyboard scancodes
  TK_A               = $04;
  TK_B               = $05;
  TK_C               = $06;
  TK_D               = $07;
  TK_E               = $08;
  TK_F               = $09;
  TK_G               = $0A;
  TK_H               = $0B;
  TK_I               = $0C;
  TK_J               = $0D;
  TK_K               = $0E;
  TK_L               = $0F;
  TK_M               = $10;
  TK_N               = $11;
  TK_O               = $12;
  TK_P               = $13;
  TK_Q               = $14;
  TK_R               = $15;
  TK_S               = $16;
  TK_T               = $17;
  TK_U               = $18;
  TK_V               = $19;
  TK_W               = $1A;
  TK_X               = $1B;
  TK_Y               = $1C;
  TK_Z               = $1D;
  TK_1               = $1E;
  TK_2               = $1F;
  TK_3               = $20;
  TK_4               = $21;
  TK_5               = $22;
  TK_6               = $23;
  TK_7               = $24;
  TK_8               = $25;
  TK_9               = $26;
  TK_0               = $27;
  TK_RETURN          = $28;
  TK_ENTER           = $28;
  TK_ESCAPE          = $29;
  TK_BACKSPACE       = $2A;
  TK_TAB             = $2B;
  TK_SPACE           = $2C;
  TK_MINUS           = $2D;
  TK_EQUALS          = $2E;
  TK_LBRACKET        = $2F;
  TK_RBRACKET        = $30;
  TK_BACKSLASH       = $31;
  TK_SEMICOLON       = $33;
  TK_APOSTROPHE      = $34;
  TK_GRAVE           = $35;
  TK_COMMA           = $36;
  TK_PERIOD          = $37;
  TK_SLASH           = $38;
  TK_F1              = $3A;
  TK_F2              = $3B;
  TK_F3              = $3C;
  TK_F4              = $3D;
  TK_F5              = $3E;
  TK_F6              = $3F;
  TK_F7              = $40;
  TK_F8              = $41;
  TK_F9              = $42;
  TK_F10             = $43;
  TK_F11             = $44;
  TK_F12             = $45;
  TK_PAUSE           = $48;
  TK_INSERT          = $49;
  TK_HOME            = $4A;
  TK_PAGEUP          = $4B;
  TK_DELETE          = $4C;
  TK_END             = $4D;
  TK_PAGEDOWN        = $4E;
  TK_RIGHT           = $4F;
  TK_LEFT            = $50;
  TK_DOWN            = $51;
  TK_UP              = $52;
  TK_KP_DIVIDE       = $54;
  TK_KP_MULTIPLY     = $55;
  TK_KP_MINUS        = $56;
  TK_KP_PLUS         = $57;
  TK_KP_ENTER        = $58;
  TK_KP_1            = $59;
  TK_KP_2            = $5A;
  TK_KP_3            = $5B;
  TK_KP_4            = $5C;
  TK_KP_5            = $5D;
  TK_KP_6            = $5E;
  TK_KP_7            = $5F;
  TK_KP_8            = $60;
  TK_KP_9            = $61;
  TK_KP_0            = $62;
  TK_KP_PERIOD       = $63;
  TK_SHIFT           = $70;
  TK_CONTROL         = $71;

  // Mouse events/states
  TK_MOUSE_LEFT       = $80; // Buttons
  TK_MOUSE_RIGHT      = $81;
  TK_MOUSE_MIDDLE     = $82;
  TK_MOUSE_X1         = $83;
  TK_MOUSE_X2         = $84;
  TK_MOUSE_MOVE       = $85; // Movement event
  TK_MOUSE_SCROLL     = $86; // Mouse scroll event
  TK_MOUSE_X          = $87; // Cusor position in cells
  TK_MOUSE_Y          = $88;
  TK_MOUSE_PIXEL_X    = $89; // Cursor position in pixels
  TK_MOUSE_PIXEL_Y    = $8A;
  TK_MOUSE_WHEEL      = $8B; // Scroll direction and amount
  TK_MOUSE_CLICKS     = $8C; // Number of consecutive clicks

  // If key was released instead of pressed, it's code will be OR'ed with TK_KEY_RELEASED
  TK_KEY_RELEASED     = $100;

  // Virtual key-codes for internal terminal states/variables.
  // These can be accessed via terminal_state function.
  TK_WIDTH            = $C0; // Terminal window size in cells
  TK_HEIGHT           = $C1;
  TK_CELL_WIDTH       = $C2; // Character cell size in pixels
  TK_CELL_HEIGHT      = $C3;
  TK_COLOR            = $C4; // Current foregroung color
  TK_BKCOLOR          = $C5; // Current background color
  TK_LAYER            = $C6; // Current layer
  TK_COMPOSITION      = $C7; // Current composition state
  TK_CHAR             = $C8; // Translated ANSI code of last produced character
  TK_WCHAR            = $C9; // Unicode codepoint of last produced character
  TK_EVENT            = $CA; // Last dequeued event
  TK_FULLSCREEN       = $CB; // Fullscreen state

  //Other events
  TK_CLOSE            = $E0;
  TK_RESIZED          = $E1;

  // Generic mode enum.
  // Right now it is used for composition option only.
  TK_OFF              =   0;
  TK_ON               =   1;

  // Input result codes for terminal_read function.
  TK_INPUT_NONE       =   0;
  TK_INPUT_CANCELLED  =  -1;

type
  Color = CUInt32;
  PColor = ^Color;

// ----------------------------------------------------------------------------
// Module interface
// ----------------------------------------------------------------------------

// Open
function terminal_open(): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_open';

// Close
procedure terminal_close();
  cdecl; external 'BearLibTerminal' name 'terminal_close';

// Set
function terminal_set(const Options: String): LongBool;

function terminal_set(const Options: UnicodeString): LongBool;

// Refresh
procedure terminal_refresh();
  cdecl; external 'BearLibTerminal' name 'terminal_refresh';

// Clear
procedure terminal_clear();
  cdecl; external 'BearLibTerminal' name 'terminal_clear';

// ClearArea
procedure terminal_clear_area(Left, Top, Width, Height: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_clear_area';

// Crop
procedure terminal_crop(Left, Top, Width, Height: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_crop';

// Color
procedure terminal_color(Color: Color);
  cdecl; external 'BearLibTerminal' name 'terminal_color';
  
procedure terminal_color(Color: String);

procedure terminal_color(Color: UnicodeString);

// BkColor
procedure terminal_bkcolor(Color: Color);
  cdecl; external 'BearLibTerminal' name 'terminal_bkcolor';

procedure terminal_bkcolor(Color: String);

procedure terminal_bkcolor(Color: UnicodeString);

// Composition
procedure terminal_composition(Mode: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_composition';

// Layer
procedure terminal_layer(Mode: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_layer';

// Put
procedure terminal_put(X, Y: Integer; Code: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_put';

procedure terminal_put(X, Y: Integer; Code: UnicodeChar);

// PutExt
procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: Integer; Corners: PColor);
  cdecl; external 'BearLibTerminal' name 'terminal_put_ext';

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: UnicodeChar; Corners: PColor);

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: Integer);

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: UnicodeChar);

// Pick
function terminal_pick(X, Y, Index: Integer): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_pick';

function terminal_pick(X, Y: Integer): Integer;

// PickColor
function terminal_pick_color(X, Y, Index: Integer): Color;
  cdecl; external 'BearLibTerminal' name 'terminal_pick_color';

function terminal_pick_color(X, Y: Integer): Color;

// PickBkColor
function terminal_pick_bkcolor(X, Y: Integer): Color;
  cdecl; external 'BearLibTerminal' name 'terminal_pick_bkcolor';

// Print
function terminal_print(X, Y: Integer; const S: String): Cardinal;

function terminal_print(X, Y: Integer; const S: UnicodeString): Cardinal;

// Measure
function terminal_measure(const S: String): Cardinal;

function terminal_measure(const S: UnicodeString): Cardinal;

// HasInput
function terminal_has_input(): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_has_input';

// State
function terminal_state(Code: Integer): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_state';

// Check
function terminal_check(Code: Integer): Boolean;

// Read
function terminal_read(): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_read';
  
// Peek
function terminal_peek(): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_peek';

// Delay
procedure terminal_delay(Period: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_delay';

// ColorFromName
function color_from_name(const Name: String): Color;

function color_from_name(const Name: UnicodeString): Color;

// ColorFromARGB
function color_from_argb(a, r, g, b: Integer): Color;

// ----------------------------------------------------------------------------
// Module implementation
// ----------------------------------------------------------------------------

implementation

function terminal_set_ansi(const Options: PChar): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_set8';

function terminal_set(const Options: String): LongBool;
begin
    terminal_set := terminal_set_ansi(PChar(Options));
end;

function terminal_set_unicode(const Options: PUnicodeChar): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_set16';

function terminal_set(const Options: UnicodeString): LongBool;
begin
    terminal_set := terminal_set_unicode(PUnicodeChar(Options));
end;

procedure terminal_color(Color: String);
begin
	terminal_color(color_from_name(Color));
end;

procedure terminal_color(Color: UnicodeString);
begin
	terminal_color(color_from_name(Color));
end;

procedure terminal_bkcolor(Color: String);
begin
	terminal_bkcolor(color_from_name(Color));
end;

procedure terminal_bkcolor(Color: UnicodeString);
begin
	terminal_bkcolor(color_from_name(Color));
end;

procedure terminal_put(X, Y: Integer; Code: UnicodeChar);
begin
    terminal_put(X, Y, ord(Code));
end;

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: UnicodeChar; Corners: PColor);
begin
	terminal_put_ext(X, Y, dX, dY, ord(Code), Corners);
end;

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: Integer);
begin
	terminal_put_ext(X, Y, dX, dY, Code, PColor(0));
end;

procedure terminal_put_ext(X, Y, dX, dY: Integer; Code: UnicodeChar);
begin
	terminal_put_ext(X, Y, dX, dY, ord(Code), PColor(0));
end;

function terminal_pick(X, Y: Integer): Integer;
begin
	terminal_pick := terminal_pick(X, Y, 0);
end;

function terminal_pick_color(X, Y: Integer): Color;
begin;
	terminal_pick_color := terminal_pick_color(X, Y, 0);
end;

function terminal_print_ansi(X, Y: Integer; const S: PChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_print8';

function terminal_print(X, Y: Integer; const S: String): Cardinal;
begin
    terminal_print := terminal_print_ansi(X, Y, PChar(S));
end;

function terminal_print_unicode(X, Y: Integer; const S: PUnicodeChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_print16';

function terminal_print(X, Y: Integer; const S: UnicodeString): Cardinal;
begin
	terminal_print := terminal_print_unicode(X, Y, PUnicodeChar(S));
end;

function terminal_measure_ansi(const S: PChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_measure8';

function terminal_measure(const S: String): Cardinal;
begin
    terminal_measure := terminal_measure_ansi(PChar(S));
end;

function terminal_measure_unicode(const S: PUnicodeChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_measure16';

function terminal_measure(const S: UnicodeString): Cardinal;
begin
	terminal_measure := terminal_measure_unicode(PUnicodeChar(S));
end;

function terminal_check(Code: Integer): Boolean;
begin
	terminal_check := terminal_state(Code) > 0;
end;

function color_from_name_ansi(const Name: PChar): Color;
  cdecl; external 'BearLibTerminal' name 'color_from_name8';

function color_from_name(const Name: String): Color;
begin
    color_from_name := color_from_name_ansi(PChar(Name));
end;

function color_from_name_unicode(const Name: PUnicodeChar): Color;
  cdecl; external 'BearLibTerminal' name 'color_from_name16';

function color_from_name(const Name: UnicodeString): Color;
begin
    color_from_name := color_from_name_unicode(PUnicodeChar(Name));
end;

function color_from_argb(a, r, g, b: Integer): Color;
begin
    if a > 255 then a := 255;
    if r > 255 then r := 255;
    if g > 255 then g := 255;
    if b > 255 then b := 255;
    color_from_argb := (a shl 24) or (r shl 16) or (g shl 8) or b;
end;

end.
