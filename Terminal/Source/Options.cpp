/*
 * Options.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#include "Options.hpp"

namespace BearLibTerminal
{
	Options::Options():
		window_size(0, 0),
		window_cellsize(0, 0),
		window_title(L"BearLibTerminal"),
		window_icon(L":default_icon"),
		output_postformatting(true),
		output_synchronous(false),
		input_nonblocking(false),
		input_events(InputEvents::KeyPress),
		input_precise_mouse(false),
		input_sticky_close(true),
		input_cursor_symbol((uint16_t)'_'),
		input_cursor_blink_rate(500),
		log_filename(g_log.GetFile()),
		log_level(g_log.GetLevel()),
		log_mode(g_log.GetMode())
	{ }
}
