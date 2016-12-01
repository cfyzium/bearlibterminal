/*
* BearLibTerminal
* Copyright (C) 2013-2016 Cfyz
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

#include "Tileset.hpp"
#include "BitmapTileset.hpp"
#include "TrueTypeTileset.hpp"
#include "DynamicTileset.hpp"
#include "Utility.hpp"
#include "Resource.hpp"
#include "Geometry.hpp"
#include "Log.hpp"
#include <stdexcept>
#include <set>

namespace BearLibTerminal
{
	std::unordered_map<char32_t, std::shared_ptr<TileInfo>> g_codespace;

	std::map<char32_t, std::shared_ptr<Tileset>> g_tilesets;

	std::string GuessResourceFormat(const std::vector<uint8_t>& data)
	{
		auto compare = [&data](const char* magic, size_t size) -> bool
		{
			if (data.size() < size)
				return false;

			return strncmp((const char*)&data[0], magic, size) == 0;
		};

		if (compare("\x89PNG", 4)) // PNG
			return "png";
		else if (compare("BM", 2)) // BMP
			return "bmp";
		else if (compare("\xFF\xD8\xFF", 3)) // JPEG
			return "jpg";
		else if (compare("\x00\x01\x00\x00\x00", 5)) // TTF
			return "ttf";
		else
			return std::string{};
	}

	Tileset::Tileset(char32_t offset):
		m_offset(offset)
	{ }

	Tileset::~Tileset()
	{ }

	char32_t Tileset::GetOffset() const
	{
		return m_offset;
	}

	bool Tileset::Provides(char32_t code)
	{
		return m_cache.find(code) != m_cache.end();
	}

	std::shared_ptr<TileInfo> Tileset::Get(char32_t code)
	{
		auto i = m_cache.find(code);
		if (i == m_cache.end())
			return std::shared_ptr<TileInfo>{};
		return i->second;
	}

	std::shared_ptr<Tileset> Tileset::Create(OptionGroup& options, char32_t offset)
	{
		std::wstring resource = options.attributes[L"_"];
		if (resource.empty())
		{
			throw std::runtime_error("Tileset::Create: main attribute is missing from tileset options");
		}

		if (resource == L"dynamic")
		{
			return std::make_shared<DynamicTileset>(offset, options);
		}

		if (resource == L"default")
		{
			options.attributes[L"size"] = L"8x16";
			options.attributes[L"codepage"] = L"tileset-default";
		}

		if (IsFontOffset(offset) && !options.attributes.count(L"size"))
		{
			throw std::runtime_error("Tileset::Create: font does not have 'size' attribute");\
		}

		bool is_raw_bitmap = options.attributes.count(L"raw-size");

		// Ascertain the raw bitmap address is in general format.
		if (is_raw_bitmap)
		{
			size_t colon_pos = resource.find(L":");
			if (colon_pos == std::wstring::npos)
			{
				Size raw_size;
				if (!try_parse(options.attributes[L"raw-size"], raw_size))
					throw std::runtime_error("Tileset::Create: failed to parse 'raw-size' attribute");
				resource += L":" + to_string<wchar_t>(raw_size.Area() * 4);
			}
		}

		auto data = Resource::Open(resource, L"tileset-");
		std::string format = GuessResourceFormat(data);

		if (is_raw_bitmap || format == "png" || format == "bmp" || format == "jpg")
		{
			if (IsFontOffset(offset) && options.attributes.count(L"transparent") == 0)
			{
				options.attributes[L"transparent"] = L"auto";
			}

			return std::make_shared<BitmapTileset>(offset, std::move(data), options);
		}
		else if (format == "ttf")
		{
			return std::make_shared<TrueTypeTileset>(offset, std::move(data), options);
		}
		else
		{
			throw std::runtime_error("Tileset::Create: resource format is not supported");
		}
	}

	bool Tileset::IsFontOffset(char32_t offset)
	{
		return (offset & kCharOffsetMask) == 0;
	}

	void AddTileset(std::shared_ptr<Tileset> tileset)
	{
		char32_t offset = tileset->GetOffset();
		g_tilesets[offset] = tileset;

		for (auto i = g_codespace.begin(); i != g_codespace.end(); )
		{
			if (i->first >= offset && i->second->tileset->GetOffset() < offset && tileset->Provides(i->first))
			{
				i->second->texture->Remove(i->second, true);
				i = g_codespace.erase(i);
			}
			else
			{
				i++;
			}
		}
	}

	void RemoveTileset(std::shared_ptr<Tileset> tileset)
	{
		for (auto i = g_codespace.begin(); i != g_codespace.end(); )
		{
			if (i->second->tileset == tileset.get())
			{
				i->second->texture->Remove(i->second);
				i = g_codespace.erase(i);
			}
			else
			{
				i++;
			}
		}

		g_tilesets.erase(tileset->GetOffset());
	}

	void RemoveTileset(char32_t offset)
	{
		auto i = g_tilesets.find(offset);
		if (i != g_tilesets.end())
			RemoveTileset(i->second);
	}
}
