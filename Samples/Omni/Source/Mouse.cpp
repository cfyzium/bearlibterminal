/*
 * Mouse.cpp
 *
 *  Created on: Dec 3, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <sstream>

void UpdateInputFilter(bool press, bool release, bool move, bool scroll)
{
	std::ostringstream s;
	s << "input.events=";
	if (press) s << "+keypress";
	if (release) s << "+keyrelease";
	if (move) s << "+mousemove";
	if (scroll) s << "+mousescroll";
	terminal_set(s.str().c_str());
}

void UpdatePreciseMovement(bool flag)
{
	std::string s = std::string("input.precise-mousemove=") + (flag? "true": "false");
	terminal_set(s.c_str());
}

void TestMouse()
{
	terminal_set("window.title=Omni: mouse input");
	terminal_composition(TK_ON);

	bool keypress = true;
	bool keyrelease = true;
	bool mousemove = false;
	bool mousescroll = false;
	bool precise_mouse = false;
	int mlx = -1, mly = -1;
	int mrx = -1, mry = -1;

	while (terminal_has_input()) terminal_read(); // Flush input
	UpdateInputFilter(keypress, keyrelease, mousemove, mousescroll);

	int counter = 0;

	for (bool proceed=true; proceed;)
	{
		int mx = terminal_state(TK_MOUSE_X);
		int my = terminal_state(TK_MOUSE_Y);
		int mz = terminal_state(TK_MOUSE_WHEEL);
		int lmb = terminal_state(TK_MOUSE_LEFT);
		int rmb = terminal_state(TK_MOUSE_RIGHT);

		terminal_clear();

		terminal_color("white");
		terminal_printf(1, 1, "Modify input events filter by setting/clearing following flags:");

		terminal_color("orange");
		terminal_put(2, 2, 0x25CF);
		if (keyrelease) terminal_put(2, 3, 0x25CF);
		if (mousemove) terminal_put(2, 4, 0x25CF);
		if (mousescroll) terminal_put(2, 5, 0x25CF);
		if (precise_mouse) terminal_put(2, 6, 0x25CF);

		terminal_color("white");
		terminal_printf(1, 2, " [U+25CB] [color=gray]Key press");
		terminal_printf(1, 3, " [U+25CB] Key release");
		terminal_printf(1, 4, " [U+25CB] Mouse movement");
		terminal_printf(1, 5, " [U+25CB] Mouse scroll");
		terminal_printf(1, 6, " [U+25CB] Precise mouse movement");

		terminal_color("white");
		terminal_printf(1, 10, "Received [color=orange]%d[/color] %s", counter, counter == 1? "event": "events");

		terminal_color("white");
		terminal_printf(1, 12, "Current state: ");
		terminal_printf(1, 13, "LMB: ");
		terminal_printf(1, 14, "RMB: ");
		terminal_printf(1, 15, "Cursor: ");
		terminal_printf(1, 16, "Wheel: ");
		terminal_color("orange");
		terminal_printf(6, 13, "%s", (lmb? "Yes": "No"));
		terminal_printf(6, 14, "%s", (rmb? "Yes": "No"));
		terminal_printf(8, 16, "%d", mz);

		// Cursor position
		int n = 9;
		terminal_color("white");
		n += terminal_printf(n, 15, "([color=orange]%d[/color], [color=orange]%d[/color])", mx, my);
		if (precise_mouse)
		{
			terminal_printf(n, 15, "[color=gray] (%d, %d)", terminal_state(TK_MOUSE_PIXEL_X), terminal_state(TK_MOUSE_PIXEL_Y));
		}

		terminal_printf(1, 18, "[color=orange]TIP:[/color] move and click mouse everywhere");
		terminal_printf(1, 20, "[color=orange]NOTE: [/color]'current state' may not always show");
		terminal_printf(1, 21, "immediate values depending on filter flags");

		if (mousemove)
		{
			terminal_color(0x80FFFFFF);
			for ( int x = 0; x < 80; x++ ) terminal_put(x, my, 0x2588);
			for ( int y = 0; y < 25; y++ ) if ( y != my ) terminal_put(mx, y, 0x2588);
		}

		terminal_color(0x8000FF00);
		terminal_put(mlx, mly, 0x2588);

		terminal_color(0x80FF00FF);
		terminal_put(mrx, mry, 0x2588);

		terminal_refresh();

		do
		{
			int code = terminal_read();
			counter += 1;

			if (code == TK_ESCAPE || code == TK_CLOSE)
			{
				proceed = false;
			}
			else if (code >= TK_2 && code <= TK_4)
			{
				if (code == TK_2) keyrelease = !keyrelease;
				if (code == TK_3) mousemove = !mousemove;
				if (code == TK_4) mousescroll = !mousescroll;
				UpdateInputFilter(keypress, keyrelease, mousemove, mousescroll);
			}
			else if (code == TK_MOUSE_LEFT)
			{
				mx = terminal_state(TK_MOUSE_X);
				my = terminal_state(TK_MOUSE_Y);

				if (mx >= 1 && mx <= 3 && (my >= 3 && my <= 6))
				{
					if (my == 3) keyrelease = !keyrelease;
					if (my == 4) mousemove = !mousemove;
					if (my == 5) mousescroll = !mousescroll;
					if (my == 6) precise_mouse = !precise_mouse;
					UpdateInputFilter(keypress, keyrelease, mousemove, mousescroll);
					UpdatePreciseMovement(precise_mouse);
				}
				else
				{
					mlx = mx;
					mly = my;
				}
			}
			else if (code == TK_MOUSE_RIGHT)
			{
				mrx = terminal_state(TK_MOUSE_X);
				mry = terminal_state(TK_MOUSE_Y);
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_color("white");
	terminal_composition(TK_OFF);
	terminal_set("input.events=keypress; input.precise_mousemove=false");
}
