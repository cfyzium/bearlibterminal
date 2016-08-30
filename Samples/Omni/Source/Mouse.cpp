/*
 * Mouse.cpp
 *
 *  Created on: Dec 3, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <sstream>

struct
{
	int x, y, width, height;
}
double_click_area = {1, 11, 17, 4}; // FIXME: common rectangle struct

void TestMouse()
{
	terminal_set("window.title='Omni: mouse input'");
	terminal_set("input.filter={keyboard, mouse+}");
	terminal_composition(TK_ON);

	bool precise_mouse = false;
	bool cursor_visible = true;
	int mlx = -1, mly = -1;
	int mrx = -1, mry = -1;
	int scroll = 0;
	bool plate = false;

	// Flush input
	while (terminal_has_input())
	{
		terminal_read();
	}

	int counter = 0;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();

		terminal_color("white");
		terminal_printf(1, 1, "Received [color=orange]%d[/color] %s", counter, counter == 1? "event": "events");

		terminal_color("white");
		terminal_printf
		(
			1, 3,
			"Buttons: "
			"[color=%s]left "
			"[color=%s]middle "
			"[color=%s]right "
			"[color=%s]x1 "
			"[color=%s]x2 ",
			terminal_state(TK_MOUSE_LEFT)? "orange": "dark gray",
			terminal_state(TK_MOUSE_MIDDLE)? "orange": "dark gray",
			terminal_state(TK_MOUSE_RIGHT)? "orange": "dark gray",
			terminal_state(TK_MOUSE_X1)? "orange": "dark gray",
			terminal_state(TK_MOUSE_X2)? "orange": "dark gray"
		);

		int n = terminal_printf
		(
			1, 4,
			"Cursor: [color=orange]%d:%d[/color] [color=dark gray]cells[/color]"
			", [color=orange]%d:%d[/color] [color=dark gray]pixels[/color]",
			terminal_state(TK_MOUSE_X),
			terminal_state(TK_MOUSE_Y),
			terminal_state(TK_MOUSE_PIXEL_X),
			terminal_state(TK_MOUSE_PIXEL_Y)
		);

		terminal_printf
		(
			1, 5,
			"Wheel: [color=orange]%d[/color] [color=dark gray]delta[/color]"
			", [color=orange]%d[/color] [color=dark gray]cumulative",
			terminal_state(TK_MOUSE_WHEEL), scroll
		);

		terminal_printf
		(
			1, 7, "[color=%s][U+25CF][/color] Precise mouse movement",
			precise_mouse? "orange": "black"
		);
		terminal_put(1, 7, 0x25CB);

		terminal_printf
		(
			1, 8, "[color=%s][U+25CF][/color] Mouse cursor is visible",
			cursor_visible? "orange": "black"
		);
		terminal_put(1, 8, 0x25CB);

		terminal_print(double_click_area.x, double_click_area.y-1, "Double-click here:");
		terminal_color(plate? "darker orange": "darker gray");
		for (int x=double_click_area.x; x<=double_click_area.x+double_click_area.width; x++)
		{
			for (int y=double_click_area.y; y<=double_click_area.y+double_click_area.height; y++)
			{
				terminal_put(x, y, 0x2588); // FIXME: fill_area
			}
		}

		int mx = terminal_state(TK_MOUSE_X);
		int my = terminal_state(TK_MOUSE_Y);
		terminal_color(0x60FFFFFF);
		for (int x = 0; x < 80; x++) terminal_put(x, my, 0x2588);
		for (int y = 0; y < 25; y++) if (y != my) terminal_put(mx, y, 0x2588);

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
			else if (code == TK_MOUSE_LEFT)
			{
				int x = terminal_state(TK_MOUSE_X);
				int y = terminal_state(TK_MOUSE_Y);

				if (x == 1 && (y == 7 || y == 8))
				{
					if (y == 7)
					{
						precise_mouse = !precise_mouse;
						terminal_setf("input.precise-mouse=%s", precise_mouse? "true": "false");
					}
					else
					{
						cursor_visible = !cursor_visible;
						terminal_setf("input.mouse-cursor=%s", cursor_visible? "true": "false");
					}
				}
				else if (x >= double_click_area.x && x <= (double_click_area.x+double_click_area.width) &&
						 y >= double_click_area.y && y <= (double_click_area.y+double_click_area.height))
				{
					int clicks = terminal_state(TK_MOUSE_CLICKS);
					if (clicks > 0 && clicks%2 == 0)
					{
						plate = !plate;
					}
				}
				else
				{
					mlx = x;
					mly = y;
				}
			}
			else if (code == TK_MOUSE_RIGHT)
			{
				mrx = terminal_state(TK_MOUSE_X);
				mry = terminal_state(TK_MOUSE_Y);
			}
			else if (code == TK_MOUSE_SCROLL)
			{
				scroll += terminal_state(TK_MOUSE_WHEEL);
			}
			else if (code == TK_SPACE)
			{
				cursor_visible = !cursor_visible;
				terminal_setf("input.mouse-cursor=%s", cursor_visible? "true": "false");
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_color("white");
	terminal_composition(TK_OFF);
	terminal_set("input: precise-mouse=false, mouse-cursor=true, filter={keyboard}");
}
