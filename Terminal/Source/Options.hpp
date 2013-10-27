/*
 * Options.hpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include "Size.hpp"
#include <string>
#include <atomic>

namespace BearLibTerminal
{
	struct Options
	{
		// Terminal
		std::wstring terminal_encoding;

		// Window
		Size window_size;
		Size window_cellsize;
		std::wstring window_title;
		std::wstring window_icon;

		// Output
		bool output_postformatting;
		bool output_synchronous;

		// Input
		bool input_nonblocking;
		uint32_t input_events;
		bool input_precise_mouse;
		bool input_sticky_close;
		uint16_t input_cursor_symbol;
		int input_cursor_blink_rate;

		// Log
		// std::wstring log_filename
		// Log::Level log_level

		Options();
	};

	struct InputEvents
	{
		static const uint32_t
			None		= 0x00,
			KeyPress	= 0x01,
			KeyRelease 	= 0x02,
			MouseMove 	= 0x04,
			MouseScroll	= 0x08,
			All			= 0x0F;
	};
}

#endif /* OPTIONS_HPP_ */
