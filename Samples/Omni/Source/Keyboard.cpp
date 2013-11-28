/*
 * Keyboard.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

struct mkey_t
{
	unsigned char vk;
	int x, y, w, h;
	const char* caption;
};

static void FillRectangle(int x, int y, int w, int h, color_t color)
{
	terminal_color(color);

	for ( int i = x; i < x+w; i++ )
	{
		for ( int j = y; j < y+h; j++ )
		{
			terminal_put(i, j, 0x2588);
		}
	}

	for ( int i = x; i < x+w; i++ )
	{
		terminal_put(i, y-1, 0x2584);
		terminal_put(i, y+h, 0x2580);
	}

	for ( int j = y; j < y+h; j++ )
	{
		terminal_put(x-1, j, 0x2590);
		terminal_put(x+w, j, 0x258C);
	}

	terminal_put(x-1, y-1, 0x2597);
	terminal_put(x-1, y+h, 0x259D);
	terminal_put(x+w, y-1, 0x2596);
	terminal_put(x+w, y+h, 0x2598);
};

void TestKeyboard()
{
	terminal_set("window.title='Omni: basic keyboard input'");
	terminal_set("input.events=keypress+keyrelease");
	terminal_composition(TK_COMPOSITION_ON);

	//
	//	"┌───┐┌──┬──┬──┬──┐┌──┬──┬──┬──┐┌──┬───┬───┬───┐┌─────┬─────┬───────┐",
	//	"│ESC││F1│F2│F3│F4││F5│F6│F7│F8││F9│F10│F11│F12││PRSCR│SCRLK│ PAUSE │",
	//	"└───┘└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴───┴───┴───┘└─────┴─────┴───────┘",
	//	"┌───┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─────┐┌────┬────┬────┐┌───┬───┬───┬───┐",
	//	"│ ` │1│2│3│4│5│6│7│8│9│0│─│=│BCKSP││INS │HOME│PGUP││NUM│ / │ * │ ─ │",
	//	"├───┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬────┤├────┼────┼────┤├───┼───┼───┼───┤",
	//	"│TAB │q│w│e│r│t│y│u│i│o│p│[│]│  E ││DEL │END │PGDN││ 7 │ 8 │ 9 │   │",
	//	"├────┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┐ N │└────┴────┴────┘├───┼───┼───┤ + │",
	//	"│CAPS │a│s│d│f│g│h│j│k│l│;│'│\│ T │                │ 4 │ 5 │ 6 │   │",
	//	"├─────┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴─┴───┤     ┌────┐     ├───┼───┼───┼───┤",
	//	"│SHIFT │z│x│c│v│b│n│m│,│.│/│ SHIFT│     │ UP │     │ 1 │ 2 │ 3 │ E │",
	//	"├────┬─┴┬┴─┴┬┴─┴─┴┬┴─┴┬┴─┼─┴─┬────┤┌────┼────┼────┐├───┴───┼───┤ N │",
	//	"│CTRL│LW│ALT│SPACE│ALT│RW│CTX│CTRL││LEFT│DOWN│RGHT││ 0     │ . │ T │",
	//	"└────┴──┴───┴─────┴───┴──┴───┴────┘└────┴────┴────┘└───────┴───┴───┘"
	//

	const wchar_t* grid[] =
	{
		L"┌───┐┌──┬──┬──┬──┐┌──┬──┬──┬──┐┌──┬───┬───┬───┐┌─────┬─────┬───────┐",
		L"│   ││  │  │  │  ││  │  │  │  ││  │   │   │   ││     │     │       │",
		L"└───┘└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴───┴───┴───┘└─────┴─────┴───────┘",
		L"┌───┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─────┐┌────┬────┬────┐┌───┬───┬───┬───┐",
		L"│   │ │ │ │ │ │ │ │ │ │ │ │ │     ││    │    │    ││   │   │   │   │",
		L"├───┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬────┤├────┼────┼────┤├───┼───┼───┼───┤",
		L"│    │ │ │ │ │ │ │ │ │ │ │ │ │    ││    │    │    ││   │   │   │   │",
		L"├────┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┐   │└────┴────┴────┘├───┼───┼───┤   │",
		L"│     │ │ │ │ │ │ │ │ │ │ │ │ │   │                │   │   │   │   │",
		L"├─────┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴─┴───┤     ┌────┐     ├───┼───┼───┼───┤",
		L"│      │ │ │ │ │ │ │ │ │ │ │      │     │    │     │   │   │   │   │",
		L"├────┬─┴┬┴─┴┬┴─┴─┴┬┴─┴┬┴─┼─┴─┬────┤┌────┼────┼────┐├───┴───┼───┤   │",
		L"│    │  │   │     │   │  │   │    ││    │    │    ││       │   │   │",
		L"└────┴──┴───┴─────┴───┴──┴───┴────┘└────┴────┴────┘└───────┴───┴───┘"
	};

	const size_t N_grid_lines = sizeof(grid)/sizeof(grid[0]);

	mkey_t keys[] =
	{
		// Functional keys row
		{ TK_ESCAPE, 1, 1, 3, 1, "ESC" },
		{ TK_F1, 6, 1, 2, 1, "F1" },
		{ TK_F2, 9, 1, 2, 1, "F2" },
		{ TK_F3, 12, 1, 2, 1, "F3" },
		{ TK_F4, 15, 1, 2, 1, "F4" },
		{ TK_F5, 19, 1, 2, 1, "F5" },
		{ TK_F6, 22, 1, 2, 1, "F6" },
		{ TK_F7, 25, 1, 2, 1, "F7" },
		{ TK_F8, 28, 1, 2, 1, "F8" },
		{ TK_F9, 32, 1, 2, 1, "F9" },
		{ TK_F10, 35, 1, 3, 1, "F10" },
		{ TK_F11, 39, 1, 3, 1, "F11" },
		{ TK_F12, 43, 1, 3, 1, "F12" },
		{ TK_PAUSE, 60, 1, 7, 1, " PAUSE " },
		// First row
		{ TK_GRAVE, 1, 4, 3, 1, " ` " },
		{ TK_1, 5, 4, 1, 1, "1" },
		{ TK_2, 7, 4, 1, 1, "2" },
		{ TK_3, 9, 4, 1, 1, "3" },
		{ TK_4, 11, 4, 1, 1, "4" },
		{ TK_5, 13, 4, 1, 1, "5" },
		{ TK_6, 15, 4, 1, 1, "6" },
		{ TK_7, 17, 4, 1, 1, "7" },
		{ TK_8, 19, 4, 1, 1, "8" },
		{ TK_9, 21, 4, 1, 1, "9" },
		{ TK_0, 23, 4, 1, 1, "0" },
		{ TK_MINUS, 25, 4, 1, 1, "-" },
		{ TK_EQUALS, 27, 4, 1, 1, "=" },
		{ TK_BACKSPACE, 29, 4, 5, 1, "BCKSP" },
		// Second row
		{ TK_TAB, 1, 6, 4, 1, "TAB" },
		{ TK_Q, 6, 6, 1, 1, "q" },
		{ TK_W, 8, 6, 1, 1, "w" },
		{ TK_E, 10, 6, 1, 1, "e" },
		{ TK_R, 12, 6, 1, 1, "r" },
		{ TK_T, 14, 6, 1, 1, "t" },
		{ TK_Y, 16, 6, 1, 1, "y" },
		{ TK_U, 18, 6, 1, 1, "u" },
		{ TK_I, 20, 6, 1, 1, "i" },
		{ TK_O, 22, 6, 1, 1, "o" },
		{ TK_P, 24, 6, 1, 1, "p" },
		{ TK_LBRACKET, 26, 6, 1, 1, "[[" },
		{ TK_RBRACKET, 28, 6, 1, 1, "]]" },
		// Third row
		// { VK_, 1, 8, 5, 1, "a" },
		{ TK_A, 7, 8, 1, 1, "a" },
		{ TK_S, 9, 8, 1, 1, "s" },
		{ TK_D, 11, 8, 1, 1, "d" },
		{ TK_F, 13, 8, 1, 1, "f" },
		{ TK_G, 15, 8, 1, 1, "g" },
		{ TK_H, 17, 8, 1, 1, "h" },
		{ TK_J, 19, 8, 1, 1, "j" },
		{ TK_K, 21, 8, 1, 1, "k" },
		{ TK_L, 23, 8, 1, 1, "l" },
		{ TK_SEMICOLON, 25, 8, 1, 1, ";" },
		{ TK_APOSTROPHE, 27, 8, 1, 1, "'" },
		{ TK_BACKSLASH, 29, 8, 1, 1, "\\" },
		// Fourth row
		{ TK_SHIFT, 1, 10, 6, 1, "SHIFT" },
		{ TK_Z, 8, 10, 1, 1, "z" },
		{ TK_X, 10, 10, 1, 1, "x" },
		{ TK_C, 12, 10, 1, 1, "c" },
		{ TK_V, 14, 10, 1, 1, "v" },
		{ TK_B, 16, 10, 1, 1, "b" },
		{ TK_N, 18, 10, 1, 1, "n" },
		{ TK_M, 20, 10, 1, 1, "m" },
		{ TK_COMMA, 22, 10, 1, 1, "," },
		{ TK_PERIOD, 24, 10, 1, 1, "." },
		{ TK_SLASH, 26, 10, 1, 1, "/" },
		{ TK_SHIFT, 28, 10, 6, 1, " SHIFT" },
		// Fifth row
		{ TK_CONTROL, 1, 12, 4, 1, "CTRL" },
		{ TK_SPACE, 13, 12, 5, 1, "SPACE" },
		{ TK_CONTROL, 30, 12, 4, 1, "CTRL" },
		// Navigation
		{ TK_INSERT, 36, 4, 4, 1, "INS" },
		{ TK_HOME, 41, 4, 4, 1, "HOME" },
		{ TK_PRIOR, 46, 4, 4, 1, "PGUP" },
		{ TK_DELETE, 36, 6, 4, 1, "DEL" },
		{ TK_END, 41, 6, 4, 1, "END" },
		{ TK_NEXT, 46, 6, 4, 1, "PGDN" },
		{ TK_UP, 41, 10, 4, 1, " UP " },
		{ TK_LEFT, 36, 12, 4, 1, "LEFT" },
		{ TK_DOWN, 41, 12, 4, 1, "DOWN" },
		{ TK_RIGHT, 46, 12, 4, 1, "RGHT" },
		// Numpad
		{ TK_DIVIDE, 56, 4, 3, 1, " / " },
		{ TK_MULTIPLY, 60, 4, 3, 1, " * " },
		{ TK_SUBTRACT, 64, 4, 3, 1, " - " },
		{ TK_NUMPAD7, 52, 6, 3, 1, " 7 " },
		{ TK_NUMPAD8, 56, 6, 3, 1, " 8 " },
		{ TK_NUMPAD9, 60, 6, 3, 1, " 9 " },
		{ TK_ADD, 64, 6, 3, 3, " + " },
		{ TK_NUMPAD4, 52, 8, 3, 1, " 4 " },
		{ TK_NUMPAD5, 56, 8, 3, 1, " 5 " },
		{ TK_NUMPAD6, 60, 8, 3, 1, " 6 " },
		{ TK_NUMPAD1, 52, 10, 3, 1, " 1 " },
		{ TK_NUMPAD2, 56, 10, 3, 1, " 2 " },
		{ TK_NUMPAD3, 60, 10, 3, 1, " 3 " },
		{ TK_NUMPAD0, 52, 12, 7, 1, " 0 " },
		{ TK_DECIMAL, 60, 12, 3, 1, " . " },
	};

	const size_t N_keys = sizeof(keys)/sizeof(keys[0]);

	mkey_t unavailable_keys[] =
	{
		{ 0, 1, 8, 5, 1, "CAPS " },
		{ 0, 6, 12, 2, 1, "LW" },
		{ 0, 9, 12, 3, 1, "ALT" },
		{ 0, 19, 12, 3, 1, "ALT" },
		{ 0, 23, 12, 2, 1, "RW" },
		{ 0, 26, 12, 3, 1, "CTX" },
		{ 0, 48, 1, 5, 1, "PRSCR" },
		{ 0, 54, 1, 5, 1, "SCRLK" },
		{ 0, 52, 4, 3, 1, "NUM" }
	};

	const size_t N_unavailable_keys = sizeof(unavailable_keys)/sizeof(unavailable_keys[0]);

	const color_t normal_text = 0xFFFFFFFF;
	const color_t pressed_key = 0xFFBB6000;
	const color_t pressed_key_text = 0xFF000000;
	const color_t available_key_text = 0xFFBBBBBB;
	const color_t unavailable_key_text = 0xFF404040;
	const color_t grid_color = 0xFF606060;
	const color_t note_text_color = 0xFF008000;

	bool proceed = true;
	while (proceed)
	{
		terminal_clear();

		for (size_t i = 0; i < N_keys; i++)
		{
			if (terminal_state(keys[i].vk))
			{
				FillRectangle(6+keys[i].x, 1+keys[i].y, keys[i].w, keys[i].h, pressed_key);
				terminal_color(pressed_key_text);
				terminal_printf(6+keys[i].x, 1+keys[i].y, "%s", keys[i].caption);
			}
			else
			{
				terminal_color(available_key_text);
				terminal_printf(6+keys[i].x, 1+keys[i].y, "%s", keys[i].caption);
			}
		}

		// Special case: Enter keys
		if (terminal_state(TK_RETURN))
		{
			// Main keyboard
			FillRectangle(6+30, 1+6, 4, 1, pressed_key);
			FillRectangle(6+31, 1+8, 3, 1, pressed_key);
			// Numpad
			FillRectangle(6+64, 1+10, 3, 3, pressed_key);
		}

		terminal_color(terminal_state(TK_RETURN)? pressed_key_text: available_key_text);
		// Main keyboard
		terminal_put(6+32, 1+6+0, 'E');
		terminal_put(6+32, 1+6+1, 'N');
		terminal_put(6+32, 1+6+2, 'T');
		// Numpad
		terminal_put(6+65, 1+10+0, 'E');
		terminal_put(6+65, 1+10+1, 'N');
		terminal_put(6+65, 1+10+2, 'T');

		terminal_color(grid_color);
		for ( size_t i=0; i<N_grid_lines; i++ )
		{
			terminal_wprintf(6, 1+i, L"%ls", grid[i]);
		}

		terminal_color(unavailable_key_text);
		for (int i=0; i < N_unavailable_keys; i++)
		{
			mkey_t& k = unavailable_keys[i];
			terminal_printf(6+k.x, 1+k.y, "%s", k.caption);
		}

		terminal_color(normal_text);
		terminal_printf(6, 1+15, "[color=orange]NOTE:[/color] keys printed in dark gray color are not available by design.");
		terminal_printf(6, 1+17, "[color=orange]NOTE:[/color] for demonstration purposes Escape will not close this demo;");
		terminal_printf(6, 1+18, "use Shift+Escape combination to exit.");

		terminal_refresh();

		do
		{
			int key = terminal_read();
			if (key == TK_CLOSE || (key == TK_ESCAPE && terminal_state(TK_SHIFT)))
			{
				proceed = false; break;
			}
		}
		while (terminal_has_input());
	}

	terminal_composition(TK_COMPOSITION_OFF);
	terminal_set("input.events=keypress");
}
