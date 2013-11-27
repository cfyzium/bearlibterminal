/*
 * TestManualCellsize.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

void TestManualCellsize()
{
	terminal_set("window.title='Omni: manual cellsize'");

	const char* font_name = "UbuntuMono-R.ttf";
	int font_size = 12;
	int cell_width = 8;
	int cell_height = 16;

	auto setup_font = [&](){terminal_setf("font: %s, size=%d", font_name, font_size);};
	auto setup_cellsize = [&](){terminal_setf("window: cellsize=%dx%d", cell_width, cell_height);};

	setup_cellsize();
	setup_font();

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_printf(2, 1, L"Hello, world!");
		terminal_printf(2, 3, L"[color=orange]Font size:[/color] %d", font_size);
		terminal_printf(2, 4, L"[color=orange]Cell size:[/color] %dx%d", cell_width, cell_height);
		terminal_printf(2, 6, L"[color=orange]TIP:[/color] Use arrow keys to change cell size");
		terminal_printf(2, 7, L"[color=orange]TIP:[/color] Use Shift+Up/Down arrow keys to change font size");

		terminal_refresh();

		do
		{
			int key = terminal_read();

			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_LEFT && !terminal_state(TK_SHIFT) && cell_width > 4)
			{
				cell_width -= 1;
				setup_cellsize();
				break;
			}
			else if (key == TK_RIGHT && !terminal_state(TK_SHIFT) && cell_width < 24)
			{
				cell_width += 1;
				setup_cellsize();
				break;
			}
			else if (key == TK_DOWN && !terminal_state(TK_SHIFT) && cell_height < 24)
			{
				cell_height += 1;
				setup_cellsize();
				break;
			}
			else if (key == TK_UP && !terminal_state(TK_SHIFT) && cell_height > 4)
			{
				cell_height -= 1;
				setup_cellsize();
				break;
			}
			else if (key == TK_DOWN && terminal_state(TK_SHIFT) && font_size < 64)
			{
				font_size += 1;
				setup_font();
				break;
			}
			else if (key == TK_UP && terminal_state(TK_SHIFT) && font_size > 4)
			{
				font_size -= 1;
				setup_font();
				break;
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_set("font: default; window.cellsize=auto");
}
