/*
 * Options.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
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
		input_sticky_close(true),
		input_cursor_symbol('_'),
		input_cursor_blink_rate(500),
		input_mouse_cursor(true),
		input_alt_functions(true),
		log_filename(L"bearlibterminal.log"), // FIXME: dependency failure
		log_level(Log::Level::Error),
		log_mode(Log::Mode::Truncate)
	{ }
}
