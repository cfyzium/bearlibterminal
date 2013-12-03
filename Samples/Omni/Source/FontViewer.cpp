/*
 * FontViewer.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <set>
#include <vector>
#include <string>
#include <sstream>

std::set<std::string> EnumerateFiles(const std::string& path)
{
	std::set<std::string> result;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string filename = path + "/" + ent->d_name;
			struct stat node;
			if ( stat(filename.c_str(), &node) == 0 && S_ISREG(node.st_mode) )
			{
				result.insert(std::string(ent->d_name));
			}
		}
		closedir(dir);
	}
	return result;
}

void DrawValueBox(int x, int y, int width, std::string s, bool up, bool down, color_t outline)
{
	// ┌───────┬──┐
	// │text...│↑↓│
	// └───────┴──┘

	terminal_color(outline);
	for (int i=0; i<width; i++)
	{
		terminal_put(x+i, y-1, L'─');
		terminal_put(x+i, y+1, L'─');
	}
	terminal_put(x-1, y-1, L'┌');
	terminal_put(x-1, y+0, L'│');
	terminal_put(x-1, y+1, L'└');
	terminal_put(x+width-3, y-1, L'┬');
	terminal_put(x+width-3, y+0, L'│');
	terminal_put(x+width-3, y+1, L'┴');
	terminal_put(x+width, y-1, L'┐');
	terminal_put(x+width, y+0, L'│');
	terminal_put(x+width, y+1, L'┘');
	terminal_wprintf(x+width-2, y, L"%s↑", up? "": "[color=gray]");
	terminal_wprintf(x+width-1, y, L"%s↓", down? "": "[color=gray]");

	int text_width = width-3;
	if (s.length() > text_width) s = s.substr(0, text_width-3) + "...";

	terminal_color("white");
	terminal_print(x, y, s.c_str());
}

struct Font
{
	std::string name;
	bool scalable;
};

void TestFontViewer()
{
	terminal_set("window.title='Omni: font viewer'");

	int active_control = 0;
	int current_font = 0;
	int current_size = 12;

	std::vector<Font> fonts;
	for (auto i: EnumerateFiles("./Fonts"))
	{
		Font fnt;
		fnt.name = i;
		fnt.scalable = fnt.name.find(".ttf") != std::string::npos;
		fonts.push_back(fnt);
	}

	auto update_font = [&]
	{
		if (fonts[current_font].scalable)
		{
			terminal_setf("U+E100: %s, size=%d, codepage=ascii", fonts[current_font].name.c_str(), current_size);
		}
		else
		{
			terminal_setf("U+E100: %s, codepage=ascii", fonts[current_font].name.c_str());
		}
	};

	update_font();

	for (bool proceed=true; proceed;)
	{
		Font& font = fonts[current_font];

		terminal_clear();

		DrawValueBox
		(
			3, 2,
			80-6-15-2,
			font.name.c_str(),
			current_font > 0,
			current_font < fonts.size()-1,
			color_from_name(active_control == 0? "orange": "light gray")
		);

		std::ostringstream ss;
		if (font.scalable)
		{
			ss << current_size << " (";
		}
		DrawValueBox
		(
			80-3-15, 2,
			15,
			"12 (8x16)",
			current_size > 4,
			current_size < 32,
			color_from_name(active_control == 1? "orange": "light gray")
		);

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
			else if (key == TK_TAB)
			{
				active_control = (active_control+1)%3;
			}
			else if (key == TK_UP)
			{
				if (active_control == 0 && current_font > 0)
				{
					current_font -= 1;
					update_font();
				}
				else if (active_control == 1 && current_size > 4)
				{
					current_size -= 1;
					update_font();
				}
			}
			else if (key == TK_DOWN)
			{
				if (active_control == 0 && current_font < fonts.size()-1)
				{
					current_font += 1;
					update_font();
				}
				else if (active_control == 1 && current_size < 32)
				{
					current_size += 1;
					update_font();
				}
			}
		}
		while (proceed && terminal_has_input());
	}
}
