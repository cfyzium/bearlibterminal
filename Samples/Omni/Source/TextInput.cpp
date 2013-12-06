/*
 * TextInput.cpp
 *
 *  Created on: Dec 6, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

void DrawFrame(int x, int y, int w, int h)
{
	terminal_clear_area(x, y, w, h);

	for (int i=x; i<x+w; i++)
	{
		terminal_put(i, y, L'─');
		terminal_put(i, y+h-1, L'─');
	}

	for (int j=y; j<y+h; j++)
	{
		terminal_put(x, j, L'│');
		terminal_put(x+w-1, j, L'│');
	}

	terminal_put(x, y, L'┌');
	terminal_put(x+w-1, y, L'┐');
	terminal_put(x, y+h-1, L'└');
	terminal_put(x+w-1, y+h-1, L'┘');
}

void TestTextInput()
{
	terminal_set("window.title='Omni: text input'");
	terminal_composition(TK_COMPOSITION_OFF);

	const int max_chars = 32;
	wchar_t buffer[max_chars+1] = {0};
	wchar_t character = L' ';
	int result = 0;
	int char_result = 0;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		terminal_print(2, 1, "Select different input tests by pressing corresponding number:");

		terminal_print(2, 3, "[color=orange]1.[/color] terminal_read_str");
		DrawFrame(5, 4, max_chars+2, 3);
		terminal_printf(6, 5, "%ls", buffer);
		terminal_printf(5+max_chars+2+1, 5, "[color=gray]last operation: %s", result >=0? "OK": "cancelled");

		terminal_print(2, 8, "[color=orange]2.[/color] terminal_read_char");
		DrawFrame(5, 9, 5, 3);
		terminal_printf(7, 10, L"%lc", character);
		terminal_printf(5+3+2+1, 10, "[color=gray]last operation: %d", char_result);

		terminal_refresh();

		int key = terminal_read();
		if (key == TK_CLOSE || key == TK_ESCAPE)
		{
			proceed = false;
		}
		else if (key == TK_1)
		{
			terminal_color("orange");
			DrawFrame(5, 4, max_chars+2, 3);
			result = terminal_read_wstr(6, 5, buffer, max_chars);
		}
		else if (key == TK_2)
		{
			terminal_color("orange");
			DrawFrame(5, 9, 5, 3);
			character = L'?';

			do
			{
				terminal_put(7, 10, character);
				terminal_refresh();

				char_result = terminal_read_char();
				if (char_result == TK_INPUT_CALL_AGAIN)
				{
					char_result = terminal_read();
					break;
				}
				else if (char_result > 0)
				{
					character = char_result;
				}
			}
			while (true);
		}
	}
}
