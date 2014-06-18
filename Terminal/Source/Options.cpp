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
		window_size(0, 0),
		window_cellsize(0, 0),
		window_title(L"BearLibTerminal"),
		window_icon(L":default_icon"),
		window_resizeable(false),
		window_minimum_size(1, 1),
		window_fullscreen(false),
		output_postformatting(true),
		output_asynchronous(true),
		output_vsync(true),
		input_keyboard(true),
		input_mouse(true),
		input_precise_mouse(false),
		input_sticky_close(true),
		input_cursor_symbol((uint16_t)'_'),
		input_cursor_blink_rate(500),
		log_filename(g_logger->GetFile()),
		log_level(g_logger->GetLevel()),
		log_mode(g_logger->GetMode())
	{ }
}
