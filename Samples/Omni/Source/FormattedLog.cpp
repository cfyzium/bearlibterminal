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

namespace { // anonymous

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

std::vector<int> alternative_fonts = { };

static std::string GenerateRandomMessage()
{
	std::ostringstream ss;

	// Total number of sentences in a message: [1..10].
	int n_sentences = 1 + rand()%9;
	for (int j=0; j<n_sentences; j++)
	{
		// Total number of words in a sentence: [5..15].
		int n_words = 5 + rand()%10;
		for (int i=0; i<n_words; i++)
		{
			// Pick up a random word from the vocabulary above.
			std::string word = vocabulary[rand()%vocabulary.size()];

			// First word in a sentence starts from capital letter.
			if (i == 0) word[0] = toupper(word[0]);

			// 1/10 chance to be randomly colored.
			bool colored = (rand()%10 == 0);

			// 1/20 chance to have another font face.
			bool alt_font = !alternative_fonts.empty() && (rand()%20 == 0);

			if (colored) ss << "[color=" << colors[rand()%colors.size()] << "]";
			if (alt_font) ss << "[font=" << alternative_fonts[rand()%alternative_fonts.size()] << "]";

			ss << word;

			if (alt_font) ss << "[/font]";
			if (colored) ss << "[/color]";

			if (i != n_words-1) ss << " ";
		}
		ss << ".";
		if (j != n_sentences-1) ss << " ";
	}

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

const int padding_left = 4;
const int padding_right = 4;
const int padding_top = 2;
const int padding_bottom = 2;
const int mouse_scroll_step = 2; // 2 text rows per mouse wheel step.

std::vector<Message> messages;
int frame_offset = 0;
int frame_width = 0;
int frame_height = 0;
int total_messages_height = 1;
int scrollbar_height = 0;
bool dragging_scrollbar = false;
int dragging_scrollbar_offset = 0;

void Reset()
{
	messages.clear();
	frame_offset = 0;
	dragging_scrollbar = false;
}

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

void ScrollToPixel(int py)
{
	py -= padding_top * terminal_state(TK_CELL_HEIGHT);
	float factor = py / ((float)frame_height * terminal_state(TK_CELL_HEIGHT));
	frame_offset = total_messages_height * factor;
	frame_offset = std::max(0, std::min(total_messages_height-frame_height, frame_offset));
}

} // namespace anonymous

void TestFormattedLog()
{
	terminal_set("window: resizeable=true, minimum-size=20x8; font: default");
	terminal_set("input.precise-mouse=true");
	terminal_set("0xE100: ../Media/Tigrex3drunes_16x16_437.PNG, spacing=2x1, transparent=auto");
	alternative_fonts.push_back(0xE100);

	Reset();

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

		// Drawing messages (+crop)
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
		int scrollbar_column = padding_left+frame_width;
		int scrollbar_offset =
			(padding_top + (frame_height-scrollbar_height) * (frame_offset / (float)(total_messages_height - frame_height))) *
			terminal_state(TK_CELL_HEIGHT);
		for (int i = 0; i < scrollbar_height; i++)
		{
			terminal_put_ext(scrollbar_column, i, 0, scrollbar_offset, 0x2588, 0);
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
			else if (key == TK_UP)
			{
				frame_offset = std::max(0, frame_offset-1);
			}
			else if (key == TK_DOWN)
			{
				frame_offset = std::min(total_messages_height-frame_height, frame_offset+1);
			}
			else if (key == TK_MOUSE_SCROLL)
			{
				// Mouse wheel scroll
				frame_offset += mouse_scroll_step * terminal_state(TK_MOUSE_WHEEL);
				frame_offset = std::max(0, std::min(total_messages_height-frame_height, frame_offset));
			}
			else if (key == TK_MOUSE_LEFT && terminal_state(TK_MOUSE_X) == scrollbar_column)
			{
				int py = terminal_state(TK_MOUSE_PIXEL_Y);
				if (py >= scrollbar_offset && py <= scrollbar_offset + (scrollbar_height * terminal_state(TK_CELL_HEIGHT)))
				{
					// Clicked on the scrollbar handle: start dragging
					dragging_scrollbar = true;
					dragging_scrollbar_offset = py - scrollbar_offset;
				}
				else
				{
					// Clicked outside of the handle: jump to position
					ScrollToPixel(terminal_state(TK_MOUSE_PIXEL_Y) - scrollbar_height * terminal_state(TK_CELL_HEIGHT) / 2);
				}
			}
			else if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED))
			{
				dragging_scrollbar = false;
			}
			else if (key == TK_MOUSE_MOVE && dragging_scrollbar)
			{
				ScrollToPixel(terminal_state(TK_MOUSE_PIXEL_Y) - dragging_scrollbar_offset);
			}
			else if (key == TK_RESIZED)
			{
				UpdateGeometry();
				break;
			}
		}
		while (terminal_has_input());
	}

	terminal_set("window: resizeable=false");
	terminal_set("U+E100: none; U+E200: none; U+E300: none");
	terminal_set("input.precise-mouse=false;");
}
