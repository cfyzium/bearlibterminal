/*
 * MultipleFonts.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

void TestMultipleFonts()
{
	terminal_set("window.title='Omni: multiple fonts in scene'");

	// Load several fonts
	terminal_set("font: ./Media/VeraMono.ttf, size=8x16");
	terminal_set("U+E100: ./Media/VeraMoIt.ttf, size=8x16");
	terminal_set("U+E200: ./Media/VeraMoBd.ttf, size=8x16");
	terminal_set("U+E300: ./Media/VeraMono.ttf, size=16x32, bbox=2x2");

	terminal_clear();
	terminal_color("white");
	terminal_printf(2, 1, "If you [color=orange][base=U+E100:437]really[/base][/color] want, you can even put [color=orange][base=U+E200:437]emphasis[/base][/color] on a text.");
	terminal_printf
	(
		2, 3,
		"This works by loading several truetype tilesets with custom codepages to an\n"
		"unused code points and using [color=orange]base[/color] postformatting tag."
	);

	terminal_print
	(
		2, 6,
		"[base=U+E300:437][spacing=2x2]It's relatively easy to print in\n"
		"bigger fonts as well using [color=orange]base[/color] and\n"
		"[color=orange]spacing[/color] postformatting tags."
	);
	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("font: default; U+E100: none; U+E200: none; U+E300: none");
}
