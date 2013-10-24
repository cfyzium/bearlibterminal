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

#ifndef BEARLIBTERMINAL_COLOR_HPP
#define BEARLIBTERMINAL_COLOR_HPP

#include <cstdint>

namespace BearLibTerminal
{
	struct Color
	{
		std::uint8_t b, g, r, a; // BGRA8 format

		Color():
			b(0), g(0), r(0), a(0)
		{ }

		Color(std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue):
			b(blue), g(green), r(red), a(alpha)
		{ }

		Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue):
			b(blue), g(green), r(red), a(0xFF)
		{ }

		Color(std::uint32_t bgra)//:\
			b(), g(), r(), a()
		{
			*(std::uint32_t*)this = bgra;
		}

		bool operator== (const Color& another) const
		{
			return *(const std::uint32_t*)this == *(const std::uint32_t*)&another;
		}

		bool operator!= (const Color& another) const
		{
			return !(*this == another);
		}

		operator uint32_t() const
		{
			return *(const std::uint32_t*)this;
		}

		Color operator+ (Color other)
		{
			return Color(r+other.r, g+other.g, b+other.b, a+other.a);
		}
	};
}

#endif // BEARLIBTERMINAL_COLOR_HPP
