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
		input_nonblocking(false),
		input_events(0x01), // FIXME
		input_precise_mouse(false),
		input_cursor_symbol((uint16_t)'_'),
		input_cursor_blink_rate(500)
	{ }
}
