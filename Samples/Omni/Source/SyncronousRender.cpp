/*
 * SyncronousRender.cpp
 *
 *  Created on: Dec 6, 2013
 *      Author: Cfyz
 */

#include "Common.hpp"
#include <cmath>
#include <cstdlib>

struct color3_t
{
	int r, g, b;
};

color3_t GetShiftedColor(int shift)
{
	float f = (shift%80) / 80.0f;
	color3_t c = {0};

	if (f < 0.33f) // From red to green (through orange)
	{
		c.r = 255 * ((0.33f - f) / 0.33f);
		c.g = 255 * ((f - 0.0f) / 0.33f);
	}
	else if (f < 0.66f) // From green to blue (through cyan)
	{
		c.g = 255 * ((0.66f - f) / 0.33f);
		c.b = 255 * ((f - 0.33f) / 0.33f);
	}
	else // From blue to red (through magenta)
	{
		c.b = 255 * ((1.0f - f) / 0.33f);
		c.r = 255 * ((f - 0.66f) / 0.33f);
	}

	if (c.r > 255) c.r = 255;
	if (c.g > 255) c.g = 255;
	if (c.b > 255) c.b = 255;

	return c;
};

color_t GetHighlightedColor(color3_t c)
{
	c.r *= 2;
	c.g *= 2;
	c.b *= 2;

	if (c.r > 255) c.r = 255;
	if (c.g > 255) c.g = 255;
	if (c.b > 255) c.b = 255;

	return 0xFF000000 | (c.r << 16) | (c.g << 8) | c.b;
};

color_t GetDimmedColor(color3_t c)
{
	c.r /= 2;
	c.g /= 2;
	c.b /= 2;

	return 0xFF000000 | (c.r << 16) | (c.g << 8) | c.b;
};

color_t color_from_another(uint8_t alpha, color_t base)
{
	return (base & 0x00FFFFFF) | (alpha << 24);
}

void TestSynchronousRender()
{
	//*
	terminal_set("window.title='Omni: syncronous rendering'");
	terminal_composition(TK_COMPOSITION_ON);

	terminal_setf("output.vsync=false");

	unsigned int shift_f = 0, shift_b = 0, shift_f2 = 0;
	float shift_f2f = 0;

	color_t shifted_f[80], shifted_b[80];
	for (int i = 0; i < 80; i++)
	{
		color3_t c = GetShiftedColor(i);
		shifted_b[i] = GetHighlightedColor(c);
		shifted_f[i] = GetDimmedColor(c);
	}

	uint64_t fps_update_time = GetTime();
	int fps_counter = 0;
	int fps_value = 0;
	bool vsync = true;
	bool async = true;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		for (int y = 0; y < 25; y++)
		{
			for (int x = 0; x < 80; x++)
			{
				terminal_color(shifted_b[(shift_b+x+y)%80]);
				terminal_put(x, y, 0x2588);
				terminal_color(color_from_another(100, shifted_b[(shift_f2+y-x)%80]));
				terminal_put(x, y, 0x2588);
				//int d = (int)std::fabs(40-(int)((shift_f)%80));
				//terminal_color(color_from_argb((int)(d/40.0f*128.0f), 255, 255, 255));
			}
		}
		terminal_printf(2, 1, "[color=black]vsync: %s\nasync: %s\nFPS: %d", vsync? "yes": "no", async? "yes": "no", fps_value);
		terminal_refresh();

		fps_counter += 1;
		uint64_t time = GetTime();
		if (time > fps_update_time + 1000)
		{
			fps_value = fps_counter;
			fps_counter = 0;
			fps_update_time = time;
		}

		while (proceed && terminal_has_input())
		{
			int code = terminal_read();
			if (code == TK_ESCAPE || code == TK_CLOSE)
			{
				proceed = false;
			}
			else if (code == TK_1)
			{
				vsync = !vsync;
				terminal_setf("output.vsync=%s", vsync? "true": "false");
			}
			else if (code == TK_2)
			{
				async = !async;
				terminal_setf("output.asynchronous=%s", async? "true": "false");
				terminal_set("font: default");
			}
		}

		shift_f -= 1;
		shift_f2 -= 2;
		shift_b += 1;

		shift_f2f -= 1.25f;
		shift_f2 = (int)shift_f2f;
	}

	terminal_set("output.vsync=true; output.asynchronous=true;");
	terminal_composition(TK_COMPOSITION_OFF);
	/*/
	//consolas_unicode_10x10.png
	terminal_set("window.title='Omni: syncronous rendering'");
	terminal_set("font: consolas_unicode_10x10.png, size=10x10; window.size=46x20");
	terminal_composition(TK_COMPOSITION_ON);

	unsigned int shift_f = 0, shift_b = 0, shift_f2 = 0;
	float shift_f2f = 0;

	color_t shifted_f[80], shifted_b[80];
	for (int i = 0; i < 80; i++)
	{
		color3_t c = GetShiftedColor(i);
		shifted_b[i] = GetHighlightedColor(c);
		shifted_f[i] = GetDimmedColor(c);
	}

	uint64_t fps_update_time = GetTime();
	int fps_counter = 0;
	int fps_value = 0;
	bool vsync = true;
	bool async = true;

	std::srand(GetTime());
	int r0[2000];
	for (int i=0; i<2000; i++) r0[i] = rand()%256;

	for (bool proceed=true; proceed;)
	{
		int r1 = rand()%256;

		terminal_clear();
		terminal_color("dark gray");
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 46; x++)
			{
				terminal_color(shifted_b[(shift_b+x+y)%46]);
				terminal_put(x, y, 0x2588);
				terminal_put(x, y, 'a'+(r1+r0[y*46+x])%26);
			}
		}
		terminal_printf(2, 1, "[color=black]vsync: %s\nasync: %s\nFPS: %d", vsync? "yes": "no", async? "yes": "no", fps_value);
		terminal_refresh();

		fps_counter += 1;
		uint64_t time = GetTime();
		if (time > fps_update_time + 1000)
		{
			fps_value = fps_counter;
			fps_counter = 0;
			fps_update_time = time;
		}

		while (proceed && terminal_has_input())
		{
			int code = terminal_read();
			if (code == TK_ESCAPE || code == TK_CLOSE)
			{
				proceed = false;
			}
			else if (code == TK_1)
			{
				vsync = !vsync;
				terminal_setf("output.vsync=%s", vsync? "true": "false");
			}
			else if (code == TK_2)
			{
				async = !async;
				terminal_setf("output.asynchronous=%s", async? "true": "false");
			}
		}

		shift_f -= 1;
		shift_f2 -= 2;
		shift_b += 1;

		shift_f2f -= 1.25f;
		shift_f2 = (int)shift_f2f;
	}

	terminal_set("output.vsync=true; output.asynchronous=true;");
	terminal_composition(TK_COMPOSITION_OFF);
	//*/
}
