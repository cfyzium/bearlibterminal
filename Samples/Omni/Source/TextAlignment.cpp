/*
 * TextAlignment.cpp
 *
 *  Created on: Jul 29, 2014
 *      Author: cfyz
 */

#include "Common.hpp"

#include <cmath>
#include <map>

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

	std::map<int, std::string> names =
	{
		{TK_ALIGN_LEFT, "TK_ALIGN_LEFT"},
		{TK_ALIGN_CENTER, "TK_ALIGN_CENTER"},
		{TK_ALIGN_RIGHT, "TK_ALIGN_RIGHT"},
		{TK_ALIGN_TOP, "TK_ALIGN_TOP"},
		{TK_ALIGN_MIDDLE, "TK_ALIGN_MIDDLE"},
		{TK_ALIGN_BOTTOM, "TK_ALIGN_BOTTOM"},
	};
	int horisontal_align = TK_ALIGN_LEFT;
	int vertical_align = TK_ALIGN_TOP;

	UpdateGeometry();

	while (true)
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
			"Current alignment is [c=orange]%s[/c] | [c=orange]%s[/c].",
			names[vertical_align].c_str(),
			names[horisontal_align].c_str()
		);

		// Text origin
		int x = frame_left, y = frame_top;

		terminal_color("white");
		terminal_print_ext(x, y, frame_width, frame_height, vertical_align | horisontal_align, lorem_ipsum);

		terminal_print(80-14, 3, "[c=orange][U+2588]");
		terminal_print_ext(80-14, 3, 0, 0, vertical_align | horisontal_align, "12345\nabc\n-=#=-");

		terminal_refresh();

		int key = terminal_read();

		if (key == TK_CLOSE || key == TK_ESCAPE)
		{
			break;
		}
		else if (key == TK_LEFT)
		{
			horisontal_align = (horisontal_align == TK_ALIGN_RIGHT)? TK_ALIGN_CENTER: TK_ALIGN_LEFT;
		}
		else if (key == TK_RIGHT)
		{
			horisontal_align = (horisontal_align == TK_ALIGN_LEFT)? TK_ALIGN_CENTER: TK_ALIGN_RIGHT;
		}
		else if (key == TK_UP)
		{
			vertical_align = (vertical_align == TK_ALIGN_BOTTOM)? TK_ALIGN_MIDDLE: TK_ALIGN_TOP;
		}
		else if (key == TK_DOWN)
		{
			vertical_align = (vertical_align == TK_ALIGN_TOP)? TK_ALIGN_MIDDLE: TK_ALIGN_BOTTOM;
		}
		else if (key == TK_RESIZED)
		{
			UpdateGeometry();
		}
	}

	terminal_composition(TK_OFF);
	terminal_set("window.resizeable=false");
}
