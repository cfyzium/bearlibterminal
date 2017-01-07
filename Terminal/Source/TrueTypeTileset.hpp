/*
* BearLibTerminal
* Copyright (C) 2013-2017 Cfyz
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

#ifndef TRUETYPETILESET_HPP_
#define TRUETYPETILESET_HPP_

#include <vector>
#include <stdint.h>
#include "Tileset.hpp"
#include "Encoding.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace BearLibTerminal
{
	class TrueTypeTileset: public Tileset
	{
	public:
		TrueTypeTileset(char32_t offset, std::vector<uint8_t> data, OptionGroup& options);
		bool Provides(char32_t code);
		std::shared_ptr<TileInfo> Get(char32_t code);
		Size GetBoundingBoxSize();

	private:
		FT_UInt GetGlyphIndex(char32_t code);
		Size m_tile_size;
		Size m_spacing;
		TileAlignment m_alignment;
		std::unique_ptr<Encoding8> m_codepage;
		std::vector<uint8_t> m_font_data;
		std::shared_ptr<FT_Library> m_font_library;
		std::shared_ptr<FT_Face> m_font_face;
		FT_Render_Mode m_render_mode;
		bool m_monospace;
		bool m_use_box_drawing;
		bool m_use_block_elements;
	};
}

#endif /* TRUETYPETILESET_HPP_ */
