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

#ifndef BEARLIBTERMINAL_TILESET_HPP
#define BEARLIBTERMINAL_TILESET_HPP

#include "Atlas.hpp"
#include "OptionGroup.hpp"
#include <memory>
#include <map>

namespace BearLibTerminal
{
	class Tileset
	{
	public:
		Tileset(char32_t offset);
		virtual ~Tileset();
		char32_t GetOffset() const;
		virtual bool Provides(char32_t code);
		virtual std::shared_ptr<TileInfo> Get(char32_t code);
		virtual Size GetBoundingBoxSize() = 0;

		static const char32_t kFontOffsetMask = 0xFF000000;
		static const char32_t kCharOffsetMask = 0x00FFFFFF;
		static std::shared_ptr<Tileset> Create(OptionGroup& options, char32_t offset);
		static bool IsFontOffset(char32_t offset);

	protected:
		char32_t m_offset;
		std::unordered_map<char32_t, std::shared_ptr<TileInfo>> m_cache;
	};

	extern std::unordered_map<char32_t, std::shared_ptr<TileInfo>> g_codespace;

	extern std::map<char32_t, std::shared_ptr<Tileset>> g_tilesets;

	void AddTileset(std::shared_ptr<Tileset> tileset);

	void RemoveTileset(std::shared_ptr<Tileset> tileset);

	void RemoveTileset(char32_t offset);
}

#endif // BEARLIBTERMINAL_TILESET_HPP
