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

namespace BearLibTerminal
{
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

	Slot::Slot()
	{ }

	Tile::Tile():
		placement(Placement::Normal),
		offset(0, 0),
		bounds(1, 1)
	{ }

	TileSlot::TileSlot()
	{ }

	AtlasTexture3::AtlasTexture3(Type type, Size initial_size):
		m_type(type),
		m_is_dirty(true)
	{
		// FIXME: check if GPU supports NPOTD, round up if necessary
		m_canvas = Bitmap(initial_size, Color());
		m_spaces.push_back(Rectangle(m_canvas.GetSize()));
	}

	AtlasTexture3::Type AtlasTexture3::GetType() const
	{
		return m_type;
	}

	static int RoundUpTo(int value, int to) // TODO: move to utils
	{
		int remainder = value % to;
		return remainder? value + (to-remainder): value;
	}

	std::shared_ptr<TileSlot> AtlasTexture3::Add(const Bitmap& bitmap, Rectangle region)
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

	void AtlasTexture3::Update(std::shared_ptr<TileSlot> slot, const Bitmap& bitmap)
	{
		if (slot->texture != this)
		{
			throw std::runtime_error("AtlasTexture3::Update(...): ownership mismatch");
		}

		m_texture.Update(slot->texture_region, bitmap);
		m_canvas.BlitUnchecked(bitmap, slot->texture_region.Location());
	}

	void AtlasTexture3::Remove(std::shared_ptr<TileSlot> slot)
	{
		auto i = std::find(m_slots.begin(), m_slots.end(), slot);
		if (i == m_slots.end())
		{
			throw std::runtime_error("AtlasTexture3::Remove(...): ownership mismatch");
		}

		// Give back used space
		m_spaces.push_back(Rectangle(slot->texture_region.Location(), slot->space_size));
		PostprocessSpaces();
	}

	bool AtlasTexture3::IsEmpty() const
	{
		return m_slots.empty();
	}

	void AtlasTexture3::Refresh()
	{
		if (m_is_dirty)
		{
			m_texture.Update(m_canvas);
			m_is_dirty = false;
		}
	}

	void AtlasTexture3::Bind()
	{
		m_texture.Bind();
	}

	void AtlasTexture3::Dispose()
	{
		m_slots.clear();
		m_spaces.clear();
		m_texture.Dispose();
		m_canvas = Bitmap();
	}

	bool AtlasTexture3::TryGrow()
	{
		// Expand to nearest greater POTD
		// FIXME: properly calculate texture size, current is not necessary a POTD
		// FIXME: unnecessary, if GPU doesn't support NPOTD, texture constructor will enlarge it to POTD
		Size old_size = m_canvas.GetSize(), new_size = old_size;
		(new_size.height <= new_size.width? new_size.height: new_size.width) *= 2;

		// FIXME: check if GPU supports textures of this size, return false if not
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

	void AtlasTexture3::PostprocessSpaces()
	{
		// FIXME: sort spaces
		// TODO: merge spaces
	}

	TexCoords AtlasTexture3::CalcTexCoords(Rectangle region)
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

	// ------------------------------------------------------------------------

	void TileSlot::BindTexture()
	{
		texture->Bind();
	}

	void TileSlot::Draw(const Leaf& leaf, int x, int y)
	{
		int right = x + texture_region.width;
		int bottom = y + texture_region.height;

		if (leaf.flags == 0) // TODO: coloring flags constants
		{
			// Single-colored version
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);

			// Top-left
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(x, y);

			// Bottom-left
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(x, bottom);

			// Bottom-right
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, y);
		}
		else
		{
			// Corner-colored version

			// Top-left
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(x, y);

			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(x, bottom);

			// Bottom-right
			glColor4ub(leaf.color[2].r, leaf.color[2].g, leaf.color[2].b, leaf.color[2].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, y);
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

		AtlasTexture3::Type type = AtlasTexture3::Type::Tile;
		if (size.width >= 64 || size.height >= 64)
		{
			float aspect = size.width / (float)size.height;
			const float aspect_treshold = 1.5f;
			if (aspect >= 1/aspect_treshold && aspect <= aspect_treshold)
			{
				// The image is too bulky to be considered tile
				type = AtlasTexture3::Type::Sprite;
			}
		}

		if (type == AtlasTexture3::Type::Sprite)
		{
			// Allocate whole texture for this image
			m_textures.emplace_back(type, size);
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
			auto i = std::find_if(m_textures.begin(), m_textures.end(), [&](const AtlasTexture3& j){return &j == slot->texture;});
			if (i == m_textures.end())
			{
				throw std::runtime_error("Atlas::Remove(...): ownership mismatch");
			}
			m_textures.erase(i);
		}
	}

	void Atlas::Refresh()
	{
		for (auto& i: m_textures) i.Refresh();
	}

	void Atlas::Dispose()
	{
		for (auto& i: m_textures) i.Dispose();
		m_textures.clear();
	}
}
