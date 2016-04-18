/*
 * FreeTypeTileset.hpp
 *
 *  Created on: Nov 7, 2013
 *      Author: cfyz
 */

#ifndef TRUETYPETILESET_HPP_
#define TRUETYPETILESET_HPP_

#include "Tileset.hpp"
#include "Encoding.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace BearLibTerminal
{
	class TrueTypeTileset: public Tileset
	{
	public:
		TrueTypeTileset(char32_t offset, OptionGroup& options);
		bool Provides(char32_t code);
		std::shared_ptr<TileInfo> Get(char32_t code);
		Size GetBoundingBoxSize();

	private:
		FT_UInt GetGlyphIndex(char32_t code);
		Size m_tile_size;
		Size m_spacing;
		TileAlignment m_alignment;
		std::unique_ptr<Encoding8> m_codepage;
		std::shared_ptr<FT_Library> m_font_library;
		std::shared_ptr<FT_Face> m_font_face;
		FT_Render_Mode m_render_mode;
		bool m_monospace;
	};
}

#endif /* TRUETYPETILESET_HPP_ */
