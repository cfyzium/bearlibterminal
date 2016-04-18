/*
* BearLibTerminal
* Copyright (C) 2013-2016 Cfyz
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

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include "Size.hpp"
#include "Log.hpp"
#include <string>
#include <set>

namespace BearLibTerminal
{
	struct Options
	{
		// Terminal
		std::wstring terminal_encoding;
		bool terminal_encoding_affects_put;

		// Window
		Size window_size;
		Size window_cellsize;
		Size window_client_size;
		std::wstring window_title;
		std::wstring window_icon;
		bool window_resizeable;
		Size window_minimum_size;
		bool window_fullscreen;

		// Output
		bool output_postformatting;
		bool output_vsync;

		// Input
		bool input_precise_mouse;
		bool input_sticky_close;
		char32_t input_cursor_symbol;
		int input_cursor_blink_rate;
		bool input_mouse_cursor;
		std::set<int> input_filter;
		bool input_alt_functions;

		// Log
		std::wstring log_filename;
		Log::Level log_level;
		Log::Mode log_mode;

		Options();
	};
}

#endif /* OPTIONS_HPP_ */
