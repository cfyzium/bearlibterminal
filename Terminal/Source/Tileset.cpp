/*
 * Tileset.cpp
 *
 *  Created on: Oct 20, 2013
 *      Author: Cfyz
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
}
