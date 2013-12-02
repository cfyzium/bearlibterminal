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

	Bitmap Bitmap::Resize(Size size)
	{
		auto bicubic_kernel = [](double x) -> double
		{
			if (x > 2.0) return 0.0;

			double a, b, c, d;
			double xm1 = x - 1.0;
			double xp1 = x + 1.0;
			double xp2 = x + 2.0;

			a = (xp2 <= 0.0)? 0.0: xp2 * xp2 * xp2;
			b = (xp1 <= 0.0)? 0.0: xp1 * xp1 * xp1;
			c = (x   <= 0.0)? 0.0: x * x * x;
			d = (xm1 <= 0.0)? 0.0: xm1 * xm1 * xm1;

			return (0.16666666666666666667 * (a - (4.0 * b) + (6.0 * c) - (4.0 * d)));
		};

		Bitmap result(size, Color());

		double xFactor = (double)m_size.width / size.width;
		double yFactor = (double)m_size.height / size.height;

		// coordinates of source points and coefficients
		double ox, oy, dx, dy, k1, k2;
		int ox1, oy1, ox2, oy2;

		// destination pixel values
		double p_g[4];

		int ymax = m_size.height-1;
		int xmax = m_size.width-1;

		for (int y=0; y<size.height; y++)
		{
			// Y coordinates
			oy  = (double)y * yFactor - 0.5;
			oy1 = (int)oy;
			dy  = oy - (double) oy1;

			uint8_t* lined = (uint8_t*)&result(0, y);

			for (int x=0; x<size.width; x++)
			{
				// X coordinates
				ox  = (double)x * xFactor - 0.5f;
				ox1 = (int)ox;
				dx  = ox - (double) ox1;

				// initial pixel value
				memset(p_g, 0, sizeof(p_g));

				for (int n=-1; n<3; n++)
				{
					// get Y coefficient
					k1 = bicubic_kernel(dy - (double)n);

					oy2 = oy1 + n;
					if (oy2 < 0) oy2 = 0;
					if (oy2 > ymax) oy2 = ymax;

					uint8_t* linek = (uint8_t*)&((*this)(0, oy2));

					for (int m=-1; m<3; m++)
					{
						// get X coefficient
						k2 = k1 * bicubic_kernel((double)m - dx);

						ox2 = ox1 + m;
						if (ox2 < 0) ox2 = 0;
						if (ox2 > xmax) ox2 = xmax;

						for (int d = 0; d<4; d++)
						{
							p_g[d] += k2 * linek[ox2*4 + d];
						}
					}
				}

				for (int d=0; d<4; d++)
				{
					lined[x*4 + d] = (uint8_t)p_g[d];
				}
			}
		}

		return result;
	}
}
