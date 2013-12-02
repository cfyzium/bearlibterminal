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

#ifndef BEARLIBTERMINAL_BITMAP_HPP
#define BEARLIBTERMINAL_BITMAP_HPP

#include <vector>
#include <stddef.h>
#include "Point.hpp"
#include "Size.hpp"
#include "Rectangle.hpp"
#include "Color.hpp"

namespace BearLibTerminal
{
	class Bitmap
	{
	public:
		Bitmap();
		Bitmap(Size size, Color color);
		Bitmap(Size size, const Color* data);
		Bitmap(const Bitmap& from);
		Bitmap(Bitmap&& from);
		void Swap(Bitmap& other);
		Bitmap& operator=(Bitmap other);
		Size GetSize() const;
		bool IsEmpty() const;
		void Blit(const Bitmap& src, Rectangle src_region, Point dst_location);
		void Blit(const Bitmap& src, Point location);
		void BlitUnchecked(const Bitmap& src, Point location);
		Bitmap Extract(Rectangle region);
		const Color& operator() (Point p) const;
		const Color& operator() (int x, int y) const;
		Color& operator() (Point p);
		Color& operator() (int x, int y);
		const Color* GetData() const;
		bool HasAlpha() const;
		void MakeTransparent(Color color);
		Bitmap Resize(Size size);

	protected:
		Size m_size;
		std::vector<Color> m_data;
	};
}

#endif // BEARLIBTERMINAL_BITMAP_HPP
