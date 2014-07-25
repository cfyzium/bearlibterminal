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

#include <sys/time.h>

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

		std::wstring name = group.attributes[L"name"];

		// Try to guess anyway, rewrite if supplied
		{
			// Try to guess from filename
			// Note name copy is intentional, it will be modified
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

			if (!l1.empty() || !l2.empty())
			{
				LOG(Debug, "Bitmap tileset: guessing encoding and tile size: \"" << group.attributes[L"name"] << "\" -> " << "\"" << l1 << "\", \"" << l2 << "\"");
			}

			Size temp_size;
			if (try_parse(l2, temp_size))
			{
				m_tile_size = temp_size;
				m_codepage = GetUnibyteEncoding(l1);
			}
			else
			{
				LOG(Debug, "Bitmap tileset: failed to parse guessed tile size, not an error");
			}
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

		ResizeFilter resize_filter = ResizeFilter::Bilinear;
		if (group.attributes.count(L"resize-filter") && !try_parse(group.attributes[L"resize-filter"], resize_filter))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'resize-filter' attribute");
		}

		ResizeMode resize_mode = ResizeMode::Stretch;
		if (group.attributes.count(L"resize-mode") && !try_parse(group.attributes[L"resize-mode"], resize_mode))
		{
			throw std::runtime_error("BitmapTileset: failed to parse 'resize-mode' attribute");
		}

		if (group.attributes.count(L"codepage"))
		{
			m_codepage = GetUnibyteEncoding(group.attributes[L"codepage"]); // Should either return an encoding or throw
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
			LOG(Trace, "BitmapTileset: resizing tileset image to " << resize_to << " with " << resize_filter << " filter, " << resize_mode << " mode");
			Size prev = m_cache.GetSize();
			m_cache = m_cache.Resize(resize_to, resize_filter, resize_mode);
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
		// Tiles are provided on-demand.
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

		m_tiles.clear();
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
			// Note that Reload is only used new objects so m_cache is not empty yet.
			m_tiles.begin()->second->Update(tileset.m_cache);
		}
		else
		{
			// Another tileset should contain already validated cache/size/codepage,
			// it should be safe to use them to reinitialize tileset.
			Remove();
			m_cache = std::move(tileset.m_cache);
			m_tile_size = tileset.m_tile_size;
			m_grid_size = tileset.m_grid_size;
			m_codepage = std::move(tileset.m_codepage);
			Save();
		}
	}

	Size BitmapTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}

	Size BitmapTileset::GetSpacing()
	{
		return m_bbox_size;
	}

	Tileset::Type BitmapTileset::GetType()
	{
		return Type::Bitmap;
	}

	bool BitmapTileset::Provides(uint16_t code)
	{
		int index = m_codepage->Convert((wchar_t)(code - m_base_code)); // FIXME: negative
		return (index >= 0 && index <= m_grid_size.Area());
	}

	void BitmapTileset::Prepare(uint16_t code)
	{
		if (!m_tiles.count(code))
		{
			Point offset;
			if (m_alignment == Tile::Alignment::Center)
			{
				// TODO: round in a way to compensate state.half_cellsize rounding error
				offset = Point(-m_tile_size.width/2, -m_tile_size.height/2);
			}

			int index = m_codepage->Convert((wchar_t)(code-m_base_code));
			int column = index % m_grid_size.width;
			int row = (index-column) / m_grid_size.width;

			Rectangle region(Point(column*m_tile_size.width, row*m_tile_size.height), m_tile_size);
			auto tile_slot = m_container.atlas.Add(m_cache, region);
			tile_slot->offset = offset;
			tile_slot->alignment = m_alignment;
			tile_slot->bounds = m_bbox_size;
			m_tiles[code] = tile_slot;

			if (m_tiles.size() == m_grid_size.Area())
			{
				// Every tile was added to container, cache is not necessary anymore.
				m_cache = Bitmap();
			}
		}

		m_container.slots[code] = std::dynamic_pointer_cast<Slot>(m_tiles[code]);
	}
}
