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
		T left, top, width, height;

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

		template<typename U> BasicRectangle(const BasicRectangle<U>& from):
			left(from.left),
			top(from.top),
			width(from.width),
			height(from.height)
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

		BasicRectangle<T> Intersection(BasicRectangle<T> other) const
		{
			T right = left + width;
			T bottom = top + height;
			T other_right = other.left + other.width;
			T other_bottom = other.top + other.height;

			if ((other.left > right) ||
				(other_right < left) ||
				(other.top > bottom) ||
				(other_bottom < top))
			{
				return BasicRectangle<T>(left, top, 0, 0);
			}

			BasicRectangle<T> result;
			result.left = std::max(left, other.left);
			result.top = std::max(top, other.top);
			result.width = std::min(right, other_right) - result.left;
			result.height = std::min(bottom, other_bottom) - result.top;
			return result;
		}

		template<typename U> BasicRectangle<T> operator+(BasicPoint<U> offset) const
		{
			return BasicRectangle<T>(left + offset.x, top + offset.y, width, height);
		}

		template<typename U> BasicRectangle<T> operator+(BasicSize<U> delta) const
		{
			return BasicRectangle<T>(left, top, width + delta.width, height + delta.height);
		}

		template<typename U> BasicRectangle<T> operator-(BasicPoint<U> offset) const
		{
			return BasicRectangle<T>(left - offset.x, top - offset.y, width, height);
		}

		template<typename U> BasicRectangle<T> operator-(BasicSize<U> delta) const
		{
			return BasicRectangle<T>(left, top, width - delta.width, height - delta.height);
		}

		template<typename U> BasicRectangle<T> operator*(BasicSize<U> factor) const
		{
			return BasicRectangle<T>(left * factor.width, top * factor.height, width * factor.width, height * factor.height);
		}

		template<typename U> BasicRectangle<T> operator/(BasicSize<U> factor) const
		{
			return BasicRectangle<T>(left / factor.width, top / factor.height, width / factor.width, height / factor.height);
		}

		template<typename U> BasicRectangle<T>& operator+=(BasicPoint<U> offset)
		{
			left += offset.x;
			top += offset.y;
			return (*this);
		}

		template<typename U> BasicRectangle<T>& operator+=(BasicSize<U> delta)
		{
			width += delta.width;
			height += delta.height;
			return (*this);
		}

		template<typename U> BasicRectangle<T>& operator-=(BasicPoint<U> offset)
		{
			left -= offset.x;
			top -= offset.y;
			return (*this);
		}

		template<typename U> BasicRectangle<T>& operator-=(BasicSize<U> delta)
		{
			width -= delta.width;
			height -= delta.height;
			return (*this);
		}

		template<typename U> BasicRectangle<T>& operator*=(BasicSize<U> factor)
		{
			left *= factor.width;
			top *= factor.height;
			width *= factor.width;
			height *= factor.height;
			return (*this);
		}

		template<typename U> BasicRectangle<T>& operator/=(BasicSize<U> factor)
		{
			left /= factor.width;
			top /= factor.height;
			width /= factor.width;
			height /= factor.height;
			return (*this);
		}
	};

	typedef BasicRectangle<int> Rectangle;

	typedef BasicRectangle<float> RectangleF;
}

#endif // BEARLIBTERMINAL_RECTANGLE_HPP
