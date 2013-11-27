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
	// TODO: runic symbols, crawl tileset, tome tileset, pc tile combination, a few artistic fonts from dafonts

	terminal_clear();
	terminal_color("white");
	for (int i=0; i<16; i++)
	{
		terminal_put(2+i*3, 10, 0xE000+i);
	}
	terminal_refresh();

	for (int key=0; key!=TK_CLOSE && key!=TK_ESCAPE; key=terminal_read());

	// Clean up
	terminal_set("U+E000: none");
}
