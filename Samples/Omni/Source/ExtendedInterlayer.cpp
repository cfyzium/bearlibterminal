/*
 * ExtendedInterlayer.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <string>
#include <cmath>

const float pi = 3.141592654f;
const int duration = 1;
const int fps = 25;
const int radius = 2;
const int base_layer = 0;
const int animation_layer = 1;

void AnimatePopup(int x, int y, const std::string& s)
{
	int n_steps = duration * fps;
	float angle_delta = 2.0f * pi / n_steps;

	terminal_layer(animation_layer);
	for (int i=0; i<n_steps; i++)
	{
		if (terminal_has_input()) break;

		terminal_clear_area(0, 0, terminal_state(TK_WIDTH), terminal_state(TK_HEIGHT));
		float dx = std::sin(i*angle_delta) * radius * terminal_state(TK_CELL_WIDTH) + i*2;
		float dy = -2.0f * radius * terminal_state(TK_CELL_WIDTH) / n_steps * i - terminal_state(TK_CELL_HEIGHT)/2;
		terminal_color(color_from_argb(255/n_steps*(n_steps-i), 255, 128, 0));
		for (int j=0; j<s.length(); j++)
		{
			terminal_put_ext(x, y, dx+j*8, dy, s[j], nullptr);
		}
		terminal_refresh();
		delay(1000/fps);
	}
	terminal_color("white");
}

void TestExtendedInterlayer()
{
	// Setup
	terminal_set("window.title='Omni: extended output / interlayer animation'");
	terminal_composition(TK_COMPOSITION_ON);

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_layer(base_layer);
		terminal_put(20, 12, '@');

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_SPACE)
			{
				AnimatePopup(20, 12, "-10");
			}
		}
		while (proceed && terminal_has_input());
	}

	// Clean up
	terminal_composition(TK_COMPOSITION_OFF);
	terminal_clear();
	terminal_layer(0);
}
