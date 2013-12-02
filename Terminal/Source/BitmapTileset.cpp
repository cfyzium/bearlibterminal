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
#include "Palette.hpp"
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

		Size resize_to;
		if (group.attributes.count(L"resize") && !try_parse(group.attributes[L"resize"], resize_to))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'resize' attribute");
		}

		if (group.attributes.count(L"codepage"))
		{
			// TODO: check for error
			m_codepage = GetUnibyteEncoding(group.attributes[L"codepage"]);
		}

		if (!m_tile_size.Area() && !m_codepage)
		{
			// Try to guess from filename
			std::wstring name = group.attributes[L"name"];
			std::wstring l1, l2;

			// Cut off extension
			size_t n = name.find_last_of(L'.');
			if (n != std::wstring::npos)
			{
				name = name.substr(0, n);
			}

			// Last part, codepage
			n = name.find_last_of(L'_');
			if (n != std::wstring::npos)
			{
				if (n < name.length()-1) l1 = name.substr(n+1);
				name = name.substr(0, n);
			}

			// Last part, size
			n = name.find_last_of(L'_');
			if (n != std::wstring::npos)
			{
				if (n < name.length()-1) l2 = name.substr(n+1);
			}

			LOG(Debug, "\"" << group.attributes[L"name"] << "\" -> " << "\"" << l1 << "\", \"" << l2 << "\"");

			Size temp_size;
			if (try_parse(l2, temp_size))
			{
				m_tile_size = temp_size;
				m_codepage = GetUnibyteEncoding(l1);
			}
		}

		if (!m_codepage)
		{
			m_codepage = GetUnibyteEncoding(L"utf8");
		}

		if (group.attributes.count(L"bbox") && !try_parse(group.attributes[L"bbox"], m_bbox_size))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'bbox' attribute");
		}

		if (group.attributes.count(L"align") && !try_parse(group.attributes[L"align"], m_alignment))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'alignment' attribute");
		}

		std::wstring name = group.attributes[L"name"];
		uint64_t address = 0;
		if (name.find(L".") == std::wstring::npos && try_parse(name, address))
		{
			LOG(Debug, "Bitmap tileset name \"" << name << "\" is a memory address");

			Size raw_size;
			if (group.attributes.count(L"raw-size") && !try_parse(group.attributes[L"raw-size"], raw_size))
			{
				throw std::runtime_error("BitmapTileset: failed to parse 'raw-size' attribute");
			}

			if (!raw_size.Area() && m_tile_size.Area())
			{
				raw_size = m_tile_size;
			}

			if (!raw_size.Area())
			{
				throw std::runtime_error("BitmapTileset: cannot guess bitmap dimensions for raw bitmap resource");
			}

			const Color* pixels = (const Color*)address;
			m_cache = Bitmap(raw_size, pixels);
		}
		else
		{
			m_cache = LoadBitmap(*Resource::Open(name, L"tileset-"));
		}

		if (!m_cache.GetSize().Area())
		{
			throw std::runtime_error("BitmapTileset: loaded image is empty (zero-sized)");
		}

		if (resize_to.Area())
		{
			m_cache = m_cache.Resize(resize_to);
		}

		if (group.attributes.count(L"transparent"))
		{
			std::wstring& name = group.attributes[L"transparent"];
			Color mask = name == L"auto"? m_cache(0, 0): Palette::Instance[name];
			m_cache.MakeTransparent(mask);
		}

		if (!m_tile_size.Area())
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

		// Adjust tile size accordingly
		if (resize_to.Area())
		{
			float hf = resize_to.width/(float)m_tile_size.width;
			float vf = resize_to.height/(float)m_tile_size.height;
			m_tile_size.width *= hf;
			m_tile_size.height *= vf;
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
				// TODO: round in a way to compensate state.half_cellsize rounding error
				offset = Point(-m_tile_size.width/2, -m_tile_size.height/2);
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

	Tileset::Type BitmapTileset::GetType()
	{
		return Type::Bitmap;
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
