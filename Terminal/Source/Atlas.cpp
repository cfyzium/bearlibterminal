/*
* BearLibTerminal
* Copyright (C) 2013-2017 Cfyz
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

#include "Atlas.hpp"
#include "Stage.hpp"
#include "Log.hpp"
#include "OpenGL.hpp"
#include "Geometry.hpp"
#include "Utility.hpp"
#include <algorithm>
#include <fstream>

namespace BearLibTerminal
{
	Atlas g_atlas;

	static int RoundUpTo(int value, int to) // TODO: move to utils
	{
		int remainder = value % to;
		return remainder? value + (to-remainder): value;
	}

	bool IsPowerOfTwo(unsigned int x)
	{
		return x && !(x & (x - 1));
	}

	int RoundUpToPow2(int x)
	{
		if (x < 0) return 0;
		--x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x+1;
	}

	TexCoords::TexCoords():
		tu1(0),
		tv1(0),
		tu2(1),
		tv2(1)
	{ }

	TexCoords::TexCoords(float tu1, float tv1, float tu2, float tv2):
		tu1(tu1),
		tv1(tv1),
		tu2(tu2),
		tv2(tv2)
	{ }

	std::wostream& operator<<(std::wostream& s, const TileAlignment& value)
	{
		switch (value)
		{
		case TileAlignment::Unknown:
			s << L"unknown";
			break;
		case TileAlignment::Center:
			s << L"center";
			break;
		case TileAlignment::DeadCenter:
			s << L"dead-center";
			break;
		case TileAlignment::TopLeft:
			s << L"top-left";
			break;
		case TileAlignment::TopRight:
			s << L"top-right";
			break;
		case TileAlignment::BottomLeft:
			s << L"bottom-left";
			break;
		case TileAlignment::BottomRight:
			s << L"bottom-right";
			break;
		}

		return s;
	}

	std::wistream& operator>>(std::wistream& s, TileAlignment& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"center")
			value = TileAlignment::Center;
		else if (temp == L"dead-center")
			value = TileAlignment::DeadCenter;
		else if (temp == L"top-left")
			value = TileAlignment::TopLeft;
		else if (temp == L"top-right")
			value = TileAlignment::TopRight;
		else if (temp == L"bottom-left")
			value = TileAlignment::BottomLeft;
		else if (temp == L"bottom-right")
			value = TileAlignment::BottomRight;
		else
			s.setstate(s.failbit);

		return s;
	}



	TileInfo::TileInfo():
		tileset(nullptr),
		texture(nullptr),
		alignment(TileAlignment::Center),
		is_animated(false)
	{ }



	AtlasTexture::AtlasTexture(Size initial_size):
		m_canvas(initial_size, Color{})
	{
		m_spaces.emplace_back(initial_size);
	}

	AtlasTexture::AtlasTexture(std::shared_ptr<TileInfo> sprite)
	{
		Size size = sprite->bitmap.GetSize();
		if (!g_has_texture_npot)
		{
			size.width = RoundUpToPow2(size.width);
			size.height = RoundUpToPow2(size.height);
		}
		if (size.width > g_max_texture_size || size.height > g_max_texture_size)
		{
			throw std::runtime_error("Sprite requires a texture bigger than supported by the hardware");
		}

		m_canvas = Bitmap{size, Color{}};
		m_canvas.Blit(sprite->bitmap, {});

		// Update the tile info.
		sprite->texture = this;
		sprite->useful_space = Rectangle{size};
		sprite->total_space = Rectangle{size};
		sprite->texture_coords = CalcTexCoords(sprite->useful_space);
		m_tiles.push_back(sprite);
	}

	bool AtlasTexture::IsEmpty() const
	{
		return m_tiles.empty();
	}

	bool AtlasTexture::Add(std::shared_ptr<TileInfo> tile)
	{
		if (!tile)
			throw std::runtime_error("Empty reference passed to AtlasTexture::Add");

		if (m_tiles.size() == 1 && (m_tiles.front()->total_space.Area() / (float)m_canvas.GetSize().Area() > 0.25f))
		{
			// This is a sprite tile texture.
			return false;
		}

		// Round tile dimensions to multiple of 4 for more discrete space partitioning
		Bitmap& bitmap = tile->bitmap;
		Size bitmap_size = bitmap.GetSize();
		Size tile_size = bitmap_size + Size(2, 2);
		tile_size.width = RoundUpTo(tile_size.width, 4);
		tile_size.height = RoundUpTo(tile_size.height, 4);

		auto find_space = [&](Size size) -> decltype(m_spaces.end())
		{
			while(true)
			{
				for (auto i = m_spaces.begin(); i != m_spaces.end(); ++i)
				{
					if (i->width >= size.width && i->height >= size.height)
						return i;
				}

				if (!TryGrow())
					return m_spaces.end();
			}
		};

		// Find suitable free space.
		auto space = find_space(tile_size);

		if (space == m_spaces.end())
		{
			// Couldn't find a space to fit this tile even after enlarging to the limits.
			return false;
		}

		// Place the tile on the canvas.
		Point location = space->Location() + Point{1, 1};
		m_canvas.Blit(bitmap, location);

		// Expand borders for correct texture sampling in scaled/fullscreen mode.
		for (int x = 0; x < bitmap_size.width; x++)
		{
			m_canvas(location.x + x, location.y - 1) = m_canvas(location.x + x, location.y);
			m_canvas(location.x + x, location.y + bitmap_size.height) = m_canvas(location.x + x, location.y + bitmap_size.height - 1);
		}
		for (int y = -1; y < bitmap_size.height + 1; y++)
		{
			m_canvas(location.x - 1, location.y+y) = m_canvas(location.x, location.y + y);
			m_canvas(location.x + bitmap_size.width, location.y + y) = m_canvas(location.x + bitmap_size.width - 1, location.y + y);
		}

		// Mark for texture update.
		m_dirty_regions.emplace_back(space->Location(), tile_size);

		// Split used space
		int p3width = (space->width - tile_size.width);
		int p3height = (space->height - tile_size.height);
		int p1 = tile_size.width * p3height;
		int p2 = tile_size.height * p3width;
		int p3 = p3width * p3height;

		Rectangle cut_space;
		if (p2 + p3 > p1 + p3)
		{
			// Split vertically
			cut_space = Rectangle(space->left + tile_size.width, space->top, p3width, space->height);
			space->width = tile_size.width;
			space->height -= tile_size.height;
			space->top += tile_size.height;
		}
		else
		{
			// Split horisontally
			cut_space = Rectangle(space->left, space->top + tile_size.height, space->width, p3height);
			space->height = tile_size.height;
			space->width -= tile_size.width;
			space->left += tile_size.width;
		}

		if (cut_space.Area() > 0)
			m_spaces.push_back(cut_space);

		if (space->Area() == 0)
			m_spaces.erase(space);

		// Postprocess a bit.
		m_spaces.sort([](Rectangle& lhs, Rectangle& rhs){return lhs.Area() < rhs.Area();});

		// Update the tile info.
		tile->texture = this;//shared_from_this();
		tile->useful_space = Rectangle{location, bitmap_size};
		tile->total_space = Rectangle{location - Point{1, 1}, tile_size};
		tile->texture_coords = CalcTexCoords(tile->useful_space);

		// Save reference.
		m_tiles.push_back(tile);

		return true;
	}

	bool AtlasTexture::TryGrow()
	{
		// Expand to nearest greater POTD
		Size old_size = m_canvas.GetSize();
		Size new_size = old_size;
		(new_size.height <= new_size.width? new_size.height: new_size.width) *= 2;
		if (new_size.width > g_max_texture_size || new_size.height > g_max_texture_size)
		{
			return false;
		}

		Bitmap new_canvas(new_size, Color{});
		new_canvas.Blit(m_canvas, Point{});
		m_canvas = std::move(new_canvas);

		// Add space
		if (new_size.width > old_size.width)
		{
			// Space added at the left
			m_spaces.push_back(Rectangle{old_size.width, 0, new_size.width-old_size.width, new_size.height});
		}
		else
		{
			// Space added at the bottom
			m_spaces.push_back(Rectangle{0, old_size.height, new_size.width, new_size.height-old_size.height});
		}

		LOG(Trace, "grow " << old_size << " -> " << m_canvas.GetSize());

		// Texture size has been changed, must recalculate texure coords for slots
		for (auto& i: m_tiles)
			i->texture_coords = CalcTexCoords(i->useful_space);

		return true;
	}

	TexCoords AtlasTexture::CalcTexCoords(const Rectangle& region)
	{
		float x1 = region.left;
		float x2 = region.left + region.width;
		float y1 = region.top;
		float y2 = region.top + region.height;

		Size size = m_canvas.GetSize();
		return TexCoords
		{
			x1 / size.width,
			y1 / size.height,
			x2 / size.width,
			y2 / size.height
		};
	}

	void AtlasTexture::Remove(std::shared_ptr<TileInfo> tile, bool copy_bitmap_back)
	{
		if (!tile)
			throw std::runtime_error("Empty reference passed to AtlasTexture::Remove");

		if (tile->texture != this)
			throw std::runtime_error("AtlasTexture::Remove: tile does not belong to this texture");

		if (copy_bitmap_back)
			tile->bitmap = m_canvas.Extract(tile->useful_space);
		tile->texture = nullptr;
		tile->total_space = tile->useful_space = Rectangle{};
		m_tiles.remove(tile);
		m_spaces.push_back(tile->total_space);
	}

	void AtlasTexture::Bind()
	{
		if (m_texture.GetSize() != m_canvas.GetSize())
		{
			m_texture.Update(m_canvas);
			m_dirty_regions.clear();
		}
		else if (!m_dirty_regions.empty())
		{
			m_dirty_regions.sort([](Rectangle& lhs, Rectangle& rhs){return lhs.top < rhs.top;});
			std::list<Rectangle> merged{m_dirty_regions.front()};

			for (auto& r: m_dirty_regions)
			{
				auto& s = merged.back();

				if (r.top + r.height <= s.top + s.height)
					continue; // Ignore.
				else if (r.top > s.top + s.height)
					merged.emplace_back(0, r.top, 0, r.height); // Start a new span.
				else
					merged.back().height = r.top + r.height - s.top; // Expand.
			}

			for (auto& r: merged)
			{
				auto area = Rectangle{0, r.top, m_canvas.GetSize().width, r.height};
				m_texture.Update(area, m_canvas.Extract(area));
			}

			m_dirty_regions.clear();
		}

		m_texture.Bind();
	}

	void AtlasTexture::Defragment()
	{
		// FIXME: NYI

		m_spaces.sort([](Rectangle& lhs, Rectangle& rhs){return lhs.Area() < rhs.Area();});
	}

	void AtlasTexture::ApplyTextureFilter()
	{
		m_texture.ApplyTextureFilter();
	}



	void Atlas::Add(std::shared_ptr<TileInfo> tile)
	{
		if (!tile)
			throw std::runtime_error("Empty reference passed to Atlas::Add");

		if (tile->bitmap.GetSize().Area() >= 100*100) // Arbitrary chosen size.
		{
			m_textures.push_back(std::make_shared<AtlasTexture>(tile));
		}
		else
		{
			for (auto& texture: m_textures)
			{
				if (texture->Add(tile))
					return;
			}

			auto texture = std::make_shared<AtlasTexture>(Size{256, 256});
			if (!texture->Add(tile))
				throw std::runtime_error("Failed to add a tile to a newly constructed texture");
			m_textures.push_back(texture);
		}
	}

	void Atlas::Remove(std::shared_ptr<TileInfo> tile)
	{
		if (!tile || !tile->texture)
			throw std::runtime_error("Empty reference passed to Atlas::Remove");

		tile->texture->Remove(tile);
	}

	void Atlas::Defragment()
	{
		for (auto& texture: m_textures)
			texture->Defragment();
	}

	void Atlas::CleanUp()
	{
		m_textures.remove_if([](std::shared_ptr<AtlasTexture>& item){return item->IsEmpty();});
	}

	void Atlas::Clear()
	{
		m_textures.clear();
	}

	void Atlas::ApplyTextureFilter()
	{
		for (auto texture: m_textures)
			texture->ApplyTextureFilter();
	}
}
