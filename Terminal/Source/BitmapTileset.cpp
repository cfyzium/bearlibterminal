/*
 * BitmapTileset.cpp
 *
 *  Created on: Oct 17, 2013
 *      Author: Cfyz
 */

#include "BitmapTileset.hpp"
#include "LoadBitmap.hpp"
#include "Geometry.hpp"
#include "Resource.hpp"
#include "Utility.hpp"
#include "Log.hpp"
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

		if (!group.attributes.count(L"name") || group.attributes[L"name"].empty())
		{
			throw std::runtime_error("BitmapTileset: missing or empty 'name' attribute");
		}

		if (!group.attributes.count(L"size"))
		{
			throw std::runtime_error("BitmapTileset: 'size' attribute is missing");
		}

		if (!try_parse<Size>(group.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'size' attribute");
		}

		if (group.attributes.count(L"codepage"))
		{
			m_codepage = GetUnibyteEncoding(group.attributes[L"codepage"]);
		}
		else
		{
			m_codepage = GetUnibyteEncoding(L"utf8");
		}

		m_cache = LoadBitmap(*Resource::Open(group.attributes[L"name"], L"tileset-"));

		// TODO: check dimensions
	}

	bool BitmapTileset::Save()
	{
		if (!m_cache.IsEmpty())
		{
			Size image_size = m_cache.GetSize();
			int columns = (int)std::floor(image_size.width / (float)m_tile_size.width);
			int rows = (int)std::floor(image_size.height / (float)m_tile_size.height);
			LOG(Debug, "Tileset has " << columns << "x" << rows << " tiles");

			int w2 = m_tile_size.width / 2; // TODO: round in a way to compensate state.half_cellsize rounding error
			int h2 = m_tile_size.height / 2;

			for (int y=0; y<rows; y++)
			{
				for (int x=0; x<columns; x++)
				{
					int i = y*columns + x;
					wchar_t j = m_codepage->Convert(i);
					if (j != 0xFFFD) // FIXME: Unicode replacement char
					{
						uint16_t code = m_base_code + j;
						Rectangle region(Point(x*m_tile_size.width, y*m_tile_size.height), m_tile_size);
						auto tile_slot = m_container.atlas.Add(m_cache, region);
						tile_slot->offset = Point(-w2, -h2);
						tile_slot->placement = TileSlot::Placement::Centered;
						m_tiles[code] = tile_slot;
						m_container.slots[code] = tile_slot;
					}
				}
			}

			// Not needed anymore
			m_cache = Bitmap();
		}
		else
		{
			for (auto i: m_tiles) // TODO: strict tileset priority
			{
				m_container.slots[i.first] = i.second;
			}
		}
	}

	void BitmapTileset::Remove()
	{
		for (auto i: m_tiles)
		{
			if (m_container.slots.count(i.first) && (void*)m_container.slots[i.first].get() == (void*)i.second.get()) // TODO: proper equality test
			{
				m_container.slots.erase(i.first);
			}

			m_container.atlas.Remove(i.second);
		}
	}

	void BitmapTileset::Reload(BitmapTileset&& tileset)
	{
		if (m_tiles.size() == 1 && m_tiles.begin()->second->texture_region.Size() == tileset.m_cache.GetSize())
		{
			// Tileset contains one tile with the same dimensions
			m_tiles.begin()->second->Update(tileset.m_cache);
		}
		else
		{
			// Another tileset should contain already validated cache/size/codepage,
			// it should be safe to use them to reinitialize tileset.
			Remove();
			m_cache = std::move(tileset.m_cache);
			m_tile_size = tileset.m_tile_size;
			m_codepage = std::move(tileset.m_codepage);
			Save();
		}
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
