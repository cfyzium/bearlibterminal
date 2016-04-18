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
#include "Log.hpp"
#include <stdexcept>
#include <set>

namespace BearLibTerminal
{
	/*
	Tileset::Tileset(TileContainer& container):
		m_container(container)
	{ }

	Tileset::~Tileset()
	{ }

	std::unique_ptr<Tileset> Tileset::Create(TileContainer& container, OptionGroup& options)
	{
		uint16_t base_code = 0;

		if (options.name != L"font" && !try_parse(options.name, base_code))
		{
			throw std::runtime_error("Tileset::Create: failed to parse font base code");
		}

		if (!options.attributes.count(L"name"))
			options.attributes[L"name"] = options.attributes[L""];
		std::wstring name = options.attributes[L"name"];
		if (name.empty())
		{
			throw std::runtime_error("Tileset::Create: main value attribute is missing or empty");
		}

		if (name == L"none")
		{
			return std::unique_ptr<Tileset>();
		}

		if (name == L"default")
		{
			options.attributes[L"size"] = L"8x16";
			options.attributes[L"codepage"] = L"tileset-default";
		}

		bool is_bitmap = std::set<std::wstring>{L"bmp", L"png", L"jpg", L"jpeg"}.count(to_lower(file_extension(name)));
		bool is_address = name.find(L".") == std::wstring::npos && try_parse<uint64_t>(name);

		if (is_bitmap || is_address || name == L"default")
		{
			LOG(Debug, L"Tileset resource name \"" << name << L"\" is recognized as a name of a bitmap resource");
			return std::unique_ptr<Tileset>(new BitmapTileset(container, options));
		}
		else if (to_lower(file_extension(name)) == L"ttf")
		{
			LOG(Debug, L"Tileset resource name \"" << name << L"\" is recognized as a name of a TrueType resource");
			return std::unique_ptr<Tileset>(new TrueTypeTileset(container, options));
		}
		else if (name == L"dynamic")
		{
			return std::unique_ptr<Tileset>(new DynamicTileset(container, options));
		}

		throw std::runtime_error("Tileset::Create: failed to recognize requested tileset type");
	}
	//*/

	std::unordered_map<char32_t, std::shared_ptr<TileInfo>> g_codespace;

	std::map<char32_t, std::shared_ptr<Tileset>> g_tilesets;

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

	std::shared_ptr<Tileset> Tileset::Create(OptionGroup& options)
	{
		char32_t base_offset = 0;
		if (options.name != L"font" && !try_parse(options.name, base_offset))
			throw std::runtime_error("BitmapTileset: failed to parse tileset offset");

		if (!options.attributes.count(L"name") || options.attributes[L"name"].empty())
			options.attributes[L"name"] = options.attributes[L""];

		if (options.attributes[L"name"] == L"dynamic")
		{
			return std::make_shared<DynamicTileset>(base_offset, options);
		}
		else if (options.attributes[L"name"].find(L".ttf") != std::wstring::npos)
		{
			return std::make_shared<TrueTypeTileset>(base_offset, options);
		}

		if (options.attributes[L"name"] == L"default")
		{
			options.attributes[L"size"] = L"8x16";
			options.attributes[L"codepage"] = L"tileset-default";
		}

		return std::make_shared<BitmapTileset>(base_offset, options); // FIXME
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
