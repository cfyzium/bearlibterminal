/*
 * BitmapTileset.cpp
 *
 *  Created on: Oct 17, 2013
 *      Author: Cfyz
 */

#include "BitmapTileset.hpp"
#include "LoadBitmap.hpp"
#include "Encoding.hpp"
#include "Geometry.hpp"
#include "Utility.hpp"
#include <stdexcept>
#include <fstream>
#include <cmath>

namespace BearLibTerminal
{
	BitmapTileset::BitmapTileset(TileContainer& container, OptionGroup& group):
		StronglyTypedReloadableTileset(container),
		m_base_code(0)
	{
		if (group.name != L"font" && !try_parse<uint16_t>(group.name, m_base_code))
		{
			throw std::runtime_error("BitmapTileset: failed to parse base code");
		}

		if (!group.attributes.count(L"size"))
		{
			throw std::runtime_error("BitmapTileset: 'size' attribute is missing");
		}

		if (!try_parse<Size>(group.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'size' attribute");
		}

		// TODO: codepage attribute

		std::string filename = UTF8->Convert(group.attributes[L"name"]);
		std::ifstream file(filename.c_str(), std::ios_base::binary); // FIXME: Resources manager
		m_cache = LoadBitmap(file);

		// TODO: check dimensions
	}

	bool BitmapTileset::Save()
	{
		if (!m_cache.IsEmpty())
		{
			Size image_size = m_cache.GetSize();
			int columns = (int)std::floor(image_size.width / (float)m_tile_size.width);
			int rows = (int)std::floor(image_size.height / (float)m_tile_size.height);

			for (int y=0; y<rows; y++)
			{
				for (int x=0; x<columns; x++)
				{
					int i = y*columns + x;
					uint16_t code = m_base_code + i;
					Rectangle region(Point(x*m_tile_size.width, y*m_tile_size.height), m_tile_size);
					auto tile_slot = m_container.atlas.Add(m_cache, region);
					m_tiles[code] = tile_slot;
					m_container.slots[code] = tile_slot;
				}
			}

			// Not needed anymore
			m_cache = Bitmap();
		}
		else
		{
			// FIXME: readd tiles
		}
	}

	void BitmapTileset::Remove()
	{
		for (auto i: m_tiles)
		{
			if (m_container.slots.count(i.first) && (void*)m_container.slots[i.first].get() == (void*)i.second.get())
			{
				m_container.slots.erase(i.first);
			}
		}
	}

	void BitmapTileset::Reload(BitmapTileset&& tileset)
	{
		// FIXME: NYI
		// 1. Remove current tiles from both container and atlas,
		// 2. Add new tiles from other tileset cache
	}

	Size BitmapTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}

	bool BitmapTileset::Provides(uint16_t code)
	{
		return m_tiles.count(code) > 0;
	}

	void BitmapTileset::Prepare(uint16_t code)
	{
		if (!m_tiles.count(code))
		{
			throw std::runtime_error("BitmapTileset::Prepare: request for a tile which is not provided by this tileset");
		}

		m_container.slots[code] = std::dynamic_pointer_cast<Slot>(m_tiles[code]);
	}
}
