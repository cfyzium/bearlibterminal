/*
 * DynamicTileset.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Cfyz
 */

#include "DynamicTileset.hpp"
#include "Log.hpp"
#include "Bitmap.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"

namespace BearLibTerminal
{
	DynamicTileset::DynamicTileset(TileContainer& container, OptionGroup& group):
		StronglyTypedReloadableTileset(container)
	{
		if (!group.attributes.count(L"size"))
		{
			throw std::runtime_error("DynamicTileset: 'size' attribute is missing");
		}

		if (!try_parse<Size>(group.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("DynamicTileset: failed to parse 'size' attribute");
		}
	}

	void DynamicTileset::Remove() // TODO: move to base class?
	{
		for (auto i: m_tiles)
		{
			if (m_container.slots.count(i.first) && (void*)m_container.slots[i.first].get() == (void*)i.second.get())
			{
				m_container.slots.erase(i.first);
			}
		}
	}

	bool DynamicTileset::Save()
	{
		int w2 = m_tile_size.width / 2;
		int h2 = m_tile_size.height / 2;

		// Add Unicode replacement character glyph
		uint16_t code = 0xFFFD;
		Bitmap canvas(m_tile_size, Color());
		for (int x=1; x<m_tile_size.width-1; x++)
		{
			canvas(x, 1) = Color(255, 255, 255, 255);
			canvas(x, m_tile_size.height-2) = Color(255, 255, 255, 255);
		}
		for (int y=1; y<m_tile_size.height-1; y++)
		{
			canvas(1, y) = Color(255, 255, 255, 255);
			canvas(m_tile_size.width-2, y) = Color(255, 255, 255, 255);
		}
		auto tile_slot = m_container.atlas.Add(canvas, Rectangle(m_tile_size));
		tile_slot->offset = Point(-w2, -h2);
		tile_slot->placement = TileSlot::Placement::Centered;
		m_tiles[code] = tile_slot;
		m_container.slots[code] = tile_slot;
		LOG(Info, L"Added Unicode replacement character tile (" << m_tile_size << L")");
	}

	void DynamicTileset::Reload(DynamicTileset&& tileset)
	{
		throw std::runtime_error("DynamicTileset is not reloadable");
	}

	Size DynamicTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}

	bool DynamicTileset::Provides(uint16_t code)
	{
		if (code == 0xFFFD) return true;

		// TODO: Box Drawing and Block Elements

		return false;
	}

	void DynamicTileset::Prepare(uint16_t code)
	{
		if (!m_tiles.count(code))
		{
			throw std::runtime_error("DynamicTileset::Prepare: request for a tile which is not provided by this tileset");
		}

		m_container.slots[code] = std::dynamic_pointer_cast<Slot>(m_tiles[code]);
	}
}
