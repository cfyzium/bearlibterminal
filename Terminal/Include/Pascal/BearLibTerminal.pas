{*
* BearLibTerminal
* Copyright (C) 2013-2017 Cfyz, Apromix
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
*}

{$H+}

unit BearLibTerminal;

interface

uses
  Types;

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
  TK_ALT             = $72;

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
  
  // Text alignment.
  TK_ALIGN_DEFAULT    =   0;
  TK_ALIGN_LEFT       =   1;
  TK_ALIGN_RIGHT      =   2;
  TK_ALIGN_CENTER     =   3;
  TK_ALIGN_TOP        =   4;
  TK_ALIGN_BOTTOM     =   8;
  TK_ALIGN_MIDDLE     =  12;

// ----------------------------------------------------------------------------
// Module interface
// ----------------------------------------------------------------------------

{$IFNDEF FPC}
type
  Int32 = Integer;
  PInt32 = ^Integer;
  UInt32 = Cardinal;
  PUInt32 = ^Cardinal;
{$ENDIF}

type
  TSize = Types.TSize;

// Open
function terminal_open(): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_open';

// Close
procedure terminal_close();
  cdecl; external 'BearLibTerminal' name 'terminal_close';

// Set
function terminal_set(const Options: AnsiString): LongBool; overload;

function terminal_set(const Options: WideString): LongBool; overload;

// Refresh
procedure terminal_refresh();
  cdecl; external 'BearLibTerminal' name 'terminal_refresh';

// Clear
procedure terminal_clear();
  cdecl; external 'BearLibTerminal' name 'terminal_clear';

// ClearArea
procedure terminal_clear_area(Left, Top, Width, Height: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_clear_area';

// Crop
procedure terminal_crop(Left, Top, Width, Height: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_crop';

// Color
procedure terminal_color(Color: UInt32); overload;
  cdecl; external 'BearLibTerminal' name 'terminal_color';
  
procedure terminal_color(Color: AnsiString); overload;

procedure terminal_color(Color: WideString); overload;

// BkColor
procedure terminal_bkcolor(Color: UInt32); overload;
  cdecl; external 'BearLibTerminal' name 'terminal_bkcolor';

procedure terminal_bkcolor(Color: AnsiString); overload;

procedure terminal_bkcolor(Color: WideString); overload;

// Composition
procedure terminal_composition(Mode: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_composition';

// Layer
procedure terminal_layer(Mode: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_layer';

// Font
procedure terminal_font(const Name: AnsiString); overload;

procedure terminal_font(const Name: WideString); overload;

// Put
procedure terminal_put(X, Y, Code: Int32); overload;

procedure terminal_put(X, Y: Int32; Code: AnsiChar); overload;

procedure terminal_put(X, Y: Int32; Code: WideChar); overload;

// PutExt
procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: Int32; Corners: PUInt32); overload;
  cdecl; external 'BearLibTerminal' name 'terminal_put_ext';

procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: WideChar; Corners: PUInt32); overload;

procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: Int32); overload;

procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: WideChar); overload;

// Pick
function terminal_pick(X, Y, Index: Int32): Int32; overload;
  cdecl; external 'BearLibTerminal' name 'terminal_pick';

function terminal_pick(X, Y: Int32): Int32; overload;

// PickColor
function terminal_pick_color(X, Y, Index: Int32): UInt32; overload;
  cdecl; external 'BearLibTerminal' name 'terminal_pick_color';

function terminal_pick_color(X, Y: Int32): UInt32; overload;

// PickBkColor
function terminal_pick_bkcolor(X, Y: Int32): UInt32;
  cdecl; external 'BearLibTerminal' name 'terminal_pick_bkcolor';

// Print
function terminal_print(X, Y: Int32; const S: AnsiString): TSize; overload;

function terminal_print(X, Y: Int32; const S: WideString): TSize; overload;

function terminal_print(X, Y, Alignment: Int32; const S: AnsiString): TSize; overload;

function terminal_print(X, Y, Alignment: Int32; const S: WideString): TSize; overload;

function terminal_print(X, Y, W, H, Alignment: Int32; const S: AnsiString): TSize; overload;

function terminal_print(X, Y, W, H, Alignment: Int32; const S: WideString): TSize; overload;

// Measure
function terminal_measure(const S: AnsiString): TSize; overload;

function terminal_measure(const S: WideString): TSize; overload;

function terminal_measure(W, H: Int32; const S: AnsiString): TSize; overload;

function terminal_measure(W, H: Int32; const S: WideString): TSize; overload;

// HasInput
function terminal_has_input(): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_has_input';

// State
function terminal_state(Code: Int32): Int32;
  cdecl; external 'BearLibTerminal' name 'terminal_state';

// Check
function terminal_check(Code: Int32): Boolean;

// Read
function terminal_read(): Int32;
  cdecl; external 'BearLibTerminal' name 'terminal_read';

// ReadStr
function terminal_read_str(X, Y: Int32; var S: AnsiString; MaxLength: Int32): Int32; overload;

