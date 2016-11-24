/*
 * DefaultFont.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include "WindowsGlyphList4.hpp"

void TestDefaultFont()
{
	terminal_set("window: size=80x25, cellsize=auto, title='Omni: WGL4'; font=default");

	const int hoffset = 40;
	int current_range = 0;

	while (true)
	{
		terminal_clear();
		terminal_wprint(2, 1, L"[color=white]Select unicode character range:");

		for (int i=0; i<g_wgl4_ranges.size(); i++)
		{
			bool selected = i == current_range;
			terminal_color(selected? color_from_name("orange"): color_from_name("light gray"));
			terminal_printf(1, 2+i, "%s%s", selected? "[U+203A]": " ", g_wgl4_ranges[i].name.c_str());
		}

		UnicodeRange& range = g_wgl4_ranges[current_range];
		for (int j=0; j<16; j++)
		{
			terminal_printf(hoffset+6+j*2, 1, "[color=orange]%X", j);
		}
		for (int code=range.start, y=0; code<=range.end; code++)
		{
			if (code%16 == 0) terminal_printf(hoffset, 2+y*1, "[color=orange]%04X:", code);

			bool included = range.codes.count(code);
			terminal_color(included? color_from_name("white"): color_from_name("dark gray"));
			terminal_put(hoffset+6+(code%16)*2, 2+y*1, code);

			if ((code+1)%16 == 0) y += 1;
		}

		terminal_color("white");
		terminal_print(hoffset, 20, L"[color=orange]TIP:[/color] Use ↑/↓ keys to select range");
		terminal_print(hoffset, 22, L"[color=orange]NOTE:[/color] Character code points printed in\ngray are not included in the WGL4 set.");

		terminal_refresh();

		int key = terminal_read();

		if (key == TK_ESCAPE || key == TK_CLOSE)
		{
			break;
		}
		else if (key == TK_UP && current_range > 0)
		{
			current_range -= 1;
		}
		else if (key == TK_DOWN && current_range < g_wgl4_ranges.size()-1)
		{
			current_range += 1;
		}
	}
}
