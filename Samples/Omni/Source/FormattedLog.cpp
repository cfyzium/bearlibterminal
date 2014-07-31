/*
 * FormattedLog.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cfyz
 */

#include "Common.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <stdlib.h>
#include <string.h>

std::vector<std::string> vocabulary =
{
	"ad",
	"adipisicing",
	"aliqua",
	"aliquip",
	"amet",
	"anim",
	"aute",
	"cillum",
	"commodo",
	"consectetur",
	"consequat",
	"culpa",
	"cupidatat",
	"deserunt",
	"do",
	"dolore",
	"dolor",
	"dolore",
	"duis",
	"ea",
	"eiusmod",
	"elit",
	"enim",
	"esse",
	"est",
	"et",
	"ex",
	"exercitation",
	"excepteur",
	"eu",
	"fugiat",
	"id",
	"in",
	"incididunt",
	"ipsum",
	"irure",
	"labore",
	"laboris",
	"laborum",
	"lorem",
	"magna",
	"minim",
	"mollit",
	"nisi",
	"non",
	"nostrud",
	"nulla",
	"occaecat",
	"officia",
	"pariatur",
	"proident",
	"qui",
	"quis",
	"reprehenderit",
	"sed",
	"sit",
	"sint",
	"sunt",
	"tempor",
	"ullamco",
	"ut",
	"velit",
	"veniam",
	"voluptate"
};

std::vector<std::string> colors =
{
	"light orange",
	"orange",
	"dark orange",
	"darker orange",
	"light gray",
	"gray"
};

std::vector<int> alternative_fonts;

static std::string GenerateRandomMessage()
{
	std::ostringstream ss;

	int n_words = 5 + rand()%50;
	for (int i=0; i<n_words; i++)
	{
		std::string word = vocabulary[rand()%vocabulary.size()];
		if (i == 0) word[0] = toupper(word[0]);

		bool colored = (rand()%6 == 0);
		bool alt_font = (rand()%8 == 0);

		if (colored)
		{
			ss << "[color=" << colors[rand()%colors.size()] << "]";
		}

		if (alt_font)
		{
			ss << "[font=" << alternative_fonts[rand()%alternative_fonts.size()] << "]";
		}

		ss << word;

		if (colored)
		{
			ss << "[/color]";
		}

		if (alt_font)
		{
			ss << "[/font]";
		}

		if (i != n_words-1) ss << " ";
	}
	ss << ".";

	return ss.str();
}

struct Message
{
	Message():
		height(0)
	{ }

	Message(const std::string& text):
		text(text),
		height(0)
	{ }

	std::string text;
	int height;
};

int UpdateHeights(std::vector<Message>& messages)
{
	int width = terminal_state(TK_WIDTH) - 4 - 2;
	int total_height = 0;

	for (auto& message: messages)
	{
		message.height = terminal_measuref("[bbox=%d]%s", width, message.text.c_str());
		total_height += message.height;
	}

	total_height += messages.size()-1;
	return total_height;
}

void TestFormattedLog()
{
	terminal_set("window: size=64x20, resizeable=true, minimum-size=20x8; font: ../Media/VeraMono.ttf, size=10x20");
	terminal_set("U+E100: ../Media/VeraMoIt.ttf, size=10x20");
	terminal_set("U+E200: ../Media/VeraMoBd.ttf, size=10x20");

	alternative_fonts.push_back(0xE100);
	alternative_fonts.push_back(0xE200);

	std::vector<Message> messages;

	const std::string prompt =
		"Use arrow keys or mouse wheel to scroll the list up and down. "
		"Try to resize the window.\n--- --- ---";
	messages.push_back(Message(prompt));

	for (size_t i=0; i<10; i++)
	{
		messages.push_back(Message(GenerateRandomMessage()));
	}

	int offset = 0;

	int total_height = UpdateHeights(messages);
	int window_width = terminal_state(TK_WIDTH) - 4 - 2;
	int window_height = terminal_state(TK_HEIGHT) - 2;
	int bar_height = std::ceil(window_height * (window_height/(float)total_height));

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		int first_message = 0, first_line = 0;
		for (; first_line < total_height; first_message++)
		{
			auto& message = messages[first_message];
			if (first_line + message.height >= offset)
			{
				break;
			}
			else
			{
				first_line += message.height + 1;
			}
		}

		int delta = first_line - offset;

		terminal_layer(1);
		terminal_crop(2, 1, window_width, window_height);

		for (; first_message < messages.size() && delta <= window_height; first_message++)
		{
			auto& message = messages[first_message];
			terminal_printf(2, 1+delta, "[bbox=%d]%s", window_width, message.text.c_str());
			delta += message.height+1;
		}

		terminal_layer(0);

		int bar_offset = /*std::floor*/(window_height * (offset / (float)total_height));
		terminal_color("darker gray");
		for (int i = 0; i < window_height; i++)
		{
			terminal_put(2+window_width+1, 1+i, 0x2588);
		}
		terminal_color("dark orange");
		for (int i = 0; i < bar_height; i++)
		{
			terminal_put(2+window_width+1, 1+bar_offset+i, 0x2588);
		}

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
				break;
			}
			else if (key == TK_UP || (key == TK_MOUSE_SCROLL && terminal_state(TK_MOUSE_WHEEL) < 0))
			{
				if (offset > 0) offset -= 1;
			}
			else if (key == TK_DOWN || (key == TK_MOUSE_SCROLL && terminal_state(TK_MOUSE_WHEEL) > 0))
			{
				if (offset < total_height-window_height) offset += 1;
			}
			else if (key == TK_RESIZED)
			{
				float percent = offset / (float)total_height;
				total_height = UpdateHeights(messages);
				window_width = terminal_state(TK_WIDTH) - 4 - 2;
				window_height = terminal_state(TK_HEIGHT) - 2;
				bar_height = std::min<int>(std::ceil(window_height * (window_height/(float)total_height)), window_height);
				offset = total_height * percent;
				offset = std::min(offset, total_height - window_height);
				if (total_height <= window_height) offset = 0;
				break;
			}
		}
		while (terminal_has_input());
	}

	terminal_set("window: size=80x25, resizeable=false; font: default; U+E100: none; U+E200: none; U+E300: none");
}