function terminal_read_str(X, Y: Int32; var S: WideString; MaxLength: Int32): Int32; overload;
  
// Peek
function terminal_peek(): Int32;
  cdecl; external 'BearLibTerminal' name 'terminal_peek';

// Delay
procedure terminal_delay(Period: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_delay';

// Get
function terminal_get(const S: AnsiString): AnsiString; overload;

function terminal_get(const S, Default: AnsiString): AnsiString; overload;

function terminal_get(const S: WideString): WideString; overload;

function terminal_get(const S, Default: WideString): WideString; overload;

// ColorFromName
function color_from_name(const Name: AnsiString): UInt32; overload;

function color_from_name(const Name: WideString): UInt32; overload;

// ColorFromARGB
function color_from_argb(a, r, g, b: Int32): UInt32;

// ----------------------------------------------------------------------------
// Module implementation
// ----------------------------------------------------------------------------

implementation

function terminal_set_ansi(const Options: PAnsiChar): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_set8';

function terminal_set(const Options: AnsiString): LongBool;
begin
    terminal_set := terminal_set_ansi(PAnsiChar(Options));
end;

function terminal_set_unicode(const Options: PWideChar): LongBool;
  cdecl; external 'BearLibTerminal' name 'terminal_set16';

function terminal_set(const Options: WideString): LongBool;
begin
    terminal_set := terminal_set_unicode(PWideChar(Options));
end;

procedure terminal_color(Color: AnsiString); overload;
begin
    terminal_color(color_from_name(Color));
end;

procedure terminal_color(Color: WideString); overload;
begin
    terminal_color(color_from_name(Color));
end;

procedure terminal_bkcolor(Color: AnsiString); overload;
begin
    terminal_bkcolor(color_from_name(Color));
end;

procedure terminal_bkcolor(Color: WideString); overload;
begin
    terminal_bkcolor(color_from_name(Color));
end;

procedure terminal_font_ansi(const Name: PAnsiChar);
  cdecl; external 'BearLibTerminal' name 'terminal_font8';

procedure terminal_font_unicode(const Name: PWideChar);
  cdecl; external 'BearLibTerminal' name 'terminal_font16';

procedure terminal_font(const Name: AnsiString); overload;
begin
	terminal_font_ansi(PAnsiChar(Name));
end;

procedure terminal_font(const Name: WideString); overload;
begin
	terminal_font_unicode(PWideChar(Name));
end;

procedure terminal_put_integer(X, Y: Int32; Code: Int32);
  cdecl; external 'BearLibTerminal' name 'terminal_put';

procedure terminal_put(X, Y, Code: Int32);
begin
    terminal_put_integer(X, Y, Code);
end;

procedure terminal_put(X, Y: Int32; Code: AnsiChar);
begin
    terminal_put_integer(X, Y, ord(Code));
end;

procedure terminal_put(X, Y: Int32; Code: WideChar);
begin
    terminal_put_integer(X, Y, ord(Code));
end;

procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: WideChar; Corners: PUInt32); overload;
begin
    terminal_put_ext(X, Y, dX, dY, ord(Code), Corners);
end;

procedure terminal_put_ext(X, Y, dX, dY, Code: Int32); overload;
begin
    terminal_put_ext(X, Y, dX, dY, Code, PUInt32(0));
end;

procedure terminal_put_ext(X, Y, dX, dY: Int32; Code: WideChar); overload;
begin
    terminal_put_ext(X, Y, dX, dY, ord(Code), PUInt32(0));
end;

function terminal_pick(X, Y: Int32): Int32; overload;
begin
    terminal_pick := terminal_pick(X, Y, 0);
end;

function terminal_pick_color(X, Y: Int32): UInt32; overload;
begin;
    terminal_pick_color := terminal_pick_color(X, Y, 0);
end;

procedure terminal_print_ansi(X, Y, W, H, Alignment: Int32; const S: PAnsiChar; OutW, OutH: PInt32);
  cdecl; external 'BearLibTerminal' name 'terminal_print_ext8';
  
procedure terminal_print_unicode(X, Y, W, H, Alignment: Int32; const S: PWideChar; OutW, OutH: PInt32);
  cdecl; external 'BearLibTerminal' name 'terminal_print_ext16';

function terminal_print(X, Y: Int32; const S: AnsiString): TSize;
begin
    terminal_print := terminal_print(X, Y, 0, 0, TK_ALIGN_DEFAULT, S);
end;

function terminal_print(X, Y: Int32; const S: WideString): TSize;
begin
    terminal_print := terminal_print(X, Y, 0, 0, TK_ALIGN_DEFAULT, S);
end;

function terminal_print(X, Y, Alignment: Int32; const S: AnsiString): TSize;
begin
    terminal_print := terminal_print(X, Y, 0, 0, Alignment, S);
end;

function terminal_print(X, Y, Alignment: Int32; const S: WideString): TSize;
begin
    terminal_print := terminal_print(X, Y, 0, 0, Alignment, S);
end;

function terminal_print(X, Y, W, H, Alignment: Int32; const S: AnsiString): TSize;
var
    OutW, OutH: Int32;
