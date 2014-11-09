/*
 * ExtendedInterlayer.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>

const float pi = 3.141592654f;
const int duration = 1;
const int fps = 25;
const int radius = 2;
const int base_layer = 0;
const int animation_layer = 1;
const int map_width = 16;
const int map_height = 8;

int player_x = 1;
int player_y = 1;
int foe_x = 0;
int foe_y = 0;
int foe_hp = 0;

void AnimateDamage(int x, int y, int damage)
{
	std::ostringstream ss;
	ss << damage;
	std::string s = ss.str();

	int n_steps = duration * fps;
	float angle_delta = 2.0f * pi / n_steps;

	terminal_layer(animation_layer);
	for (int i=0; i<n_steps; i++)
	{
		if (terminal_has_input()) break;
		terminal_clear_area(0, 0, terminal_state(TK_WIDTH), terminal_state(TK_HEIGHT));
		float dx = std::sin(i*angle_delta) * radius * terminal_state(TK_CELL_WIDTH) + i*2;
		float dy = -2.0f * radius * terminal_state(TK_CELL_WIDTH) / n_steps * i - terminal_state(TK_CELL_HEIGHT)/2;
		terminal_color(color_from_argb(255/n_steps*(n_steps-i), 255, 64, 0));
		terminal_printf(x, y, "[offset=%dx%d]%s", (int)dx, (int)dy, s.c_str());
		terminal_refresh();
		terminal_delay(1000/fps);
	}
	terminal_color("white");
}

std::vector<int> PrepareMap()
{
	std::vector<int> result(map_width*map_height, 0);
	for (int x=0; x<map_width; x++)
	{
		result[x] = result[(map_height-1)*map_width+x] = L'─';
	}
	for (int y=0; y<map_height; y++)
	{
		result[y*map_width] = result[(y+1)*map_width-1] = L'│';
	}
	result[0] = L'┌';
	result[map_width-1] = L'┐';
	result[(map_height-1)*map_width] = L'└';
	result[map_height*map_width-1] = L'┘';

	return result;
}

void PlaceFoe()
{
	do
	{
		foe_x = 1+rand()%(map_width-2);
		foe_y = 1+rand()%(map_height-2);
		if (foe_x != player_x && foe_y != player_y) break;
	}
	while (true);

	foe_hp = 25;
}

void DrawMap(int x, int y, std::vector<int>& map)
{
	terminal_layer(base_layer);
	for (int j=0; j<map_height; j++)
	{
		for (int i=0; i<map_width; i++)
		{
			int v = map[j*map_width+i];
			if (i == player_x && j == player_y)
			{
				terminal_color("orange");
				terminal_put(x+i, y+j, '@');
			}
			else if (i == foe_x && j == foe_y && foe_hp > 0)
			{
				terminal_color("white");
				terminal_put(x+i, y+j, 'g');
			}
			else if (v == 0 || v == 1)
			{
				terminal_color(v? "red": "gray");
				terminal_put(x+i, y+j, 0x2219);
			}
			else
			{
				terminal_color("white");
				terminal_put(x+i, y+j, v);
			}
		}
	}
}

void TestExtendedInterlayer()
{
	// Setup
	terminal_set("window.title='Omni: extended output / interlayer animation'");
	terminal_composition(TK_ON);

	int map_left = terminal_state(TK_WIDTH)/2 - map_width/2;
	int map_top = terminal_state(TK_HEIGHT)/2 - map_height/2;

	std::srand(std::time(nullptr));
	auto map = PrepareMap();
	PlaceFoe();

	auto Attack = [&]
	{
		int damage = 1+std::rand()%10;
		foe_hp -= damage;
		int x = foe_x, y = foe_y;
		if (foe_hp <= 0)
		{
			map[foe_y*map_width+foe_x] = 1;
			PlaceFoe();
			terminal_clear();
			DrawMap(map_left, map_top, map);
		}
		AnimateDamage(map_left+x, map_top+y, -damage);
	};

	auto Step = [&](int dx, int dy)
	{
		if (player_x+dx == foe_x && player_y+dy == foe_y)
		{
			Attack();
		}
		else if (map[(player_y+dy)*map_width+player_x+dx] <= 1)
		{
			player_x += dx;
			player_y += dy;
		}
	};

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		DrawMap(map_left, map_top, map);
		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_LEFT)
			{
				Step(-1, 0);
			}
			else if (key == TK_UP)
			{
				Step(0, -1);
			}
			else if (key == TK_RIGHT)
			{
				Step(1, 0);
			}
			else if (key == TK_DOWN)
			{
				Step(0, 1);
			}
		}
		while (proceed && terminal_has_input());
	}

	// Clean up
	terminal_composition(TK_OFF);
	terminal_clear();
	terminal_layer(0);
}
