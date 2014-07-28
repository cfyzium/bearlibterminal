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

#ifndef BEARLIBTERMINAL_RECTANGLE_HPP
#define BEARLIBTERMINAL_RECTANGLE_HPP

#include "Point.hpp"
#include "Size.hpp"

namespace BearLibTerminal
{
	template<typename T> struct BasicRectangle
	{
		BasicRectangle():
			left(),
			top(),
			width(),
			height()
		{ }

		BasicRectangle(T left, T top, T width, T height):
			left(left),
			top(top),
			width(width),
			height(height)
		{ }

		BasicRectangle(BasicSize<T> size):
			left(),
			top(),
			width(size.width),
			height(size.height)
		{ }

		BasicRectangle(BasicPoint<T> location, BasicSize<T> size):
			left(location.x),
			top(location.y),
			width(size.width),
			height(size.height)
		{ }

		BasicPoint<T> Location() const
		{
			return BasicPoint<T>(left, top);
		}

		BasicSize<T> Size() const
		{
			return BasicSize<T>(width, height);
		}

		T Area() const
		{
			return width*height;
		}

		bool Contains(BasicPoint<T> point) const
		{
			return
				(point.x >= left) &&
				(point.y >= top) &&
				(point.x < left+width) &&
				(point.y < top+height);
		}

		bool Contains(BasicRectangle<T> rectangle) const
		{
			return
				(rectangle.left >= left) &&
				(rectangle.top >= top) &&
				(rectangle.left+rectangle.width <= left+width) &&
				(rectangle.top+rectangle.height <= top+height);
		}

		T left, top, width, height;
	};

	template<typename T> BasicRectangle<T> operator*(const BasicRectangle<T>& rect, const BasicSize<T>& factor)
	{
		return BasicRectangle<T>
		(
			rect.left * factor.width,
			rect.top * factor.height,
			rect.width * factor.width,
			rect.height * factor.height
		);
	}

	template<typename T> BasicRectangle<T> operator*=(const BasicRectangle<T>& rect, const BasicSize<T>& factor)
	{
		return rect * factor;
	}

	template<typename T> BasicRectangle<T> operator+(const BasicRectangle<T>& rect, const BasicPoint<T>& offset)
	{
		return BasicRectangle<T>(rect.left + offset.x, rect.top + offset.y, rect.width, rect.height);
	}

	template<typename T> BasicRectangle<T>& operator+=(BasicRectangle<T>& rect, const BasicPoint<T>& offset)
	{
		rect = rect + offset;
		return rect;
	}

	typedef BasicRectangle<int> Rectangle;

	typedef BasicRectangle<float> RectangleF;
}

#endif // BEARLIBTERMINAL_RECTANGLE_HPP
