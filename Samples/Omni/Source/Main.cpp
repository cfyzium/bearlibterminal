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
	terminal_color(0xFF000000);
	terminal_bkcolor(0xFFEE9000);
	terminal_wprint(2, 2, L"Hello, [color=white]world[/color].[U+2250] \x1234 [base=1]abc");
	terminal_refresh();
	terminal_read();
	terminal_close();
	return 0;
}
