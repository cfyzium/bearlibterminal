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
	class TrueTypeTileset: public StronglyTypedReloadableTileset<TrueTypeTileset>
	{
	public:
		TrueTypeTileset(TileContainer& container, OptionGroup& group);
		~TrueTypeTileset();
		void Dispose();
		void Remove() override;
		bool Save() override;
		void Reload(TrueTypeTileset&& tileset) override;
		Size GetBoundingBoxSize() override;
		Type GetType() override;
		bool Provides(uint16_t code) override;
		void Prepare(uint16_t code) override;
	private:
		Bitmap PrepareBitmap(uint16_t code);
		uint16_t m_base_code;
		Size m_tile_size;
		Size m_bbox_size;
		std::unique_ptr<Encoding<char>> m_codepage;
		Tile::Alignment m_alignment;
		FT_Library m_font_library;
		FT_Face m_font_face;
		FT_Render_Mode m_render_mode;
		bool m_monospace;
	};
}

#endif /* TRUETYPETILESET_HPP_ */
