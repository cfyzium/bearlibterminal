/*
 * TextAlignment.cpp
 *
 *  Created on: Jul 29, 2014
 *      Author: cfyz
 */

#include "Common.hpp"

namespace
{
	enum Alignment
	{
		Left = 1, Center = 2, Right = 3, Top = 1, Bottom = 3
	};

	const char* lorem_ipsum =
		"[c=orange]Lorem[/c] ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
		"tempor incididunt ut labore et dolore magna aliqua. [c=orange]Ut[/c] enim ad minim "
		"veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex "
		"ea commodo consequat. [c=orange]Duis[/c] aute irure dolor in reprehenderit in voluptate "
		"velit esse cillum dolore eu fugiat nulla pariatur. [c=orange]Excepteur[/c] sint occaecat "
		"cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
		"est laborum.";
}

void TestTextAlignment()
{
	terminal_set("window.title='Omni: text alignment'");

	Alignment horisontal_shift[] = {Left, Left, Center, Right, Right};
	Alignment vertical_shift[] = {Top, Top, Center, Bottom, Bottom};
	const char* horisontal_names[] = {"", "left", "center", "right"};
	const char* vertical_names[] = {"", "top", "center", "bottom"};
	Alignment horisontal = Left;
	Alignment vertical = Top;

	int padding_sides = 2;
	int padding_top = 6;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();

		// Background square
		terminal_bkcolor("darkest gray");
		terminal_clear_area
		(
			padding_sides*2,
			padding_top,
			terminal_state(TK_WIDTH) - (padding_sides*2*2),
			terminal_state(TK_HEIGHT) - (padding_sides+padding_top)
		);
		terminal_bkcolor("none");

		// Comment
		terminal_printf
		(
			padding_sides*2,
			padding_sides,
			"Use arrows to change text alignment.\n"
			"Current alignment is [c=orange]%s-%s[/c].",
			vertical_names[(int)vertical], horisontal_names[(int)horisontal]
		);

		// Origin
		int x, y;

		switch (horisontal)
		{
		case Right:
			x = terminal_state(TK_WIDTH) - (padding_sides*2+1);
			break;
		case Center:
			x = terminal_state(TK_WIDTH)/2;
			break;
		default: // Left
			x = padding_sides*2;
			break;
		}

		switch (vertical)
		{
		case Bottom:
			y = terminal_state(TK_HEIGHT) - (padding_sides+1);
			break;
		case Center:
			y = padding_top + (terminal_state(TK_HEIGHT) - padding_top - padding_sides)/2;
			break;
		default: // Top
			y = padding_top;
			break;
		}

		// Wrapped text
		terminal_printf
		(
			x, y,
			"[wrap=%d][align=%s-%s]%s",
			terminal_state(TK_WIDTH) - (padding_sides*2*2+1),
			vertical_names[(int)vertical], horisontal_names[(int)horisontal],
			lorem_ipsum
		);

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_LEFT)
			{
				horisontal = horisontal_shift[(int)horisontal-1];
			}
			else if (key == TK_RIGHT)
			{
				horisontal = horisontal_shift[(int)horisontal+1];
			}
			else if (key == TK_UP)
			{
				vertical = vertical_shift[(int)vertical-1];
			}
			else if (key == TK_DOWN)
			{
				vertical = vertical_shift[(int)vertical+1];
			}
		}
		while (proceed && terminal_has_input());
	}
}
