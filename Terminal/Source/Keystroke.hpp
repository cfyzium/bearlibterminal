/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
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

namespace BearLibTerminal
{
	struct Keystroke
	{
		static const std::uint32_t
			None        = 0,
			KeyPress    = (1<<0),
			KeyRelease  = (1<<1),
			MouseMove   = (1<<2),
			MouseScroll = (1<<3),
			Unicode     = (1<<4),
			All = KeyPress|KeyRelease|MouseMove|MouseScroll|Unicode;

		typedef std::uint32_t Type;

		Keystroke(Type type, std::uint8_t scancode): // keypress/keyrelease events
			type(type),
			scancode(scancode),
			character(0),
			x(0), y(0), z(0)
		{ }

		Keystroke(Type type, std::uint8_t scancode, char16_t character): // character-producing keypress event
			type(type),
			scancode(scancode),
			character(character),
			x(0), y(0), z(0)
		{ }

		Keystroke(Type type, std::uint8_t scancode, int x, int y, int z): // mouse events
			type(type),
			scancode(scancode),
			character(0),
			x(x), y(y), z(z)
		{ }

		Type type;
		std::uint8_t scancode;
		char16_t character;
		int x, y, z;
	};
}

#endif // BEARLIBTERMINAL_KEYSTROKE_HPP
