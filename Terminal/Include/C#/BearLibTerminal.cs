/*
* BearLibTerminal C# wrapper
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
* Release date: 2013-12-27
*/

using System;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace BearLib
{
    public static class Terminal
    {
        public struct Keys
        {
            public const int LeftButton     = 0x01;
            public const int RightButton    = 0x02;
            public const int Close          = 0x03; // Same code as VK_CANCEL from winuser.h
            public const int Back           = 0x08; // Backspace
            public const int Backspace      = 0x08; // Backspace (alias)
            public const int Tab            = 0x09; 
            public const int Return         = 0x0D; // Enter
            public const int Shift          = 0x10;
            public const int Control        = 0x11;
            public const int Pause          = 0x13; // Pause/Break
            public const int Escape         = 0x1B;
            public const int Space          = 0x20;
            public const int Prior          = 0x21; // Page Up
            public const int Next           = 0x22; // Page Down
            public const int End            = 0x23;
            public const int Home           = 0x24;
            public const int Left           = 0x25; // Left arrow
            public const int Up             = 0x26; // Up arrow
            public const int Right          = 0x27; // Right arrow
            public const int Down           = 0x28; // Down arrow
            public const int Insert         = 0x2D;
            public const int Delete         = 0x2E;
            public const int D0             = 0x30;
            public const int D1             = 0x31;
            public const int D2             = 0x32;
            public const int D3             = 0x33;
            public const int D4             = 0x34;
            public const int D5             = 0x35;
            public const int D6             = 0x36;
            public const int D7             = 0x37;
            public const int D8             = 0x38;
            public const int D9             = 0x39;
            public const int A              = 0x41;
            public const int B              = 0x42;
            public const int C              = 0x43;
            public const int D              = 0x44;
            public const int E              = 0x45;
            public const int F              = 0x46;
            public const int G              = 0x47;
            public const int H              = 0x48;
            public const int I              = 0x49;
            public const int J              = 0x4A;
            public const int K              = 0x4B;
            public const int L              = 0x4C;
            public const int M              = 0x4D;
            public const int N              = 0x4E;
            public const int O              = 0x4F;
            public const int P              = 0x50;
            public const int Q              = 0x51;
            public const int R              = 0x52;
            public const int S              = 0x53;
            public const int T              = 0x54;
            public const int U              = 0x55;
            public const int V              = 0x56;
            public const int W              = 0x57;
            public const int X              = 0x58;
            public const int Y              = 0x59;
            public const int Z              = 0x5A;
            public const int Grave          = 0xC0; //  ` 
            public const int Minus          = 0xBD; //  -
            public new const int Equals     = 0xBB; //  =
            public const int Backslash      = 0xDC; //  \ .
            public const int LeftBracket    = 0xDB; //  [
            public const int RightBracket   = 0xDD; //  ]
            public const int Semicolon      = 0xBA; //  ;
            public const int Apostrophe     = 0xDE; //  '
            public const int Comma          = 0xBC; //  ,
            public const int Period         = 0xBE; //  .
            public const int Slash          = 0xBF; //  /
            public const int Numpad0        = 0x60;
            public const int Numpad1        = 0x61;
            public const int Numpad2        = 0x62;
            public const int Numpad3        = 0x63;
            public const int Numpad4        = 0x64;
            public const int Numpad5        = 0x65;
            public const int Numpad6        = 0x66;
            public const int Numpad7        = 0x67;
            public const int Numpad8        = 0x68;
            public const int Numpad9        = 0x69;
            public const int Multiply       = 0x6A; // '*' on numpad
            public const int Add            = 0x6B; // '+' on numpad
            public const int Subtract       = 0x6D; // '-' on numpad
            public const int Decimal        = 0x6E; // '.' on numpad
            public const int Divide         = 0x6F; // '/' on numpad
            public const int F1             = 0x70;
            public const int F2             = 0x71;
            public const int F3             = 0x72;
            public const int F4             = 0x73;
            public const int F5             = 0x74;
            public const int F6             = 0x75;
            public const int F7             = 0x76;
            public const int F8             = 0x77;
            public const int F9             = 0x78;
            public const int F10            = 0x79;
            public const int F11            = 0x7A;
            public const int F12            = 0x7B;
            public const int FlagReleased   = 0x100; // This flag is set if key was released instead of pressed
        }
        
        public struct Events
        {
            public const int MouseMove      = 0xD3; // Mouse movement event
            public const int MouseScroll    = 0xD4; // Mouse wheel scroll event
            public const int WindowResize   = 0xDF; // Window resize
        }

        public struct States
        {
            public const int MouseX	        = 0xD0; // Mouse cursor position in characters
            public const int MouseY         = 0xD1;
            public const int MousePixelX    = 0xD5; // Mouse cursor position in pixels
            public const int MousePixelY    = 0xD6;
            public const int MouseWheel     = 0xD2;
            public const int CellWidth      = 0xD7; // Character cell size in pixels
            public const int CellHeight     = 0xD8;
            public const int Width			= 0xD9; // Screen size in cells
            public const int Height			= 0xDA;
            public const int Composition	= 0xC1; // Current composition state
			public const int Color			= 0xC2; // Current foregroung color
			public const int BkColor		= 0xC3; // Current background color
			public const int Layer			= 0xC4; // Current layer
        }
        
        public struct ReadFlags
        {
        	public const int Character      = 1;
        	public const int NoRemove       = 2;
        }

        public struct CompositionMode
        {
            public const int Off            = 0;
            public const int On             = 1;
        }
        
        public struct InputResult
        {
        	public const int None           =  0;
        	public const int Cancelled      = -1;
        	public const int CallAgain      = -2;
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_open", CallingConvention=CallingConvention.Cdecl)]
        public static extern bool Open();

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_close", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Close();

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_refresh", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Refresh();

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_set16", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool Set(string options);

        public static bool Set(string options, params object[] args)
        {
        	Dictionary<Bitmap, BitmapData> bitmaps = new Dictionary<Bitmap, BitmapData>();
        	for (int i=0; i<args.Length; i++)
        	{
        		if (args[i] is Bitmap)
        		{
        			Bitmap bitmap = args[i] as Bitmap;
        			BitmapData data = bitmap.LockBits
					(
						new Rectangle(0, 0, bitmap.Width, bitmap.Height),
						ImageLockMode.ReadOnly,
						PixelFormat.Format32bppArgb
					);
					bitmaps[bitmap] = data;
					args[i] = string.Format("0x{0:X}", (System.UInt64)data.Scan0.ToInt64());
        		}
        	}
        	bool result = Set(string.Format(options, args));
        	foreach(KeyValuePair<Bitmap, BitmapData> i in bitmaps)
        	{
        		i.Key.UnlockBits(i.Value);
        	}
            return result;
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_clear", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Clear();

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_clear_area", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ClearArea(int x, int y, int w, int h);

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_color", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ColorImpl(int argb);

        public static void Color(Color color)
        {
            ColorImpl(color.ToArgb());
        }
        
        public static void Color(string name)
        {
        	ColorImpl(ColorFromNameImpl(name));
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_bkcolor", CallingConvention=CallingConvention.Cdecl)]
        private static extern void BkColorImpl(int argb);

        public static void BkColor(Color color)
        {
            BkColorImpl(color.ToArgb());
        }
        
        public static void BkColor(string name)
        {
        	BkColorImpl(ColorFromNameImpl(name));
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_composition", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Composition(int mode);
        
        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_layer", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Layer(int mode);

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_put", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Put(int x, int y, int code);

        public static void Put(int x, int y, char code)
        {
            Put(x, y, (int)code);
        }
        
        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_put_ext", CallingConvention = CallingConvention.Cdecl)]
        private static extern void PutExtImpl(int x, int y, int dx, int dy, int code, [MarshalAs(UnmanagedType.LPArray)]System.Int32[] corners);
        
        public static void PutExt(int x, int y, int dx, int dy, int code)
        {
        	PutExtImpl(x, y, dx, dy, code, null);
        }
        
        public static void PutExt(int x, int y, int dx, int dy, char code)
        {
        	PutExt(x, y, dx, dy, (int)code);
        }
        
        public static void PutExt(int x, int y, int dx, int dy, int code, Color[] corners)
        {
        	System.Int32[] values = new System.Int32[4];
        	for (int i=0; i<4; i++) values[i] = corners[i].ToArgb();
        	PutExtImpl(x, y, dx, dy, code, values);
        }
        
        public static void PutExt(int x, int y, int dx, int dy, char code, Color[] corners)
        {
        	PutExt(x, y, dx, dy, (int)code, corners);
        }

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_print16", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Print(int x, int y, string text);

        public static int Print(int x, int y, string text, params object[] args)
        {
            return Print(x, y, string.Format(text, args));
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_has_input", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool HasInput();
        
        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_state", CallingConvention = CallingConvention.Cdecl)]
        public static extern int State(int code);
        
        public static bool Check(int code)
        {
            return State(code) == 1;
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_read", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Read();
        
        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_read_ext", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ReadExt(int flags);

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_read_str16", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ReadStr(int x, int y, StringBuilder text, int max);

        public static int ReadStr(int x, int y, ref string text, int max)
        {
            StringBuilder buffer = new StringBuilder(text, max);
            int result = ReadStr(x, y, buffer, max);
            text = buffer.ToString();
            return result;
        }

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "color_from_name16", CallingConvention=CallingConvention.Cdecl)]
        private static extern int ColorFromNameImpl(string name);

        public static Color ColorFromName(string name)
        {
            return System.Drawing.Color.FromArgb(ColorFromNameImpl(name));
        }
    }
}