begin
    terminal_print_ansi(X, Y, W, H, Alignment, PAnsiChar(S), @OutW, @OutH);
    terminal_print.cx := OutW;
    terminal_print.cy := OutH;
end;

function terminal_print(X, Y, W, H, Alignment: Int32; const S: WideString): TSize;
var
    OutW, OutH: Int32;
begin
    terminal_print_unicode(X, Y, W, H, Alignment, PWideChar(S), @OutW, @OutH);
    terminal_print.cx := OutW;
    terminal_print.cy := OutH;
end;

procedure terminal_measure_ansi(W, H: Int32; const S: PAnsiChar; OutW, OutH: PInt32);
  cdecl; external 'BearLibTerminal' name 'terminal_measure_ext8';

procedure terminal_measure_unicode(W, H: Int32; const S: PWideChar; OutW, OutH: PInt32);
  cdecl; external 'BearLibTerminal' name 'terminal_measure_ext16';

function terminal_measure(const S: AnsiString): TSize;
begin
    terminal_measure := terminal_measure(0, 0, S);
end;

function terminal_measure(const S: WideString): TSize;
begin
    terminal_measure := terminal_measure(0, 0, S);
end;

function terminal_measure(W, H: Int32; const S: AnsiString): TSize;
var
    OutW, OutH: Int32;
begin
    terminal_measure_ansi(W, H, PAnsiChar(S), @OutW, @OutH);
    terminal_measure.cx := OutW;
    terminal_measure.cy := OutH;
end;

function terminal_measure(W, H: Int32; const S: WideString): TSize;
var
    OutW, OutH: Int32;
begin
    terminal_measure_unicode(W, H, PWideChar(S), @OutW, @OutH);
    terminal_measure.cx := OutW;
    terminal_measure.cy := OutH;
end;

function terminal_check(Code: Int32): Boolean;
begin
    terminal_check := terminal_state(Code) > 0;
end;

function terminal_read_str_ansi(X, Y: Int32; S: PAnsiChar; MaxLength: Int32): Int32;
  cdecl; external 'BearLibTerminal' name 'terminal_read_str8';

function terminal_read_str(X, Y: Int32; var S: AnsiString; MaxLength: Int32): Int32;
var
    i: Integer;
begin
    SetLength(S, MaxLength*3+1); // Bigger size to accomodate possible UTF-8 string.
    terminal_read_str := terminal_read_str_ansi(X, Y, PAnsiChar(S), MaxLength);
    for i := 1 to MaxLength*3+1 do
        if S[i] = AnsiChar(0) then
            break;
    SetLength(S, i);
end;

function terminal_read_str_unicode(X, Y: Int32; S: PWideChar; MaxLength: Int32): Int32;
  cdecl; external 'BearLibTerminal' name 'terminal_read_str16';

function terminal_read_str(X, Y: Int32; var S: WideString; MaxLength: Int32): Int32;
var
    i: Integer;
begin
    SetLength(S, MaxLength);
    terminal_read_str := terminal_read_str_unicode(X, Y, PWideChar(S), MaxLength);
    for i := 1 to MaxLength do
        if S[i] = WideChar(0) then
            break;
    SetLength(S, i);
end;

function terminal_get_ansi(const S, Default: PAnsiChar): PAnsiChar;
  cdecl; external 'BearLibTerminal' name 'terminal_get8';

function terminal_get_unicode(const S, Default: PWideChar): PWideChar;
  cdecl; external 'BearLibTerminal' name 'terminal_get16';

function terminal_get(const S: AnsiString): AnsiString;
begin
    terminal_get := terminal_get_ansi(PAnsiChar(S), nil);
end;

function terminal_get(const S, Default: AnsiString): AnsiString;
begin
    terminal_get := terminal_get_ansi(PAnsiChar(S), PAnsiChar(Default));
end;

function terminal_get(const S: WideString): WideString;
begin
    terminal_get := terminal_get_unicode(PWideChar(S), nil);
end;

function terminal_get(const S, Default: WideString): WideString;
begin
    terminal_get := terminal_get_unicode(PWideChar(S), PWideChar(Default));
end;

function color_from_name_ansi(const Name: PAnsiChar): UInt32;
  cdecl; external 'BearLibTerminal' name 'color_from_name8';

function color_from_name(const Name: AnsiString): UInt32;
begin
    color_from_name := color_from_name_ansi(PAnsiChar(Name));
end;

function color_from_name_unicode(const Name: PWideChar): UInt32;
  cdecl; external 'BearLibTerminal' name 'color_from_name16';

function color_from_name(const Name: WideString): UInt32;
begin
    color_from_name := color_from_name_unicode(PWideChar(Name));
end;

function color_from_argb(a, r, g, b: Int32): UInt32;
begin
    if a > 255 then a := 255;
    if r > 255 then r := 255;
    if g > 255 then g := 255;
    if b > 255 then b := 255;
    color_from_argb := (a shl 24) or (r shl 16) or (g shl 8) or b;
end;

end.
