/*
 * TestManualCellsize.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <string>
#include <vector>

void TestManualCellsize()
{
	terminal_set("window.title='Omni: manual cellsize'");

	const char* font_name = "../Media/VeraMono.ttf";
	std::vector<std::string> font_hintings = {"normal", "autohint", "none"};
	int font_hinting = 0;
	int font_size = 12;
	int cell_width = 8;
	int cell_height = 16;

	auto setup_font = [&](){terminal_setf("font: %s, size=%d, hinting=%s", font_name, font_size, font_hintings[font_hinting].c_str());};
	auto setup_cellsize = [&](){terminal_setf("window: cellsize=%dx%d", cell_width, cell_height);};

	setup_cellsize();
	setup_font();

	while (true)
	{
		terminal_clear();
		terminal_color("white");

		terminal_printf(2, 1, "Hello, world!");
		terminal_printf(2, 3, "[color=orange]Font size:[/color] %d", font_size);
		terminal_printf(2, 4, "[color=orange]Font hinting:[/color] %s", font_hintings[font_hinting].c_str());
		terminal_printf(2, 5, "[color=orange]Cell size:[/color] %dx%d", cell_width, cell_height);
		terminal_printf(2, 7, "[color=orange]TIP:[/color] Use arrow keys to change cell size");
		terminal_printf(2, 8, "[color=orange]TIP:[/color] Use Shift+Up/Down arrow keys to change font size");
		terminal_printf(2, 9, "[color=orange]TIP:[/color] Use TAB to switch font hinting mode");

		terminal_refresh();

		int key = terminal_read();

		if (key == TK_CLOSE || key == TK_ESCAPE)
		{
			break;
		}
		else if (key == TK_LEFT && !terminal_state(TK_SHIFT) && cell_width > 4)
		{
			cell_width -= 1;
			setup_cellsize();
		}
		else if (key == TK_RIGHT && !terminal_state(TK_SHIFT) && cell_width < 24)
		{
			cell_width += 1;
			setup_cellsize();
		}
		else if (key == TK_DOWN && !terminal_state(TK_SHIFT) && cell_height < 24)
		{
			cell_height += 1;
			setup_cellsize();
		}
		else if (key == TK_UP && !terminal_state(TK_SHIFT) && cell_height > 4)
		{
			cell_height -= 1;
			setup_cellsize();
		}
		else if (key == TK_UP && terminal_state(TK_SHIFT) && font_size < 64)
		{
			font_size += 1;
			setup_font();
		}
		else if (key == TK_DOWN && terminal_state(TK_SHIFT) && font_size > 4)
		{
			font_size -= 1;
			setup_font();
		}
		else if (key == TK_TAB)
		{
			font_hinting = (font_hinting + 1) % font_hintings.size();
			setup_font();
		}
	}

	terminal_set("font: default; window.cellsize=auto");
}
