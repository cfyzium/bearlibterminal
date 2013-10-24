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

#include "Bitmap.hpp"
#include <stdexcept>
#include <cstring>

namespace BearLibTerminal
{
	Bitmap::Bitmap()
	{ }

	Bitmap::Bitmap(Size size, Color color):
		m_size(size),
		m_data(m_size.Area(), color)
	{ }

	Bitmap::Bitmap(Size size, Color const* data):
		m_size(size),
		m_data(data, data+m_size.Area())
	{ }

	Bitmap::Bitmap(const Bitmap& other):
		m_size(other.m_size),
		m_data(other.m_data)
	{ }

	Bitmap::Bitmap(Bitmap&& other):
		m_size(other.m_size),
		m_data(std::move(other.m_data))
	{ }

	void Bitmap::Swap(Bitmap& other)
	{
		std::swap(m_size, other.m_size);
		std::swap(m_data, other.m_data);
	}

	Bitmap& Bitmap::operator=(Bitmap other)
	{
		Swap(other);
		return *this;
	}

	Size Bitmap::GetSize() const
	{
		return m_size;
	}

	bool Bitmap::IsEmpty() const
	{
		return m_size.Area() == 0;
	}

	void Bitmap::Blit(const Bitmap& src, Rectangle src_region, Point location)
	{
		Size src_size = src_region.Size();

		if (!Rectangle(m_size).Contains(Rectangle(location, src_size)))
		{
			throw std::out_of_range("Bitmap::Blit: region is out of range");
		}

		for (int y=location.y; y<location.y+src_size.height; y++)
		{
			std::memcpy
			(
				m_data.data()+(y*m_size.width+location.x),
				src.m_data.data()+((src_region.top+(y-location.y))*src.GetSize().width + src_region.left),
				src_size.width*sizeof(Color)
			);
		}
	}

	void Bitmap::Blit(const Bitmap& src, Point location)
	{
		Size src_size = src.GetSize();

		if (!Rectangle(m_size).Contains(Rectangle(location, src_size)))
		{
			throw std::out_of_range("Bitmap::Blit: region is out of range");
		}

		for (int y=location.y; y<location.y+src_size.height; y++)
		{
			std::memcpy
			(
				m_data.data()+(y*m_size.width+location.x),
				src.m_data.data()+((y-location.y)*src.m_size.width),
				src.m_size.width*sizeof(Color)
			);
		}
	}

	void Bitmap::BlitUnchecked(const Bitmap& src, Point location)
	{
		Size src_size = src.GetSize();

		int left = std::max<int>(-location.x, 0);
		int right = std::min<int>((int)src_size.width-1, (int)m_size.width-location.x-1);
		int top = std::max<int>(-location.y, 0);
		int bottom = std::min<int>((int)src_size.height-1, (int)m_size.height-location.y-1);

		if (left > right || top > bottom) return;

		size_t linesize = right-left+1;
		size_t dst_x = location.x+left;

		for (int y = top; y <= bottom; y++)
		{
			size_t dst_y = location.y + y;
			std::memcpy
			(
				m_data.data()+(dst_y*m_size.width+dst_x),
				src.m_data.data()+(y*src.m_size.width+left),
				linesize*sizeof(Color)
			);
		}
	}

	Bitmap Bitmap::Extract(Rectangle region)
	{
		if (!Rectangle(0, 0, m_size.width, m_size.height).Contains(region))
		{
			throw std::out_of_range("Bitmap::Extract: region is out of range");
		}

		Bitmap result(region.Size(), Color());
		for (int y=region.top; y<region.top+region.height; y++)
		{
			std::memcpy
			(
				result.m_data.data()+((y-region.top)*result.m_size.width),
				m_data.data()+(y*m_size.width+region.left),
				result.m_size.width*sizeof(Color)
			);
		}

		return result;
	}

	const Color& Bitmap::operator() (Point p) const
	{
		return m_data[p.y*m_size.width+p.x];
	}

	const Color& Bitmap::operator() (int x, int y) const
	{
		return m_data[y*m_size.width+x];
	}

	Color& Bitmap::operator() (Point p)
	{
		return m_data[p.y*m_size.width+p.x];
	}

	Color& Bitmap::operator() (int x, int y)
	{
		return m_data[y*m_size.width+x];
	}

	const Color* Bitmap::GetData() const
	{
		return m_data.data();
	}

	bool Bitmap::HasAlpha() const
	{
		for (const Color& pixel: m_data) if (pixel.a < 0xFF) return true;

		return false;
	}

	void Bitmap::MakeTransparent(Color color)
	{
		for (Color& pixel: m_data) if (pixel == color) pixel.a = 0;
	}
}
