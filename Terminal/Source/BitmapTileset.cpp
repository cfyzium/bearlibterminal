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
		m_base_code(0),
		m_alignment(Tile::Alignment::Unknown)
	{
		if (group.name != L"font" && !try_parse(group.name, m_base_code))
		{
			throw std::runtime_error("BitmapTileset: failed to parse base code");
		}

		if (!group.attributes.count(L"name") || group.attributes[L"name"].empty())
		{
			throw std::runtime_error("BitmapTileset: missing or empty 'name' attribute");
		}

		if (group.attributes.count(L"size") && !try_parse(group.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'size' attribute");
		}

		if (group.attributes.count(L"bbox") && !try_parse(group.attributes[L"bbox"], m_bbox_size))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'bbox' attribute");
		}

		if (group.attributes.count(L"codepage"))
		{
			// TODO: check for error
			m_codepage = GetUnibyteEncoding(group.attributes[L"codepage"]);
		}
		else
		{
			m_codepage = GetUnibyteEncoding(L"utf8");
		}

		if (group.attributes.count(L"align") && !try_parse(group.attributes[L"align"], m_alignment))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'alignemnt' attribute");
		}

		m_cache = LoadBitmap(*Resource::Open(group.attributes[L"name"], L"tileset-"));

		if (m_tile_size.Area() == 0)
		{
			m_tile_size = m_cache.GetSize();
		}
		else if (!Rectangle(m_cache.GetSize()).Contains(Rectangle(m_tile_size)))
		{
			throw std::runtime_error("Bitmap tileset: bitmap is smaller than tile size");
		}

		if (m_bbox_size.width < 1 || m_bbox_size.height < 1)
		{
			m_bbox_size = Size(1, 1);
		}

		Size image_size = m_cache.GetSize();
		int columns = (int)std::floor(image_size.width / (float)m_tile_size.width);
		int rows = (int)std::floor(image_size.height / (float)m_tile_size.height);
		m_grid_size = Size(columns, rows);
		LOG(Debug, "Tileset has " << columns << "x" << rows << " tiles");

		if (m_alignment == Tile::Alignment::Unknown)
		{
			if (m_grid_size.Area() > 1)
			{
				m_alignment = Tile::Alignment::Center;
			}
			else
			{
				m_alignment = Tile::Alignment::TopLeft;
			}
		}
	}

	bool BitmapTileset::Save()
	{
		if (!m_cache.IsEmpty())
		{
			Point offset;
			if (m_alignment == Tile::Alignment::Center)
			{
				offset = Point(-m_tile_size.width/2, -m_tile_size.height/2); // TODO: round in a way to compensate state.half_cellsize rounding error
			}

			// Iterate tiles left to right, top to bottom.
			for (int y=0; y<m_grid_size.height; y++)
			{
				for (int x=0; x<m_grid_size.width; x++)
				{
					int i = y*m_grid_size.width + x;
					wchar_t j = m_codepage->Convert(i);
					if (j != kUnicodeReplacementCharacter)
					{
						uint16_t code = m_base_code + j;
						Rectangle region(Point(x*m_tile_size.width, y*m_tile_size.height), m_tile_size);
						auto tile_slot = m_container.atlas.Add(m_cache, region);
						tile_slot->offset = offset;
						tile_slot->alignment = m_alignment;
						tile_slot->bounds = m_bbox_size;
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

		return true;
	}

	void BitmapTileset::Remove()
	{
		for (auto i: m_tiles)
		{
			if (m_container.slots.count(i.first) && m_container.slots[i.first].get() == i.second.get())
			{
				m_container.slots.erase(i.first);
			}

			m_container.atlas.Remove(i.second);
		}
	}

	void BitmapTileset::Reload(BitmapTileset&& tileset)
	{
		bool eligible_to_update =
			(m_tile_size == tileset.m_tile_size) &&
			(m_bbox_size == tileset.m_bbox_size) &&
			(m_codepage->GetName() == tileset.m_codepage->GetName()) &&
			(m_alignment == tileset.m_alignment) &&
			(m_tiles.size() == 1) &&
			(m_tiles.begin()->second->texture_region.Size() == tileset.m_cache.GetSize());

		if (eligible_to_update)
		{
			// Tileset contains exactly one tile with the same dimensions and
			// all other parameters are the same.
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
