/*
* BearLibTerminal
* Copyright (C) 2013-2014 Cfyz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BEARLIBTERMINAL_KEYSTROKE_HPP
#define BEARLIBTERMINAL_KEYSTROKE_HPP

#include <cstdint>
#include <vector>

namespace BearLibTerminal
{
	struct Keystroke
	{
		static constexpr std::uint32_t
			None        = 0,
			KeyPress    = (1<<0),
			KeyRelease  = (1<<1),
			Keys        = KeyPress|KeyRelease,
			MouseMove   = (1<<2),
			MouseScroll = (1<<3),
			Mouse       = MouseMove|MouseScroll,
			Unicode     = (1<<4),
			All = Keys|Mouse|Unicode;

		typedef std::uint32_t Type;

		Keystroke(Type type, std::uint8_t scancode); // keypress/keyrelease events
		Keystroke(Type type, std::uint8_t scancode, char16_t character); // character-producing keypress event
		Keystroke(Type type, std::uint8_t scancode, int x, int y, int z); // mouse events

		Type type;
		std::uint8_t scancode;
		char16_t character;
		int x, y, z;
	};

	struct Event
	{
		enum class Domain
		{
			System,    // TK_CLOSE, TK_WINDOW_RESIZE, ...
			Keyboard,  // TK_A, TK_0, ...
			Mouse      // TK_LBUTTON, TK_MOUSE_MOVE, ...
		};

		struct Property
		{
			int slot;  // TK_A, TK_MOUSE_X, TK_CHAR
			int value; // boolean or integer value of a slot
		};

		Domain domain;
		int code;
		std::vector<Property> properties;

		Event(int code);
		Event(int code, std::vector<Property> properties);

		static Domain GetDomainByCode(int code);
	};
}

#endif // BEARLIBTERMINAL_KEYSTROKE_HPP
