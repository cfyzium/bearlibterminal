/*
 * WindowResize.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: cfyz
 */

#include "Common.hpp"

void TestWindowResize()
{
	terminal_set("window: title='Omni: window resizing', resizeable=true, minimum-size=27x5");

	const int symbol = 0x2588;

	for (bool proceed=true; proceed;)
	{
		do
		{
			terminal_clear();
			int w = terminal_state(TK_WIDTH);
			int h = terminal_state(TK_HEIGHT);
			for (int x=0; x<w; x++)
			{
				terminal_put(x, 0, x%2? symbol: (int)'#');
				terminal_put(x, h-1, x%2? symbol: (int)'#');
			}
			for (int y=0; y<h; y++)
			{
				terminal_put(0, y, y%2? symbol: (int)'#');
				terminal_put(w-1, y, y%2? symbol: (int)'#');
			}
			terminal_printf(3, 2, "Terminal size is %dx%d", w, h);
			terminal_refresh();

			int key = terminal_read();

			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_RESIZE)
			{
				// ?..
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_set("window: resizeable=false");
}
