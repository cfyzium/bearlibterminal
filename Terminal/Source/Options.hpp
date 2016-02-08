/*
 * Options.hpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include "Size.hpp"
#include "Log.hpp"
#include <string>
#include <atomic>
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
		uint16_t input_cursor_symbol;
		int input_cursor_blink_rate;
		bool input_mouse_cursor;
		std::set<int> input_filter;

		// Log
		std::wstring log_filename;
		Log::Level log_level;
		Log::Mode log_mode;

		Options();
	};
}

#endif /* OPTIONS_HPP_ */
