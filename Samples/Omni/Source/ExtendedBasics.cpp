/*
 * ExtendedBasics.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <cmath>
#include <vector>

const float g_pi = 3.141592654f;

void TestExtendedBasics()
{
	// Setup
	terminal_set("window.title='Omni: extended output / basics'");
	terminal_set("0xE000: ../Media/Tiles.png, size=32x32, align=top-left");
	terminal_composition(TK_ON);

	int cx = 10, cy = 5;
	int n_symbols = 10;
	int radius = 5;
	float angle = 0.0f;
	int fps = 25;
	color_t transparent = 0x00FFFFFF, opaque = 0xFFFFFFFF;

	color_t m00[] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00};
	color_t m01[] = {opaque, opaque, transparent, transparent};

	color_t m11[] = {transparent, transparent, opaque, transparent};
	color_t m12[] = {transparent, opaque, transparent, transparent};
	color_t m21[] = {transparent, transparent, transparent, opaque};
	color_t m22[] = {opaque, transparent, transparent, transparent};

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_print(2, 1, "[color=orange]1.[/color] terminal_put_ext(x, y, [color=orange]dx[/color], [color=orange]dy[/color], code, corners);");
		for (int i=0; i<n_symbols; i++)
		{
			float angle_delta = 2.0f*g_pi/n_symbols;
			float dx = std::cos(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH);
			float dy = std::sin(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH)-4;
			terminal_color(i? "white": "orange");
			terminal_put_ext(cx, cy, dx, dy, 'a'+i, nullptr);
		}
		angle += 2.0f*g_pi / (2*fps);

		terminal_print(2, 9, "[color=orange]2.[/color] terminal_put_ext(x, y, dx, dy, code, [color=orange]corners[/color]);");
		terminal_put_ext(5, 11, 0, 0, 0xE000+19, m00);
		terminal_put_ext(10, 11, 0, 0, 0xE000+19, m01);

		terminal_print(2, 14, "[color=orange]3.[/color] terminal_put_ext + composition");
		int x1 = 5;
		int y1 = 16;
		terminal_put(x1+0, y1+0, 0xE000+19);
		terminal_put(x1+0, y1+2, 0xE000+8);
		terminal_put(x1+5, y1+0, 0xE000+19);
		terminal_put(x1+9, y1+0, 0xE000+19);
		terminal_put(x1+5, y1+2, 0xE000+19);
		terminal_put(x1+9, y1+2, 0xE000+19);
		terminal_put_ext(x1+5, y1+0, 0, 0, 0xE000+8, m11);
		terminal_put_ext(x1+9, y1+0, 0, 0, 0xE000+8, m12);
		terminal_put_ext(x1+5, y1+2, 0, 0, 0xE000+8, m21);
		terminal_put_ext(x1+9, y1+2, 0, 0, 0xE000+8, m22);

		terminal_refresh();

		while (terminal_has_input())
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
				break;
			}
		}

		delay(1000/fps);
	}

	// Clean up
	terminal_composition(TK_OFF);
	terminal_set("0xE000: none");
}
