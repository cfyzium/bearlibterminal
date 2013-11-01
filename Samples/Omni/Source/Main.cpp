/*
 * Main.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#include "BearLibTerminal.h"

TERMINAL_TAKE_CARE_OF_WINMAIN

int main()
{
	terminal_open();
	terminal_setf("window.title='%s'", "Test title");
	terminal_set("window.cellsize=24x24");
	terminal_set("0xE000: default.png, size=16x16");
	terminal_set("0xE200: sample.jpeg");
	terminal_set("0xE201: sample2.jpg");
	terminal_color(0xFF000000);
	terminal_bkcolor(0xFFEE9000);
	terminal_wprint(2, 2, L"Hello, [color=white]world[/color].[U+2250] \x1234 {абв} [base=0xE000]abc");

	terminal_bkcolor(0);
	color_t corners[] = {0xFFFF0000, 0xFF00FF00, 0xFF6060FF, 0xFFFF00FF};
	terminal_put_ext(2, 4, 0, 0, L'я', corners);
	//terminal_bkcolor(0xFFFFFFFF);
	terminal_put_ext(3, 5, 0, 0, 0x2593, corners);
	terminal_put_ext(4, 4, 0, 0, 11*16+2, corners);
	terminal_bkcolor(0);

	terminal_color(0xFFFFFFFF);
	//terminal_bkcolor(0xFFEE9000);
	for (int y=0; y<16; y++)
	{
		for (int x=0; x<16; x++)
		{
			terminal_put(6+x, 6+y, y*16+x);
		}
	}

	terminal_color(0xFFFFFFFF);
	terminal_put(28, 10, 0xE200);

	terminal_layer(1);
	terminal_put(27, 9, 0xE201);

	terminal_refresh();
	terminal_read();
	terminal_close();
	return 0;
}
