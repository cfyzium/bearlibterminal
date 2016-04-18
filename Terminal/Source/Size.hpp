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

#ifndef BEARLIBTERMINAL_SIZE_HPP
#define BEARLIBTERMINAL_SIZE_HPP

#include <type_traits>
#include <cmath>

namespace BearLibTerminal
{
	template<typename T> struct BasicSize
	{
		typedef T value_type;

		T width, height;

		BasicSize():
			width(0),
			height(0)
		{ }

		BasicSize(T width, T height):
			width(width),
			height(height)
		{ }

		template<typename U> BasicSize(BasicSize<U> from):
			width(from.width),
			height(from.height)
		{ }

		bool operator==(BasicSize<T> other) const
		{
			return width == other.width && height == other.height;
		}

		bool operator!=(BasicSize<T> other) const
		{
			return width != other.width || height != other.height;
		}

		BasicSize<T> operator+(BasicSize<T> other) const
		{
			return BasicSize<T>(width+other.width, height+other.height);
		}

		BasicSize<T> operator-(BasicSize<T> other) const
		{
			return BasicSize<T>(width-other.width, height-other.height);
		}

		template<typename U, typename R = typename std::common_type<T, U>::type> BasicSize<R> operator*(BasicSize<U> factor) const
		{
			return BasicSize<R>(width*factor.width, height*factor.height);
		}

		template<typename U> BasicSize<T> operator*(U factor) const
		{
			return BasicSize<T>(T(width*factor), T(height*factor));
		}

		template<typename U> BasicSize<T>& operator*=(U factor)
		{
			width *= factor;
			height *= factor;
			return *this;
		}

		template<typename U, typename R = typename std::common_type<T, U>::type> BasicSize<R> operator/(BasicSize<U> factor) const
		{
			return BasicSize<R>(width/factor.width, height/factor.height);
		}

		template<typename U> BasicSize<T> operator/(U factor) const
		{
			return BasicSize<T>(T(width/factor), T(height/factor));
		}

		template<typename U> BasicSize<T>& operator/=(U factor)
		{
			width /= factor;
			height /= factor;
			return *this;
		}

		T Area() const
		{
			return width*height;
		}

		template<typename U> BasicSize<U> As() const
		{
			return BasicSize<U>(width, height);
		}
	};

	typedef BasicSize<int> Size;

	typedef BasicSize<float> SizeF;
}

namespace std
{
	template<typename T> BearLibTerminal::BasicSize<T> floor(BearLibTerminal::BasicSize<T> value)
	{
		return BearLibTerminal::BasicSize<T>(std::floor(value.width), std::floor(value.height));
	}
}

#endif // BEARLIBTERMINAL_SIZE_HPP
