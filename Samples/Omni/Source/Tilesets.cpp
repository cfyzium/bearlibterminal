/*
 * Tilesets.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

void TestTilesets()
{
	terminal_set("window.title='Omni: tilesets'");

	// Load tilesets
	terminal_set("U+E000: fontawesome-webfont.ttf, size=16x16, bbox=2x1, codepage=fontawesome-codepage.txt");
	terminal_set("U+E100: runic.png, size=8x16");
	// TODO: runic symbols, crawl tileset, tome tileset, pc tile combination, a few artistic fonts from dafonts

	terminal_clear();
	terminal_color("white");

	terminal_print(2, 1, "[color=orange]1.[/color] Of course, there is default font tileset.");

	terminal_print(2, 3, "[color=orange]2.[/color] You can load some arbitrary tiles and use them as glyphs:");
	terminal_print
	(
		2, 4,
		"Fire rune ([color=red][U+E102][/color]), "
		"water rune ([color=lighter blue][U+E103][/color]), "
		"earth rune ([color=darker green][U+E104][/color])"
	);

	terminal_print(2, 6, "[color=orange]3.[/color] These additional tiles are not required to be of the same size:");

	terminal_print(2, 10, "[color=orange]4.[/color] All tiles are functinally the same and may be freely colored and combined:");

	terminal_print(2, 14, "[color=orange]5.[/color] And tiles can event come from TrueType fonts like this:");
	for (int i=0; i<16; i++)
	{
		terminal_put(2+i*3, 15, 0xE000+i);
	}

	terminal_print(2, 17, "...or like this:");

	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("U+E000: none");
}
