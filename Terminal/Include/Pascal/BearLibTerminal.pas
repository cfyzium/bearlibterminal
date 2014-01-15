{*
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
*
* Release date: 2013-12-27
*}

{$H+}

unit BeaRLibTerminal;

interface  

uses ctypes;

const
  // Keyboard scancodes
  TK_LBUTTON        = $01;
  TK_RBUTTON        = $02;
  TK_CLOSE          = $03; // Same code as VK_CANCEL from winuser.h
  TK_BACK           = $08; // Backspace
  TK_BACKSPACE      = $08; // Backspace (alias)
  TK_TAB            = $09;
  TK_RETURN         = $0D; // Enter
  TK_SHIFT          = $10;
  TK_CONTROL        = $11;
  TK_PAUSE          = $13; // Pause/Break
  TK_ESCAPE         = $1B;
  TK_SPACE          = $20;
  TK_PRIOR          = $21; // Page Up
  TK_NEXT           = $22; // Page Down
  TK_END            = $23;
  TK_HOME           = $24;
  TK_LEFT           = $25; // Left arrow
  TK_UP             = $26; // Up arrow
  TK_RIGHT          = $27; // Right arrow
  TK_DOWN           = $28; // Down arrow
  TK_INSERT         = $2D;
  TK_DELETE         = $2E;
  TK_0              = $30;
  TK_1              = $31;
  TK_2              = $32;
  TK_3              = $33;
  TK_4              = $34;
  TK_5              = $35;
  TK_6              = $36;
  TK_7              = $37;
  TK_8              = $38;
  TK_9              = $39;
  TK_A              = $41;
  TK_B              = $42;
  TK_C              = $43;
  TK_D              = $44;
  TK_E              = $45;
  TK_F              = $46;
  TK_G              = $47;
  TK_H              = $48;
  TK_I              = $49;
  TK_J              = $4A;
  TK_K              = $4B;
  TK_L              = $4C;
  TK_M              = $4D;
  TK_N              = $4E;
  TK_O              = $4F;
  TK_P              = $50;
  TK_Q              = $51;
  TK_R              = $52;
  TK_S              = $53;
  TK_T              = $54;
  TK_U              = $55;
  TK_V              = $56;
  TK_W              = $57;
  TK_X              = $58;
  TK_Y              = $59;
  TK_Z              = $5A;
  TK_GRAVE          = $C0; //  `
  TK_MINUS          = $BD; //  -
  TK_EQUALS         = $BB; //  =
  TK_BACKSLASH      = $DC; //  \ .
  TK_LBRACKET       = $DB; //  [
  TK_RBRACKET       = $DD; //  ]
  TK_SEMICOLON      = $BA; //  ;
  TK_APOSTROPHE     = $DE; //  '
  TK_COMMA          = $BC; //  ,
  TK_PERIOD         = $BE; //  .
  TK_SLASH          = $BF; //  /
  TK_NUMPAD0        = $60;
  TK_NUMPAD1        = $61;
  TK_NUMPAD2        = $62;
  TK_NUMPAD3        = $63;
  TK_NUMPAD4        = $64;
  TK_NUMPAD5        = $65;
  TK_NUMPAD6        = $66;
  TK_NUMPAD7        = $67;
  TK_NUMPAD8        = $68;
  TK_NUMPAD9        = $69;
  TK_MULTIPLY       = $6A; // '*' on numpad
  TK_ADD            = $6B; // '+' on numpad
  TK_SUBTRACT       = $6D; // '-' on numpad
  TK_DECIMAL        = $6E; // '.' on numpad
  TK_DIVIDE         = $6F; // '/' on numpad
  TK_F1             = $70;
  TK_F2             = $71;
  TK_F3             = $72;
  TK_F4             = $73;
  TK_F5             = $74;
  TK_F6             = $75;
  TK_F7             = $76;
  TK_F8             = $77;
  TK_F9             = $78;
  TK_F10            = $79;
  TK_F11            = $7A;
  TK_F12            = $7B;
  
  // If key was released instead of pressed, it's code will be OR'ed with VK_FLAG_RELEASED constant
  TK_FLAG_RELEASED  = $100; // This flag is set if key was released instead of pressed
  
  // Specific input events
  TK_MOUSE_MOVE     = $D3; // Mouse movement event
  TK_MOUSE_SCROLL   = $D4; // Mouse wheel scroll event
  TK_WINDOW_RESIZE  = $DF; // Window resize event

  // Virtual codes for various virtual states:
  TK_MOUSE_X        = $D0; // Mouse cursor position in cells
  TK_MOUSE_Y        = $D1;
  TK_MOUSE_PIXEL_X  = $D5; // Mouse cursor position in pixels
  TK_MOUSE_PIXEL_Y  = $D6;
  TK_MOUSE_WHEEL    = $D2; // Mouse wheel counter (absolute value)
  TK_CELL_WIDTH     = $D7; // Character cell size in pixels
  TK_CELL_HEIGHT    = $D8;
  TK_WIDTH          = $D9; // Terminal window size in cells
  TK_HEIGHT         = $DA;
  TK_COMPOSITION    = $C1; // Current composition state
  TK_COLOR			= $C2; // Current foregroung color
  TK_BKCOLOR		= $C3; // Current background color
  TK_LAYER			= $C4; // Current layer

  // Composition option. If turned on it allows for placing several tiles in one cell.
  TK_COMPOSITION_OFF = 0;
  TK_COMPOSITION_ON  = 1;
  
  // Input result codes for terminal_read, terminal_read_char and terminal_read_str.
  TK_INPUT_NONE       =  0;
  TK_INPUT_CANCELLED  = -1;
  TK_INPUT_CALL_AGAIN = -2;
  
  // Extended reading flags
  TK_READ_CHAR        = 1; // Read an Unicode character, not a virtual key code
  TK_READ_NOREMOVE    = 2; // Do not remove the event from input queue

