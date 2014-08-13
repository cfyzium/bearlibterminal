/*
 * TextAlignment.cpp
 *
 *  Created on: Jul 29, 2014
 *      Author: cfyz
 */

#include "Common.hpp"

#include <cmath>

namespace { // anonymous

enum Alignment
{
	Left   = 1,
	Center = 2,
	Right  = 3,
	Top    = 1,
	Bottom = 3
};

const char* lorem_ipsum =
	"[c=orange]Lorem[/c] ipsum dolor sit amet, consectetur adipisicing elit, "
	"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
	"[c=orange]Ut[/c] enim ad minim veniam, quis nostrud exercitation ullamco "
	"laboris nisi ut aliquip ex ea commodo consequat. [c=orange]Duis[/c] aute "
	"irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. [c=orange]Excepteur[/c] sint occaecat cupidatat "
	"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

const int padding_h = 4;
const int padding_v = 2;

int frame_left = 0;
int frame_top = 0;
int frame_width = 0;
int frame_height = 0;

void UpdateGeometry()
{
	frame_left = padding_h;
	frame_top = padding_v * 2 + 2;
	frame_width = terminal_state(TK_WIDTH) - frame_left - padding_h;
	frame_height = terminal_state(TK_HEIGHT) - frame_top - padding_v;
}

} // namespace anonymous

void TestTextAlignment()
{
	terminal_set("window.title='Omni: text alignment', resizeable=true, minimum-size=44x12");
	terminal_composition(TK_ON);

	Alignment horisontal_shift[] = {Left, Left, Center, Right, Right};
	Alignment vertical_shift[] = {Top, Top, Center, Bottom, Bottom};
	const char* horisontal_names[] = {"", "left", "center", "right"};
	const char* vertical_names[] = {"", "top", "center", "bottom"};
	Alignment horisontal = Left;
	Alignment vertical = Top;

	UpdateGeometry();

	for (bool proceed=true; proceed;)
	{
		terminal_clear();

		// Background square
		terminal_bkcolor("darkest gray");
		terminal_clear_area(frame_left, frame_top, frame_width, frame_height);
		terminal_bkcolor("none");

		// Comment
		terminal_printf
		(
			frame_left,
			frame_top - padding_v - 2,
			"Use arrows to change text alignment.\n"
			"Current alignment is [c=orange]%s-%s[/c].",
			vertical_names[(int)vertical], horisontal_names[(int)horisontal]
		);

		// Text origin
		int x, y;

		switch (horisontal)
		{
		case Right:
			x = frame_left + frame_width - 1;
			break;
		case Center:
			x = frame_left + frame_width / 2;
			break;
		default: // Left
			x = frame_left;
			break;
		}

		switch (vertical)
		{
		case Bottom:
			y = frame_top + frame_height - 1;
			break;
		case Center:
			y = frame_top + frame_height / 2;
			break;
		default: // Top
			y = frame_top;
			break;
		}

		terminal_color("darkest orange");
		terminal_put(x, y, 0x2588);

		terminal_color("white");
		terminal_printf
		(
			x, y,
			"[wrap=%dx%d][align=%s-%s]%s",
			frame_width, frame_height,
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
			else if (key == TK_RESIZED)
			{
				UpdateGeometry();
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_composition(TK_OFF);
	terminal_set("window.resizeable=false");
}
