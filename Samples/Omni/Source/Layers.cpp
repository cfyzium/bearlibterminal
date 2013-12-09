/*
 * Layers.cpp
 *
 *  Created on: Dec 9, 2013
 *      Author: Cfyz
 */

#include "Common.hpp"

void TestLayers()
{
	terminal_set("window.title='Omni: layers'");

	color_t pixel = color_from_name("dark gray");
	terminal_setf("U+E000: %#p, size=1x1, resize=48x48, resize-filter=nearest", &pixel);

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_print(2, 1, "[color=orange]1.[/color] Without layers:");
		terminal_put(7, 3, 0xE000);
		terminal_print(5, 4, "[color=dark green]abcdefghij");

		terminal_print(2, 8, "[color=orange]2.[/color] With layers:");
		terminal_layer(1);
		terminal_put(7, 10, 0xE000);
		terminal_layer(0);
		terminal_print(5, 11, "[color=dark green]abcdefghij");

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_set("U+E000: none");
}
