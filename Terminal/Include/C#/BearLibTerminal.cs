/*
* BearLibTerminal C# wrapper
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
        public const int

        // Keyboard scancodes 
        TK_A                = 0x04,
        TK_B                = 0x05,
        TK_C                = 0x06,
        TK_D                = 0x07,
        TK_E                = 0x08,
        TK_F                = 0x09,
        TK_G                = 0x0A,
        TK_H                = 0x0B,
        TK_I                = 0x0C,
        TK_J                = 0x0D,
        TK_K                = 0x0E,
        TK_L                = 0x0F,
        TK_M                = 0x10,
        TK_N                = 0x11,
        TK_O                = 0x12,
        TK_P                = 0x13,
        TK_Q                = 0x14,
        TK_R                = 0x15,
        TK_S                = 0x16,
        TK_T                = 0x17,
        TK_U                = 0x18,
        TK_V                = 0x19,
        TK_W                = 0x1A,
        TK_X                = 0x1B,
        TK_Y                = 0x1C,
        TK_Z                = 0x1D,
        TK_1                = 0x1E,
        TK_2                = 0x1F,
        TK_3                = 0x20,
        TK_4                = 0x21,
        TK_5                = 0x22,
        TK_6                = 0x23,
        TK_7                = 0x24,
        TK_8                = 0x25,
        TK_9                = 0x26,
        TK_0                = 0x27,
        TK_RETURN           = 0x28,
        TK_ENTER            = 0x28,
        TK_ESCAPE           = 0x29,
        TK_BACKSPACE        = 0x2A,
        TK_TAB              = 0x2B,
        TK_SPACE            = 0x2C,
        TK_MINUS            = 0x2D,
        TK_EQUALS           = 0x2E,
        TK_LBRACKET         = 0x2F,
        TK_RBRACKET         = 0x30,
        TK_BACKSLASH        = 0x31,
        TK_SEMICOLON        = 0x33,
        TK_APOSTROPHE       = 0x34,
        TK_GRAVE            = 0x35,
        TK_COMMA            = 0x36,
        TK_PERIOD           = 0x37,
        TK_SLASH            = 0x38,
        TK_F1               = 0x3A,
        TK_F2               = 0x3B,
        TK_F3               = 0x3C,
        TK_F4               = 0x3D,
        TK_F5               = 0x3E,
        TK_F6               = 0x3F,
        TK_F7               = 0x40,
        TK_F8               = 0x41,
        TK_F9               = 0x42,
        TK_F10              = 0x43,
        TK_F11              = 0x44,
        TK_F12              = 0x45,
        TK_PAUSE            = 0x48,
        TK_INSERT           = 0x49,
        TK_HOME             = 0x4A,
        TK_PAGEUP           = 0x4B,
        TK_DELETE           = 0x4C,
        TK_END              = 0x4D,
        TK_PAGEDOWN         = 0x4E,
        TK_RIGHT            = 0x4F,
        TK_LEFT             = 0x50,
        TK_DOWN             = 0x51,
        TK_UP               = 0x52,
        TK_KP_DIVIDE        = 0x54,
        TK_KP_MULTIPLY      = 0x55,
        TK_KP_MINUS         = 0x56,
        TK_KP_PLUS          = 0x57,
        TK_KP_ENTER         = 0x58,
        TK_KP_1             = 0x59,
        TK_KP_2             = 0x5A,
        TK_KP_3             = 0x5B,
        TK_KP_4             = 0x5C,
        TK_KP_5             = 0x5D,
        TK_KP_6             = 0x5E,
        TK_KP_7             = 0x5F,
        TK_KP_8             = 0x60,
        TK_KP_9             = 0x61,
        TK_KP_0             = 0x62,
        TK_KP_PERIOD        = 0x63,
        TK_SHIFT            = 0x70,
        TK_CONTROL          = 0x71,
        TK_ALT              = 0x72,

        // Mouse events/states
        TK_MOUSE_LEFT       = 0x80, // Buttons
        TK_MOUSE_RIGHT      = 0x81,
        TK_MOUSE_MIDDLE     = 0x82,
        TK_MOUSE_X1         = 0x83,
        TK_MOUSE_X2         = 0x84,
        TK_MOUSE_MOVE       = 0x85, // Movement event
        TK_MOUSE_SCROLL     = 0x86, // Mouse scroll event
        TK_MOUSE_X          = 0x87, // Cusor position in cells
        TK_MOUSE_Y          = 0x88,
        TK_MOUSE_PIXEL_X    = 0x89, // Cursor position in pixels
        TK_MOUSE_PIXEL_Y    = 0x8A,
        TK_MOUSE_WHEEL      = 0x8B, // Scroll direction and amount
        TK_MOUSE_CLICKS     = 0x8C, // Number of consecutive clicks

        //If key was released instead of pressed, it's code will be OR'ed withTK_KEY_RELEASED
        TK_KEY_RELEASED     = 0x100,

        // Virtual key-codes for internal terminal states/variables.
        // These can be accessed via terminal_state function.
        TK_WIDTH            = 0xC0, // Terminal width in cells
        TK_HEIGHT           = 0xC1, // Terminal height in cells
        TK_CELL_WIDTH       = 0xC2, // Cell width in pixels
        TK_CELL_HEIGHT      = 0xC3, // Cell height in pixels
        TK_COLOR            = 0xC4, // Current foregroung color
        TK_BKCOLOR          = 0xC5, // Current background color
        TK_LAYER            = 0xC6, // Current layer
        TK_COMPOSITION      = 0xC7, // Current composition state
        TK_CHAR             = 0xC8, // Translated ANSI code of last produced character
        TK_WCHAR            = 0xC9, // Unicode codepoint of last produced character
        TK_EVENT            = 0xCA, // Last dequeued event
        TK_FULLSCREEN       = 0xCB, // Fullscreen state

        // Other events
        TK_CLOSE            = 0xE0,
        TK_RESIZED          = 0xE1,

        // Input result codes for terminal_read function.
        TK_INPUT_NONE       =    0,
        TK_INPUT_CANCELLED  =   -1;
        
        private static string Format(string text, object[] args)
        {
        	if (args != null && args.Length > 0)
        		return string.Format(text, args);
        	else
        		return text;
        }
        
        private static int LibraryAlignmentFromContentAlignment(ContentAlignment alignment)
        {
            switch (alignment)
            {
                case ContentAlignment.TopLeft:
                    return 5;
                case ContentAlignment.TopCenter:
                    return 7;
                case ContentAlignment.TopRight:
                    return 6;
                case ContentAlignment.MiddleLeft:
                    return 13;
                case ContentAlignment.MiddleCenter:
                    return 15;
                case ContentAlignment.MiddleRight:
                    return 14;
                case ContentAlignment.BottomLeft:
                    return 9;
                case ContentAlignment.BottomCenter:
                    return 11;
                case ContentAlignment.BottomRight:
                    return 10;
                default:
                    return 5;
            }
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
                else if (args[i] is Size)
                {
                    Size size = (Size)args[i];
                    args[i] = string.Format("{0}x{1}", size.Width, size.Height);
                }
                else if (args[i] is bool)
                {
                    bool value = (bool)args[i];
                    args[i] = value? "true": "false";
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

        public static void ClearArea(Rectangle area)
        {
            ClearArea(area.X, area.Y, area.Width, area.Height);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_crop", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Crop(int x, int y, int w, int h);

        public static void Crop(Rectangle area)
        {
            Crop(area.X, area.Y, area.Width, area.Height);
        }

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
        private static extern void CompositionImpl(int mode);

        public static void Composition(bool enabled)
        {
        	CompositionImpl(enabled? 1: 0);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_layer", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Layer(int index);

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_font16", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Font(string name);

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_put", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Put(int x, int y, int code);

        public static void Put(Point location, int code)
        {
            Put(location.X, location.Y, code);
        }

        public static void Put(int x, int y, char code)
        {
            Put(x, y, (int)code);
        }

        public static void Put(Point location, char code)
        {
            Put(location.X, location.Y, (int)code);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_put_ext", CallingConvention = CallingConvention.Cdecl)]
        private static extern void PutExtImpl(int x, int y, int dx, int dy, int code, [MarshalAs(UnmanagedType.LPArray)]System.Int32[] corners);

        public static void PutExt(int x, int y, int dx, int dy, int code)
        {
            PutExtImpl(x, y, dx, dy, code, null);
        }

        public static void PutExt(Point location, Point offset, int code)
        {
            PutExtImpl(location.X, location.Y, offset.X, offset.Y, code, null);
        }

        public static void PutExt(int x, int y, int dx, int dy, char code)
        {
            PutExtImpl(x, y, dx, dy, (int)code, null);
        }

        public static void PutExt(Point location, Point offset, char code)
        {
            PutExtImpl(location.X, location.Y, offset.X, offset.Y, (int)code, null);
        }

        public static void PutExt(int x, int y, int dx, int dy, int code, Color[] corners)
        {
            System.Int32[] values = new System.Int32[4];
            for (int i=0; i<4; i++) values[i] = corners[i].ToArgb();
            PutExtImpl(x, y, dx, dy, code, values);
        }

        public static void PutExt(Point location, Point offset, int code, Color[] corners)
        {
            PutExt(location.X, location.Y, offset.X, offset.Y, code, corners);
        }

        public static void PutExt(int x, int y, int dx, int dy, char code, Color[] corners)
        {
            PutExt(x, y, dx, dy, (int)code, corners);
        }

        public static void PutExt(Point location, Point offset, char code, Color[] corners)
        {
            PutExt(location.X, location.Y, offset.X, offset.Y, (int)code, corners);
        }

        public static int Pick(int x, int y)
        {
            return Pick(x, y, 0);
        }

        public static int Pick(Point location)
        {
            return Pick(location.X, location.Y, 0);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_pick", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Pick(int x, int y, int index);

        public static int Pick(Point location, int index)
        {
            return Pick(location.X, location.Y, index);
        }

        public static Color PickColor(int x, int y)
        {
            return PickColor(x, y, 0);
        }

        public static Color PickColor(Point location)
        {
            return PickColor(location.X, location.Y, 0);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_pick_color", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PickColorImpl(int x, int y, int index);

        public static Color PickColor(int x, int y, int index)
        {
            return System.Drawing.Color.FromArgb(PickColorImpl(x, y, index));
        }

        public static Color PickColor(Point location, int index)
        {
            return PickColor(location.X, location.Y, index);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_pick_bkcolor", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PickBkColorImpl(int x, int y);

        public static Color PickBkColor(int x, int y)
        {
            return System.Drawing.Color.FromArgb(PickBkColorImpl(x, y));
        }

        public static Color PickBkColor(Point location)
        {
            return PickBkColor(location.X, location.Y);
        }

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_print_ext16", CallingConvention = CallingConvention.Cdecl)]
        private static extern void PrintImpl(int x, int y, int w, int h, int align, string text, out int out_w, out int out_h);
        
        public static Size Print(Rectangle layout, ContentAlignment alignment, string text, params object[] args)
        {
        	int width, height;
        	PrintImpl(layout.X, layout.Y, layout.Width, layout.Height, LibraryAlignmentFromContentAlignment(alignment), Format(text, args), out width, out height);
        	return new Size(width, height);
        }
        
        public static Size Print(Rectangle layout, string text, params object[] args)
        {
        	return Print(layout, ContentAlignment.TopLeft, text, args);
        }
        
        public static Size Print(Point location, ContentAlignment alignment, string text, params object[] args)
        {
        	return Print(location.X, location.Y, alignment, text, args);
        }
        
        public static Size Print(Point location, string text, params object[] args)
        {
        	return Print(location.X, location.Y, text, args);
        }
        
        public static Size Print(int x, int y, ContentAlignment alignment, string text, params object[] args)
        {
        	int width, height;
        	PrintImpl(x, y, 0, 0, LibraryAlignmentFromContentAlignment(alignment), Format(text, args), out width, out height);
        	return new Size(width, height);
        }
        
        public static Size Print(int x, int y, string text, params object[] args)
        {
        	int width, height;
        	PrintImpl(x, y, 0, 0, 0, Format(text, args), out width, out height);
        	return new Size(width, height);
        }  

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_measure_ext16", CallingConvention = CallingConvention.Cdecl)]
        private static extern void MeasureImpl(int width, int height, string text, out int out_w, out int out_h);
        
        public static Size Measure(Size bbox, string text, params object[] args)
        {
        	int width, height;
        	MeasureImpl(bbox.Width, bbox.Height, Format(text, args), out width, out height);
        	return new Size(width, height);
        }
        
        public static Size Measure(string text, params object[] args)
        {
        	return Measure(new Size(), text, args);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_has_input", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool HasInput();

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_state", CallingConvention = CallingConvention.Cdecl)]
        public static extern int State(int code);

        public static bool Check(int code)
        {
            return State(code) > 0;
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_read", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Read();

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_read_str16", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ReadStr(int x, int y, StringBuilder text, int max);

        public static int ReadStr(Point location, StringBuilder text, int max)
        {
            return ReadStr(location.X, location.Y, text, max);
        }

        public static int ReadStr(int x, int y, ref string text, int max)
        {
            StringBuilder buffer = new StringBuilder(text, max);
            int result = ReadStr(x, y, buffer, max);
            text = buffer.ToString();
            return result;
        }

        public static int ReadStr(Point location, ref string text, int max)
        {
            return ReadStr(location.X, location.Y, ref text, max);
        }

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_peek", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Peek();

        [DllImport("BearLibTerminal.dll", EntryPoint = "terminal_delay", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Delay(int period);

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "terminal_get16", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetImpl(string key, string default_value);

        public static string Get(string name, string default_value = "")
        {
            return Marshal.PtrToStringUni(GetImpl(name, default_value));
        }

        private static object ParseSize(string s)
        {
            string[] parts = s.Split('x');
            return new Size(int.Parse(parts[0]), int.Parse(parts[1]));
        }

        private static object ParseBool(string s)
        {
            return s == "true";
        }

        public static T Get<T>(string name, T default_value = default(T))
        {
            string result_str = Get(name);
            if (result_str.Length == 0)
            {
                return default_value;
            }
            else
            {
                try
                {
                    Type type = typeof(T);
                    
                    if (type == typeof(Size))
                    {
                        return (T)ParseSize(result_str);
                    }
                    else if (type == typeof(bool))
                    {
                        return (T)ParseBool(result_str);
                    }
                    else if (type.IsPrimitive)
                    {
                        return (T)Convert.ChangeType(result_str, typeof(T));
                    }
                    else if (type.IsEnum)
                    {
                        return (T)System.Enum.Parse(type, result_str);
                    }
                    else
                    {
                        return (T)System.Activator.CreateInstance(type, result_str);
                    }
                }
                catch
                {
                    return default_value;
                }
            }
        }

        [DllImport("BearLibTerminal.dll", CharSet = CharSet.Unicode, EntryPoint = "color_from_name16", CallingConvention=CallingConvention.Cdecl)]
        private static extern int ColorFromNameImpl(string name);

        public static Color ColorFromName(string name)
        {
            return System.Drawing.Color.FromArgb(ColorFromNameImpl(name));
        }
    }
}
