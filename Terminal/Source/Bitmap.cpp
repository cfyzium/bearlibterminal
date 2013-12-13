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
#include "Log.hpp"
#include <stdexcept>
#include <cstring>
#include <cmath>

namespace BearLibTerminal
{
	std::wostream& operator<<(std::wostream& s, const ResizeFilter& value)
	{
		switch (value)
		{
		case ResizeFilter::Nearest:
			s << "nearest";
			break;
		case ResizeFilter::Bilinear:
			s << "bilinear";
			break;
		case ResizeFilter::Bicubic:
			s << "bicubic";
			break;
		default:
			s << "n/a";
			break;
		}

		return s;
	}

	std::wistream& operator>>(std::wistream& s, ResizeFilter& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"bilinear")
		{
			value = ResizeFilter::Bilinear;
		}
		else if (temp == L"bicubic")
		{
			value = ResizeFilter::Bicubic;
		}
		else if (temp == L"nearest")
		{
			value = ResizeFilter::Nearest;
		}
		else
		{
			s.setstate(std::wistream::badbit);
		}

		return s;
	}

	std::wostream& operator<<(std::wostream& s, const ResizeMode& value)
	{
		switch (value)
		{
		case ResizeMode::Stretch:
			s << "stretch";
			break;
		case ResizeMode::Fit:
			s << "fit";
			break;
		case ResizeMode::Crop:
			s << "crop";
			break;
		default:
			s << "n/a";
			break;
		}

		return s;
	}

	std::wistream& operator>>(std::wistream& s, ResizeMode& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"stretch")
		{
			value = ResizeMode::Stretch;
		}
		else if (temp == L"fit")
		{
			value = ResizeMode::Fit;
		}
		else if (temp == L"crop")
		{
			value = ResizeMode::Crop;
		}
		else
		{
			s.setstate(std::wistream::badbit);
		}

		return s;
	}

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

	Bitmap ResizeNearest(Bitmap& original, Size size)
	{
		Bitmap result(size, Color());

		Size original_size = original.GetSize();
		float hfactor = size.width / (float)original_size.width;
		float vfactor = size.height / (float)original_size.height;

		for (int y=0; y<size.height; y++)
		{
			int original_y = (int)std::floor(y / vfactor);
			for (int x=0; x<size.width; x++)
			{
				int original_x = (int)std::floor(x / hfactor);
				result(x, y) = original(original_x, original_y);
			}
		}

		return result;
	}

	Bitmap ResizeBilinear(Bitmap& original, Size size)
	{
		Bitmap result(size, Color());

		auto filter = [&](int x, int y, float ox, float oy)
		{
			int x1 = std::floor(ox);
			int y1 = std::floor(oy);

			float dx1 = ox-x1, dx2 = (x1+1)-ox;
			float dy1 = oy-y1, dy2 = (y1+1)-oy;

			float w1 = dx2*dy2;
			float w2 = dx1*dy2;
			float w3 = dx2*dy1;
			float w4 = dx1*dy1;

			Color q11 = original(x1+0, y1+0);
			Color q12 = original(x1+0, y1+1);
			Color q21 = original(x1+1, y1+0);
			Color q22 = original(x1+1, y1+1);

			int r = q11.r*w1 + q21.r*w2 + q12.r*w3 + q22.r*w4;
			int g = q11.g*w1 + q21.g*w2 + q12.g*w3 + q22.g*w4;
			int b = q11.b*w1 + q21.b*w2 + q12.b*w3 + q22.b*w4;
			int a = q11.a*w1 + q21.a*w2 + q12.a*w3 + q22.a*w4;

			return Color(a, r, g, b);
		};

		Size original_size = original.GetSize();
		float hfactor = size.width / (float)original_size.width;
		float vfactor = size.height / (float)original_size.height;

		for (int y=0; y<size.height; y++)
		{
			float original_y = y / vfactor;
			for (int x=0; x<size.width; x++)
			{
				float original_x = x / hfactor;
				result(x, y) = filter(x, y, original_x, original_y);
			}
		}

		return result;
	}

	Bitmap ResizeBicubic(Bitmap& original, Size size)
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

		Size original_size = original.GetSize();
		double xFactor = (double)original_size.width / size.width;
		double yFactor = (double)original_size.height / size.height;

		// coordinates of source points and coefficients
		double ox, oy, dx, dy, k1, k2;
		int ox1, oy1, ox2, oy2;

		// destination pixel values
		double p_g[4];

		int ymax = original_size.height-1;
		int xmax = original_size.width-1;

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

					uint8_t* linek = (uint8_t*)&original(0, oy2);

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

	Bitmap Bitmap::Resize(Size size, ResizeFilter filter, ResizeMode mode)
	{
		Size intermediate_size = size;

		if (mode == ResizeMode::Fit)
		{
			// Less or equal size
			float factor = std::min(size.width / (float)m_size.width, size.height / (float)m_size.height);
			intermediate_size.width = m_size.width * factor;
			intermediate_size.height = m_size.height * factor;
		}
		else if (mode == ResizeMode::Crop)
		{
			// Greater or equal size
			float factor = std::max(size.width / (float)m_size.width, size.height / (float)m_size.height);
			intermediate_size.width = m_size.width * factor;
			intermediate_size.height = m_size.height * factor;
		}
		else if (mode != ResizeMode::Stretch)
		{
			throw std::runtime_error("Bitmap::Resize: unknown resize mode");
		}

		Bitmap intermediate;

		if (filter == ResizeFilter::Nearest)
		{
			intermediate = ResizeNearest(*this, intermediate_size);
		}
		else if (filter == ResizeFilter::Bilinear)
		{
			intermediate =  ResizeBilinear(*this, intermediate_size);
		}
		else if (filter == ResizeFilter::Bicubic)
		{
			intermediate = ResizeBicubic(*this, intermediate_size);
		}
		else
		{
			throw std::runtime_error("Bitmap::Resize: unknown resize filter");
		}

		if (intermediate_size == size)
		{
			// Stretch should fall here
			return intermediate;
		}
		else if (mode == ResizeMode::Fit)
		{
			Bitmap result(size, Color(255, 0, 0, 0));
			int left = (size.width - intermediate_size.width)/2;
			int top = (size.height - intermediate_size.height)/2;
			result.Blit(intermediate, Point(left, top));
			return result;
		}
		else if (mode == ResizeMode::Crop)
		{
			Bitmap result(size, Color());
			int left = (intermediate_size.width-size.width)/2;
			int top = (intermediate_size.height-size.height)/2;
			result.Blit(intermediate, Rectangle(Point(left, top), size), Point());
			return result;
		}
		else
		{
			throw std::runtime_error("Bitmap::Resize: internal logic error");
		}
	}
}
