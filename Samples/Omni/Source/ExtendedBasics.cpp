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
	terminal_set("0xE000: dg_grounds32.png, size=32x32");
	terminal_composition(TK_COMPOSITION_ON);

	int cx = 15, cy = 11;
	int n_symbols = 10;
	int radius = 5;
	float angle = 0.0f;

	int fps = 25;

	color_t transparent = 0x00FFFFFF;
	color_t opaque = 0xFFFFFFFF;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_print(2, 1, "terminal_put_ext(x, y, dx, dy, code, corners);");

		for (int i=0; i<n_symbols; i++)
		{
			float angle_delta = 2.0f*g_pi/n_symbols;
			float dx = std::cos(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH);
			float dy = std::sin(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH);
			terminal_color(i? "white": "orange");
			terminal_put_ext(cx, cy, dx, dy, 'a'+i, nullptr);
		}
		angle += 2.0f*g_pi / (2*fps);

		terminal_print
		(
			30, cy-6+2,
			"Unfortunately, due to limitations of bare OpenGL\n"
			"(i.e. without shaders) pipeline, corner-coloring\n"
			"can be used for producing linear horisontal or\n"
			"vertical gradients only."
		);

		terminal_put_ext(80-cx-5-17, cy+2, 0, 0, 0xE000+9, nullptr);
		terminal_put(80-cx-5-13, cy+2, 0x2192);

		color_t c1[] = {opaque, opaque, transparent, transparent};
		color_t c2[] = {transparent, opaque, opaque, transparent};
		terminal_put_ext(80-cx-5-9, cy+2, 0, 0, 0xE000+9, c1);
		terminal_put_ext(80-cx-5-4, cy+2, 0, 0, 0xE000+9, c2);

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
	terminal_composition(TK_COMPOSITION_OFF);
	terminal_set("0xE000: none");
}
