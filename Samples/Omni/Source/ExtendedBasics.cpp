/*
 * ExtendedBasics.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <cmath>

const float g_pi = 3.141592654f;

void TestExtendedBasics()
{
	// Setup
	terminal_set("window.title='Omni: extended output / basics'");
	terminal_composition(TK_COMPOSITION_ON);

	int cx = 20, cy = 11;
	int n_symbols = 10;
	int radius = 5;
	float angle = 0.0f;

	int fps = 25;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		for (int i=0; i<n_symbols; i++)
		{
			float angle_delta = 2.0f*g_pi/n_symbols;
			float dx = std::cos(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH);
			float dy = std::sin(angle+i*angle_delta)*radius*terminal_state(TK_CELL_WIDTH);
			terminal_color(i? "white": "orange");
			terminal_put_ext(cx, cy, dx, dy, 'a'+i, nullptr);
		}
		angle += 2.0f*g_pi / (2*fps);

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
}
