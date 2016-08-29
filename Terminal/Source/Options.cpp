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

#include "Options.hpp"
#include "Keystroke.hpp"

namespace BearLibTerminal
{
	Options::Options():
		terminal_encoding_affects_put(true),
		window_size(0, 0),
		window_cellsize(0, 0),
		window_title(L"BearLibTerminal"),
		window_icon(L":default_icon"),
		window_resizeable(false),
		window_minimum_size(1, 1),
		window_fullscreen(false),
		output_postformatting(true),
		output_vsync(true),
		input_precise_mouse(false),
		input_cursor_symbol('_'),
		input_cursor_blink_rate(500),
		input_mouse_cursor(true),
		input_alt_functions(true),
		log_filename(L"bearlibterminal.log"), // FIXME: dependency failure
		log_level(Log::Level::Error),
		log_mode(Log::Mode::Truncate)
	{ }
}
