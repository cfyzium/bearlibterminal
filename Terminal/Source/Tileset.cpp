/*
 * Tileset.cpp
 *
 *  Created on: Oct 20, 2013
 *      Author: Cfyz
 */

#include "Tileset.hpp"
#include "BitmapTileset.hpp"
#include "Utility.hpp"
#include <stdexcept>

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
		if (options.name != L"font" && !try_parse<uint16_t>(options.name, base_code))
		{
			throw std::runtime_error("Tileset::Create: failed to parse font base code");
		}

		if (!options.attributes.count(L"name"))
		{
			throw std::runtime_error("Tileset::Create: 'name' attribute is missing");
		}

		if (options.attributes[L"name"] == L"none")
		{
			return std::unique_ptr<Tileset>();
		}

		if (options.attributes[L"name"] == L"default")
		{
			options.attributes[L"name"] = L"default.png";
			options.attributes[L"size"] = L"16x16";
		}

		if (ends_with<wchar_t>(options.attributes[L"name"], L".png")) // bmp, jpg
		{
			return std::unique_ptr<Tileset>(new BitmapTileset(container, options));
		}

		throw std::runtime_error("Tileset::Create: failed to recognize requested tileset type");
	}
}
