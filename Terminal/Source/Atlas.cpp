/*
 * Atlas.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#include "Atlas.hpp"
#include "Stage.hpp"
#include "Log.hpp"
#include "OpenGL.hpp"
#include "Geometry.hpp"
#include <algorithm>
#include <fstream>

namespace BearLibTerminal
{
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

	Leaf::Leaf():
		dx(0),
		dy(0),
		code(0),
		flags(0),
		reserved(0)
	{ }

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

	Slot::Slot():
		texture_id(0)
	{ }

	// ------------------------------------------------------------------------

	Tile::Tile():
		alignment(Alignment::Unknown),
		offset(0, 0),
		bounds(1, 1)
	{ }

	std::wostream& operator<<(std::wostream& s, const Tile::Alignment& value)
	{
		switch (value)
		{
		case Tile::Alignment::Unknown:
			s << L"unknown";
			break;
		case Tile::Alignment::Center:
			s << L"center";
			break;
		case Tile::Alignment::TopLeft:
			s << L"top-left";
			break;
		case Tile::Alignment::TopRight:
			s << L"top-right";
			break;
		case Tile::Alignment::BottomLeft:
			s << L"bottom-left";
			break;
		case Tile::Alignment::BottomRight:
			s << L"bottom-right";
			break;
		}

		return s;
	}

	std::wistream& operator>>(std::wistream& s, Tile::Alignment& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"center") value = Tile::Alignment::Center;
		else if (temp == L"top-left") value = Tile::Alignment::TopLeft;
		else if (temp == L"top-right") value = Tile::Alignment::TopRight;
		else if (temp == L"bottom-left") value = Tile::Alignment::BottomLeft;
		else if (temp == L"bottom-right") value = Tile::Alignment::BottomRight;
		else s.setstate(s.failbit);

		return s;
	}

	// ------------------------------------------------------------------------

	AtlasTexture::AtlasTexture(Type type, Size initial_size):
		m_type(type),
		m_is_dirty(true)
	{
		if (!g_has_texture_npot && (!IsPowerOfTwo(initial_size.width) || !IsPowerOfTwo(initial_size.height)))
		{
			Size new_size(RoundUpToPow2(initial_size.width), RoundUpToPow2(initial_size.height));
			LOG(Trace, "GPU does not support NPOTD textures, enlarging " << initial_size << " -> " << new_size);
			initial_size = new_size;
		}
		m_canvas = Bitmap(initial_size, Color());
		m_spaces.push_back(Rectangle(m_canvas.GetSize()));
	}

	AtlasTexture::Type AtlasTexture::GetType() const
	{
		return m_type;
	}

	std::shared_ptr<TileSlot> AtlasTexture::Add(const Bitmap& bitmap, Rectangle region)
	{
		// Round requested rectangle dimensions to multiple of 4 for more discrete space partitioning
		Size tile_size = region.Size();
		tile_size.width = RoundUpTo(tile_size.width, 4);
		tile_size.height = RoundUpTo(tile_size.height, 4);

		// Find suitable free space
		auto find_acceptable_space = [=](Size size) -> decltype(m_spaces.begin())
		{
			for (auto i = m_spaces.begin(); i != m_spaces.end(); i++)
			{
				if (i->width >= size.width && i->height >= size.height)
				{
					return i;
				}
			}

			return m_spaces.end();
		};

		auto acceptable_space = m_spaces.end();
		while ((acceptable_space = find_acceptable_space(tile_size)) == m_spaces.end())
		{
			if (!TryGrow()) break;
		}

		if (acceptable_space == m_spaces.end())
		{
			// Couldn't find space to fit this even after enlarging to the limits
			return std::shared_ptr<TileSlot>();
		}

		// Place
		Point tile_location = acceptable_space->Location();
		m_canvas.Blit(bitmap, region, tile_location);
		m_is_dirty = true;

		// Split used space
		int p3width = (acceptable_space->width - tile_size.width);
		int p3height = (acceptable_space->height - tile_size.height);
		int p1 = tile_size.width * p3height;
		int p2 = tile_size.height * p3width;
		int p3 = p3width * p3height;
		Rectangle cut_space;

		if (p2 + p3 > p1 + p3)
		{
			// Split vertically
			cut_space = Rectangle
			(
				acceptable_space->left + tile_size.width,
				acceptable_space->top,
				p3width,
				acceptable_space->height
			);

			acceptable_space->width = tile_size.width;
			acceptable_space->height -= tile_size.height;
			acceptable_space->top += tile_size.height;
		}
		else
		{
			// Split horisontally
			cut_space = Rectangle
			(
				acceptable_space->left,
				acceptable_space->top + tile_size.height,
				acceptable_space->width,
				p3height
			);

			acceptable_space->height = tile_size.height;
			acceptable_space->width -= tile_size.width;
			acceptable_space->left += tile_size.width;
		}

		if (cut_space.Area() > 0)
		{
			m_spaces.push_back(cut_space);
		}

		if (acceptable_space->Area() == 0)
		{
			m_spaces.erase(acceptable_space);
		}

		PostprocessSpaces();

		auto result = std::make_shared<TileSlot>();
		result->space_size = tile_size;
		result->texture = this;
		result->texture_id = (uint64_t)this;
		result->texture_region = Rectangle(tile_location, region.Size());
		result->texture_coords = CalcTexCoords(result->texture_region);
		m_slots.push_back(result);
		return result;
	}

	void AtlasTexture::Update(std::shared_ptr<TileSlot> slot, const Bitmap& bitmap)
	{
		if (slot->texture != this)
		{
			throw std::runtime_error("AtlasTexture3::Update(...): ownership mismatch");
		}

		m_texture.Update(slot->texture_region, bitmap);
		m_canvas.BlitUnchecked(bitmap, slot->texture_region.Location());
	}

	void AtlasTexture::Remove(std::shared_ptr<TileSlot> slot)
	{
		auto i = std::find(m_slots.begin(), m_slots.end(), slot);
		if (i == m_slots.end())
		{
			throw std::runtime_error("AtlasTexture3::Remove(...): ownership mismatch");
		}

		// Remove slot
		m_slots.erase(i);

		// Give back used space
		m_spaces.push_back(Rectangle(slot->texture_region.Location(), slot->space_size));

		//*
		// DEBUG: clear freed area with some color
		Rectangle area = Rectangle(slot->texture_region.Location(), slot->space_size);
		for (int y=area.top; y<area.top+area.height; y++)
		{
			for (int x=area.left; x<area.left+area.width; x++)
			{
				m_canvas(x, y) = Color(255, 0, 0, 255);
			}
		}
		m_is_dirty = true;
		//*/

		PostprocessSpaces();
	}

	bool AtlasTexture::IsEmpty() const
	{
		return m_slots.empty();
	}

	void AtlasTexture::Refresh()
	{
		if (m_is_dirty)
		{
			m_texture.Update(m_canvas);
			m_is_dirty = false;
		}

		Bind();
	}

	void AtlasTexture::Bind()
	{
		m_texture.Bind();
	}

	void AtlasTexture::Dispose()
	{
		m_slots.clear();
		m_spaces.clear();
		m_texture.Dispose();
		m_canvas = Bitmap();
	}

	bool AtlasTexture::TryGrow()
	{
		// Expand to nearest greater POTD
		Size old_size = m_canvas.GetSize(), new_size = old_size;
		(new_size.height <= new_size.width? new_size.height: new_size.width) *= 2;
		if (new_size.width > g_max_texture_size || new_size.height > g_max_texture_size)
		{
			return false;
		}

		Bitmap new_canvas(new_size, Color());
		new_canvas.Blit(m_canvas, Point());
		m_canvas = std::move(new_canvas);

		// Add space
		if (new_size.width > old_size.width)
		{
			// Space added at the left
			m_spaces.push_back(Rectangle(old_size.width, 0, new_size.width-old_size.width, new_size.height));
		}
		else
		{
			// Space added at the bottom
			m_spaces.push_back(Rectangle(0, old_size.height, new_size.width, new_size.height-old_size.height));
		}

		// Texture size has been changed, must recalculate texure coords for slots
		for (auto& i: m_slots)
		{
			i->texture_coords = CalcTexCoords(i->texture_region);
		}

		return true;
	}

	void AtlasTexture::PostprocessSpaces()
	{
		// FIXME: sort spaces
		// TODO: merge spaces
	}

	TexCoords AtlasTexture::CalcTexCoords(Rectangle region)
	{
		float x1 = region.left;
		float x2 = region.left + region.width;
		float y1 = region.top;
		float y2 = region.top + region.height;

		Size size = m_canvas.GetSize();
		return TexCoords
		(
			x1 / size.width,
			y1 / size.height,
			x2 / size.width,
			y2 / size.height
		);
	}

	Bitmap AtlasTexture::GetCanvasMap()
	{
		Bitmap result = m_canvas;
		Color cs = Color(255, 255, 0, 0);
		Color ct = Color(255, 0, 255, 0);

		for (auto space: m_spaces)
		{
			int x1 = space.left;
			int x2 = x1+space.width;
			int y1 = space.top;
			int y2 = y1+space.height;

			for (int x=x1; x<x2; x++)
			{
				result(x, y1) = cs;
				result(x, y2-1) = cs;
			}

			for (int y=y1; y<y2; y++)
			{
				result(x1, y) = cs;
				result(x2-1, y) = cs;
			}
		}

		for (auto slot: m_slots)
		{
			Rectangle region = slot->texture_region;
			int x1 = region.left;
			int x2 = x1+region.width;
			int y1 = region.top;
			int y2 = y1+region.height;

			for (int x=x1; x<x2; x++)
			{
				result(x, y1) = ct;
				result(x, y2-1) = ct;
			}

			for (int y=y1; y<y2; y++)
			{
				result(x1, y) = ct;
				result(x2-1, y) = ct;
			}
		}

		return result;
	}

	// ------------------------------------------------------------------------

	TileSlot::TileSlot():
		texture(nullptr)
	{ }

	void TileSlot::BindTexture()
	{
		texture->Bind();
	}

	void TileSlot::Draw(const Leaf& leaf, int x, int y, int w2, int h2)
	{
		// TODO: Think up of some optimization?
		// There are a lot of calculations done.

		int left, top;

		w2 *= bounds.width;
		h2 *= bounds.height;

		switch (alignment)
		{
		case Tile::Alignment::Center:
			left = x + offset.x + w2 + leaf.dx;
			top = y + offset.y + h2 + leaf.dy;
			break;
		case Tile::Alignment::TopLeft:
			left = x + offset.x + leaf.dx;
			top = y + offset.y + leaf.dy;
			break;
		case Tile::Alignment::TopRight:
			left = x + offset.x + 2*w2 - texture_region.width + leaf.dx;
			top = y + offset.y + leaf.dy;
			break;
		case Tile::Alignment::BottomLeft:
			left = x + offset.x + leaf.dx;
			top = y + offset.y + 2*h2 - texture_region.height + leaf.dy;
			break;
		case Tile::Alignment::BottomRight:
			left = x + offset.x + 2*w2 - texture_region.width + leaf.dx;
			top = y + offset.y + 2*h2 - texture_region.height + leaf.dy;
			break;
		default:
			// Same as TopLeft
			left = x + offset.x + leaf.dx;
			top = y + offset.y + leaf.dy;
			break;
		}

		int right = left + texture_region.width;
		int bottom = top + texture_region.height;

		if (leaf.flags & Leaf::CornerColored)
		{
			/*
			// Single-quad version (incorrect interpolation)
			// Top-left
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(left, top);

			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(left, bottom);

			// Bottom-right
			glColor4ub(leaf.color[2].r, leaf.color[2].g, leaf.color[2].b, leaf.color[2].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, top);
			/*/

			// 2-quad version
			// Center color
			int cr = (leaf.color[0].r + leaf.color[1].r + leaf.color[2].r + leaf.color[3].r)/4;
			int cg = (leaf.color[0].g + leaf.color[1].g + leaf.color[2].g + leaf.color[3].g)/4;
			int cb = (leaf.color[0].b + leaf.color[1].b + leaf.color[2].b + leaf.color[3].b)/4;
			int ca = (leaf.color[0].a + leaf.color[1].a + leaf.color[2].a + leaf.color[3].a)/4;
			// Center texture coords
			float cu = (texture_coords.tu1 + texture_coords.tu2)/2.0f;
			float cv = (texture_coords.tv1 + texture_coords.tv2)/2.0f;
			// Center coordinate
			float cx = (left + right)/2.0f;
			float cy = (top + bottom)/2.0f;

			// First quad
			// Top-left
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(left, top);
			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(left, bottom);
			// Center
			glColor4ub(cr, cg, cb, ca);
			glTexCoord2f(cu, cv);
			glVertex2i(cx, cy);
			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, top);

			// Second squad
			// Bottom-right
			glColor4ub(leaf.color[2].r, leaf.color[2].g, leaf.color[2].b, leaf.color[2].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);
			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, top);
			// Center
			glColor4ub(cr, cg, cb, ca);
			glTexCoord2f(cu, cv);
			glVertex2i(cx, cy);
			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(left, bottom);
			//*/
		}
		else
		{
			// Single-colored version
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);

			// Top-left
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(left, top);

			// Bottom-left
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(left, bottom);

			// Bottom-right
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, top);
		}
	}

	void TileSlot::Update(const Bitmap& bitmap)
	{
		texture->Update(shared_from_this(), bitmap);
	}

	// ------------------------------------------------------------------------

	std::shared_ptr<TileSlot> Atlas::Add(const Bitmap& bitmap, Rectangle region)
	{
		Size size = region.Size();

		AtlasTexture::Type type = AtlasTexture::Type::Tile;
		if ((size.width > 64 || size.height > 64) && size.Area() > 64*64) // TODO: optional threshold parameter
		{
			LOG(Debug, "New slot is a sprite because of its size: " << size);
			type = AtlasTexture::Type::Sprite;
		}

		if (type == AtlasTexture::Type::Sprite)
		{
			// Allocate whole texture for this image
			m_textures.emplace_back(type, Size(RoundUpTo(size.width, 4), RoundUpTo(size.height,4)));
			return m_textures.back().Add(bitmap, region);
		}
		else
		{
			// Try to fit into one of the existing tile textures
			std::shared_ptr<TileSlot> result;
			for (auto& i: m_textures)
			{
				if (i.GetType() != type) continue;
				result = i.Add(bitmap, region);
				if (result) break;
			}

			if (!result)
			{
				m_textures.emplace_back(type, Size(256, 256));
				result = m_textures.back().Add(bitmap, region);
			}

			return result;
		}
	}

	void Atlas::Remove(std::shared_ptr<TileSlot> slot)
	{
		slot->texture->Remove(slot);
		if (slot->texture->IsEmpty())
		{
			// Remove texture
			auto i = std::find_if(m_textures.begin(), m_textures.end(), [&](const AtlasTexture& j){return &j == slot->texture;});
			if (i == m_textures.end())
			{
				throw std::runtime_error("Atlas::Remove(...): ownership mismatch");
			}
			m_textures.erase(i);
		}
	}

	void Atlas::Refresh()
	{
		/*
		for (auto& i: m_textures) i.Refresh();
		/*/
		int j = 0;
		for (auto& i: m_textures)
		{
			i.Refresh();
			glEnable(GL_TEXTURE_2D);
			int x = 640-(j+1)*74;
			int y = 400-74;
			glBegin(GL_QUADS);
			// Single-colored version
			glColor4ub(255, 255, 255, 255);
			// Top-left
			glTexCoord2f(0.0f, 0.0f);
			glVertex2i(x, y);
			// Bottom-left
			glTexCoord2f(0.0f, 0.2f);
			glVertex2i(x, y+64);
			// Bottom-right
			glTexCoord2f(0.2f, 0.2f);
			glVertex2i(x+64, y+64);
			// Top-right
			glTexCoord2f(0.2f, 0.0f);
			glVertex2i(x+64, y);
			glEnd();
			j++;
		}
		//*/
	}

	void Atlas::Dispose()
	{
		for (auto& i: m_textures) i.Dispose();
		m_textures.clear();
	}

	// Prototype
	void SaveBMP(const Bitmap& bitmap, std::ostream& stream);

	void Atlas::Dump()
	{
		int counter = 0;

		for (auto& texture: m_textures)
		{
			std::ostringstream ss;
			ss << "terminal.atlas." << counter++ << ".bmp";
			std::ofstream file(ss.str(), std::ios_base::out|std::ios_base::binary);
			SaveBMP(texture.GetCanvasMap(), file);
		}
	}
}
