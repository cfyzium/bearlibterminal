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
	terminal_set("window.size=64x20; font: ../Media/VeraMono.ttf, size=10x20");
	terminal_set("italic font: ../Media/VeraMoIt.ttf, size=10x20");
	terminal_set("bold font: ../Media/VeraMoBd.ttf, size=10x20");
	terminal_set("huge font: ../Media/VeraMono.ttf, size=20x40, spacing=2x2");

	terminal_clear();
	terminal_color("white");
	int h = terminal_printf
	(
		2, 1,
		"[wrap=60]If you [color=orange][font=italic]really[/font][/color] want, "
		"you can even put [color=orange][font=bold]emphasis[/font][/color] on a text. "
		"This works by loading several truetype tilesets with custom codepages to an "
		"unused code points and using [color=orange]font[/color] postformatting tag."
	);

	terminal_print
	(
		2, 1+h+1,
		"[font=huge][wrap=60]It's pretty easy to print in bigger fonts as well."
	);
	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("window.size=80x25; font: default; italic font: none; bold font: none; huge font: none");
}
