/*
 * Tilesets.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <vector>

void TestTilesets()
{
	terminal_set("window.title='Omni: tilesets'");
	terminal_composition(TK_COMPOSITION_ON);

	// Load tilesets
	//terminal_set("U+E000: ./Media/fontawesome-webfont.ttf, size=16x16, bbox=2x1, codepage=./Media/fontawesome-codepage.txt");
	terminal_set("U+E100: ./Media/Runic.png, size=8x16");
	terminal_set("U+E200: ./Media/Tiles.png, size=32x32, align=top-left");
	terminal_set("U+E300: ./Media/fontawesome-webfont.ttf, size=24x24, align=top-left, codepage=./Media/fontawesome-codepage.txt");
	terminal_set("U+E400: ./Media/Zodiac-S.ttf, size=24x24, align=top-left, codepage=437");

	terminal_clear();
	terminal_color("white");

	terminal_print(2, 1, "[color=orange]1.[/color] Of course, there is default font tileset.");

	terminal_print(2, 3, "[color=orange]2.[/color] You can load some arbitrary tiles and use them as glyphs:");
	terminal_print
	(
		2+3, 4,
		"Fire rune ([color=red][U+E102][/color]), "
		"water rune ([color=lighter blue][U+E103][/color]), "
		"earth rune ([color=darker green][U+E104][/color])"
	);

	terminal_print(2, 6, "[color=orange]3.[/color] Tiles are not required to be of the same size as font symbols:");
	terminal_put(2+3+0, 7, 0xE200+7);
	terminal_put(2+3+5, 7, 0xE200+8);
	terminal_put(2+3+10, 7, 0xE200+9);

	terminal_print(2, 10, "[color=orange]4.[/color] Like font characters, tiles may be freely colored and combined:");
	terminal_put(2+3+0, 11, 0xE200+8);
	terminal_color("lighter orange");
	terminal_put(2+3+5, 11, 0xE200+8);
	terminal_color("orange");
	terminal_put(2+3+10, 11, 0xE200+8);
	terminal_color("dark orange");
	terminal_put(2+3+15, 11, 0xE200+8);

	terminal_color("white");
	std::vector<int> order = {11, 10, 14, 12, 13};
	for (int i=0; i<order.size(); i++)
	{
		terminal_put(2+3+25+i*4, 11, 0xE200+order[i]);
		terminal_put(2+3+25+(order.size()+1)*4, 11, 0xE200+order[i]);
	}
	terminal_put(2+3+25+order.size()*4, 11, 0xE200+15);

	terminal_print(2, 14, "[color=orange]5.[/color] And tiles can even come from TrueType fonts like this:");
	for (int i=0; i<6; i++)
	{
		terminal_put(2+3+i*5, 15, 0xE300+i);
	}

	terminal_print(2+3, 18, "...or like this:");
	terminal_print(2+3, 19, "[base=0xE400]D    F    G    S    C");

	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("U+E100: none; U+E200: none; U+E300: none; U+E400: none");
	terminal_composition(TK_COMPOSITION_OFF);
}