type
  Color = CUInt32;
  PColor = ^Color;

// Open
function terminal_open(): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_open';

// Close
procedure terminal_close();
  cdecl; external 'BearLibTerminal' name 'terminal_close';

// Set
function terminal_set(const Options: PChar): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_set8';

function terminal_set(const Options: String): Integer;

function terminal_set(const Options: PUnicodeChar): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_set16';

function terminal_set(const Options: UnicodeString): Integer;

// Refresh
procedure terminal_refresh();
  cdecl; external 'BearLibTerminal' name 'terminal_refresh';

// Clear
procedure terminal_clear();
  cdecl; external 'BearLibTerminal' name 'terminal_clear';

// ClearArea
procedure terminal_clear_area(Left, Top, Width, Height: Integer);
  cdecl; external 'BearLibTerminal' name 'terminal_clear_area';

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

// Print
function terminal_print(X, Y: Integer; const s: PChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_print8';

function terminal_print(X, Y: Integer; const S: string): Cardinal;

function terminal_print(X, Y: Integer; const s: PUnicodeChar): Cardinal;
  cdecl; external 'BearLibTerminal' name 'terminal_print16';

function terminal_print(X, Y: Integer; const S: UnicodeString): Cardinal;

// HasInput
function terminal_has_input(): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_has_input';

// State
function terminal_state(Code: Integer): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_state';

// Read
function terminal_read(): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_read';

// ReadExt
function terminal_read_ext(Flags: Integer): Integer;
  cdecl; external 'BearLibTerminal' name 'terminal_read_ext';

// ColorFromName
function color_from_name(const Name: PChar): Color;
  cdecl; external 'BearLibTerminal' name 'color_from_name8';

function color_from_name(const Name: String): Color;

function color_from_name(const Name: PUnicodeChar): Color;
  cdecl; external 'BearLibTerminal' name 'color_from_name16';

function color_from_name(const Name: UnicodeString): Color;

// ColorFromARGB
function color_from_argb(a, r, g, b: Integer): Color;

implementation

function terminal_set(const Options: String): Integer;
begin
    terminal_set := terminal_set(PChar(Options));
end;

function terminal_set(const Options: UnicodeString): Integer;
begin
    terminal_set := terminal_set(PUnicodeChar(Options));
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

function terminal_print(X, Y: Integer; const S: string): Cardinal;
begin
    terminal_print := terminal_print(X, Y, PChar(S));
end;

function terminal_print(X, Y: Integer; const S: UnicodeString): Cardinal;
begin
     terminal_print := terminal_print(X, Y, PUnicodeChar(S));
end;

function color_from_name(const Name: string): Color;
begin
    color_from_name := color_from_name(PChar(Name));
end;

function color_from_name(const Name: UnicodeString): Color;
begin
    color_from_name := color_from_name(PUnicodeChar(Name));
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
