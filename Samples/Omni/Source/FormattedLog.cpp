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
	"ad", "adipisicing", "aliqua", "aliquip", "amet", "anim", "aute", "cillum",
	"commodo", "consectetur", "consequat", "culpa", "cupidatat", "deserunt",
	"do", "dolore", "dolor", "dolore", "duis", "ea", "eiusmod", "elit", "enim",
	"esse", "est", "et", "ex", "exercitation", "excepteur", "eu", "fugiat",
	"id", "in", "incididunt", "ipsum", "irure", "labore", "laboris", "laborum",
	"lorem", "magna", "minim", "mollit", "nisi", "non", "nostrud", "nulla",
	"occaecat", "officia", "pariatur", "proident", "qui", "quis", "reprehenderit",
	"sed", "sit", "sint", "sunt", "tempor", "ullamco", "ut", "velit", "veniam",
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

std::vector<int> alternative_fonts =
{
	// ...
};

static std::string GenerateRandomMessage()
{
	std::ostringstream ss;

	// Total number of words in a message: [5..55].
	int n_words = 5 + rand()%50;
	for (int i=0; i<n_words; i++)
	{
		// Pick up a random word from the vocabulary above.
		std::string word = vocabulary[rand()%vocabulary.size()];

		// First word in a message starts from capital letter.
		if (i == 0) word[0] = toupper(word[0]);

		// 1/6 chance to be colored.
		bool colored = (rand()%6 == 0);

		// 1/8 chance to have another font face.
		bool alt_font = !alternative_fonts.empty() && (rand()%8 == 0);

		if (colored) ss << "[color=" << colors[rand()%colors.size()] << "]";
		if (alt_font) ss << "[font=" << alternative_fonts[rand()%alternative_fonts.size()] << "]";

		ss << word;

		if (alt_font) ss << "[/font]";
		if (colored) ss << "[/color]";

		if (i != n_words-1) ss << " ";
	}
	ss << ".";

	return ss.str();
}

// A string plus its precalculated height.
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

std::vector<Message> messages;

const int padding_left = 4;
const int padding_right = 4;
const int padding_top = 2;
const int padding_bottom = 2;

int frame_offset = 0;
int frame_width = 0;
int frame_height = 0;
int total_messages_height = 1;
int scrollbar_height = 0;

int UpdateHeights()
{
	int total_height = 0;
	for (auto& message: messages)
	{
		message.height = terminal_measuref("[bbox=%d]%s", frame_width, message.text.c_str());
		total_height += message.height;
	}

	// Add blank lines between messages
	total_height += messages.size()-1;

	return total_height;
}

void UpdateGeometry()
{
	// Save current scroll position
	float current_offset_percentage = frame_offset / (float)total_messages_height;

	// Update frame dimensions
	frame_width = terminal_state(TK_WIDTH) - (padding_left + padding_right + 1);
	frame_height = terminal_state(TK_HEIGHT) - (padding_top + padding_bottom);

	// Calculate new message list height
	total_messages_height = UpdateHeights();

	// Scrollbar
	scrollbar_height = std::min<int>(std::ceil(frame_height * (frame_height/(float)total_messages_height)), frame_height);

	// Try to recover scroll position
	frame_offset = total_messages_height * current_offset_percentage;
	frame_offset = std::min(frame_offset, total_messages_height - frame_height);
	if (total_messages_height <= frame_height) frame_offset = 0;
}

void TestFormattedLog()
{
	/*
	terminal_set("window: size=64x20, resizeable=true, minimum-size=20x8; font: ../Media/VeraMono.ttf, size=10x20");
	terminal_set("U+E100: ../Media/VeraMoIt.ttf, size=10x20");
	terminal_set("U+E200: ../Media/VeraMoBd.ttf, size=10x20");
	/*/
	terminal_set("window: resizeable=true, minimum-size=20x8; font: default");
	//terminal_set("U+E100: ../Media/VeraMoIt.ttf, size=8x16");
	//terminal_set("U+E200: ../Media/VeraMoBd.ttf, size=8x16");
	//*/
	//alternative_fonts.push_back(0xE100);
	//alternative_fonts.push_back(0xE200);

	//std::vector<Message> messages;

	const std::string prompt =
		"Use arrow keys or mouse wheel to scroll the list up and down. "
		"Try to resize the window.\n\n--- --- ---";
	messages.push_back(Message(prompt));

	// Add a dozen of random messages
	for (size_t i=0; i<10; i++)
	{
		messages.push_back(Message(GenerateRandomMessage()));
	}

	// Initial update
	UpdateGeometry();

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		// Frame background
		terminal_layer(0);
		terminal_bkcolor("darkest gray");
		terminal_clear_area(padding_left, padding_top, frame_width, frame_height);
		terminal_bkcolor("none");

		// Find topmost visible message
		int index = 0, first_line = 0;
		for (; first_line < total_messages_height; index++)
		{
			auto& message = messages[index];
			if (first_line + message.height >= frame_offset)
			{
				// This message is partially visible
				break;
			}

			first_line += message.height + 1;
		}
		int delta = first_line - frame_offset;

		// Drawin messages (+crop)
		terminal_layer(1);
		for (; index < messages.size() && delta <= frame_height; index++)
		{
			auto& message = messages[index];
			terminal_printf(padding_left, padding_top+delta, "[bbox=%d]%s", frame_width, message.text.c_str());
			delta += message.height + 1;
		}
		terminal_crop(padding_left, padding_top, frame_width, frame_height);

		// Scroll bar
		terminal_layer(0);
		terminal_bkcolor("darker gray");
		terminal_clear_area(padding_left+frame_width, padding_top, 1, frame_height);
		terminal_bkcolor("none");
		terminal_color("dark orange");
		float bar_offset = (frame_height-scrollbar_height) * (frame_offset / (float)(total_messages_height - frame_height));
		for (int i = 0; i < scrollbar_height; i++)
		{
			terminal_put_ext(padding_left+frame_width, padding_top+i, 0, bar_offset*16, 0x2588, 0);
		}

		// Render
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
				if (frame_offset > 0) frame_offset -= 1;
			}
			else if (key == TK_DOWN || (key == TK_MOUSE_SCROLL && terminal_state(TK_MOUSE_WHEEL) > 0))
			{
				if (frame_offset < total_messages_height-frame_height) frame_offset += 1;
			}
			else if (key == TK_RESIZED)
			{
				UpdateGeometry();//messages);
				break;
			}
		}
		while (terminal_has_input());
	}

	/*
	terminal_set("window: size=80x25, resizeable=false; font: default; U+E100: none; U+E200: none; U+E300: none");
	/*/
	terminal_set("window: size=80x25, resizeable=false; font: default; U+E100: none; U+E200: none; U+E300: none");
	//*/
}
