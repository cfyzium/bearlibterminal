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
	terminal_bkcolor(0xFFAA9000);
	terminal_wprint(2, 2, L"Hello, world.");
	terminal_refresh();
	terminal_read();
	terminal_close();
	return 0;
}
