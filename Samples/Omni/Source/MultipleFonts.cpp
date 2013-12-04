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
	/*
	terminal_set("font: UbuntuMono-R.ttf, size=12");
	terminal_set("U+E100: UbuntuMono-RI.ttf, size=12, codepage=437");
	terminal_set("U+E200: UbuntuMono-B.ttf, size=12, codepage=437");
	/*/
	terminal_set("font: ./Fonts/VeraMono.ttf, size=8x16");
	terminal_set("U+E100: ./Fonts/VeraMoIt.ttf, size=8x16, codepage=437");
	terminal_set("U+E200: ./Fonts/VeraMoBd.ttf, size=8x16, codepage=437");
	//*/

	terminal_clear();
	terminal_color("white");
	terminal_printf(2, 1, "If you [color=orange][base=U+E100:437]really[/base][/color] want, you can even put [color=orange][base=U+E200:437]emphasis[/base][/color] on a text.");
	terminal_printf
	(
		2, 3,
		"This works by loading several truetype tilesets with custom codepages to an\n"
		"unused code points and using [[base]] postformatting tag."
	);
	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("font: default; U+E100: none; U+E200: none");
}
