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

#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include "Size.hpp"
#include "Point.hpp"
#include "Rectangle.hpp"
#include <istream>
#include <ostream>

namespace BearLibTerminal
{
	// stream << Rectangle
	template<typename char_t, typename T> std::basic_ostream<char_t>& operator<< (std::basic_ostream<char_t>& stream, const BasicRectangle<T>& s)
	{
			stream << s.left << "," << s.top << "-" << s.width << "x" << s.height;
			return stream;
	}

	// stream << Size
	template<typename char_t, typename T> std::basic_ostream<char_t>& operator<< (std::basic_ostream<char_t>& stream, const BasicSize<T>& s)
	{
			stream << s.width << "x" << s.height;
			return stream;
	}

	// stream >> Size
	template<typename char_t, typename T> std::basic_istream<char_t>& operator>> (std::basic_istream<char_t>& stream, BasicSize<T>& s)
	{
			stream >> s.width;
			stream.ignore(1);
			stream >> s.height;
			return stream;
	}

	// stream << Point
	template<typename char_t, typename T> std::basic_ostream<char_t>& operator<< (std::basic_ostream<char_t>& stream, const BasicPoint<T>& s)
	{
			stream << s.x << "," << s.y;
			return stream;
	}

	// stream >> Point
	template<typename char_t, typename T> std::basic_istream<char_t>& operator>> (std::basic_istream<char_t>& stream, BasicPoint<T>& s)
	{
			stream >> s.x;
			stream.ignore(1);
			stream >> s.y;
			return stream;
	}
}

#endif /* GEOMETRY_HPP_ */
