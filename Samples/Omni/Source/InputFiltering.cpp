#include "Common.hpp"
#include <utility>
#include <vector>

void TestInputFiltering()
{
	terminal_set("window.title='Omni: input filtering'");

	std::vector<std::pair<const char*, int>> events =
	{
		{"0123456789", 1}, // 0 - disabled, 1 - keypress, 2 - both keypress and keyrelease
		{"close",      1},
		{"escape",     1},
		{"q",          0},
		{"abc",        0},
		{"keyboard",   0},
		{"mouse-left", 0},
		{"mouse",      0}
	};

	const char* colors[] = {"dark gray", "white", "lightest blue"};

	auto apply_input_filter = [&events]()
	{
		std::ostringstream ss;
		for (auto& event: events)
		{
			if (event.second == 0) continue; // disabled
			ss << event.first; // keypress
			if (event.second == 2) ss << "+"; // keyrelease too
			ss << ", ";
		}

		terminal_setf("input.filter=[%s]", ss.str().c_str());
	};

	apply_input_filter();

	int event_counter = 0;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		int h = terminal_printf
		(
			2, 1,
			"[bbox=76]Modify input filter by pressing corresponding numbers (digits are added "
			"to filter automatically). Gray color ([color=%s]like this[/color]) means that "
			"event is disabled. Regular white color means keypress is enabled. Blueish color "
			"([color=%s]like this[/color]) means both keypress and keyrelease are enabled.\n\n"
			"Both CLOSE and ESCAPE close this demo.",
			colors[0], colors[2]
		);

		for (size_t i = 0; i < events.size(); i++)
		{
			terminal_printf
			(
				2, 1+h+1+i,
				"[color=orange]%i[/color]. [color=%s]%s",
				i, colors[events[i].second], events[i].first
			);
		}

		terminal_printf
		(
			2, 1+h+1+events.size()+1,
			"Events read: [color=orange]%i",
			event_counter
		);

		terminal_refresh();

		do
		{
			int key = terminal_read();
			event_counter += 1;

			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
				break;
			}
			else if (key >= TK_1 && key <= TK_9)
			{
				int index = (key - TK_1) + 1;
				if (index < events.size())
				{
					auto& event = events[index];
					event.second = (event.second + 1) % 3;
				}

				apply_input_filter();
			}
		}
		while (proceed && terminal_has_input());
	}

	terminal_set("input.filter={keyboard}");
}
