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

#ifndef BEARLIBTERMINAL_POINT_HPP
#define BEARLIBTERMINAL_POINT_HPP

namespace BearLibTerminal
{
	template<typename T> struct BasicPoint
	{
		typedef T value_type;

		T x, y;

		BasicPoint():
			x(0),
			y(0)
		{ }

		BasicPoint(T x, T y):
			x(x),
			y(y)
		{ }

		BasicPoint(const BasicPoint<T>& from):
			x(from.x),
			y(from.y)
		{ }

		inline bool operator==(BasicPoint<T> other) const
		{
			return x == other.x && y == other.y;
		}

		inline bool operator!= (BasicPoint<T> other) const
		{
			return x != other.x || y != other.y;
		}

		inline BasicPoint<T> operator+(BasicPoint<T> other)
		{
			return BasicPoint<T>(x+other.x, y+other.y);
		}

		inline BasicPoint<T> operator-(BasicPoint<T> other)
		{
			return BasicPoint<T>(x-other.x, y-other.y);
		}

		template<typename U> BasicPoint<T> operator*(U factor)
		{
			return BasicPoint<T>(x * factor, y * factor);
		}

		template<typename U> BasicPoint<T> operator/(U factor)
		{
			return BasicPoint<T>(x / factor, y / factor);
		}
	};

	typedef BasicPoint<int> Point;

	typedef BasicPoint<float> PointF;
}

#endif // BEARLIBTERMINAL_POINT_HPP
