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

#include "TrueTypeTileset.hpp"
#include "Geometry.hpp"
#include "Utility.hpp"
#include "Log.hpp"
#include <cmath>
#include <freetype/ftlcdfil.h>
#include <freetype/ftglyph.h>

namespace BearLibTerminal
{
	TrueTypeTileset::TrueTypeTileset(char32_t offset, std::vector<uint8_t> data, OptionGroup& options):
		Tileset(offset),
		m_alignment(TileAlignment::Center),
		m_font_data(std::move(data)),
		m_font_library(nullptr),
		m_font_face(nullptr),
		m_render_mode(FT_RENDER_MODE_NORMAL)
	{
		if (options.attributes.count(L"spacing") && !try_parse(options.attributes[L"spacing"], m_spacing))
			throw std::runtime_error("TrueTypeTileset: failed to parse 'spacing' attribute");

		if (m_spacing.width < 1 || m_spacing.height < 1)
			m_spacing = Size{1, 1};

		if (options.attributes.count(L"codepage"))
			m_codepage = GetUnibyteEncoding(options.attributes[L"codepage"]); // Should either return an encoding or throw
		else
			m_codepage = GetUnibyteEncoding(L"utf8");

		if (options.attributes.count(L"align") && !try_parse(options.attributes[L"align"], m_alignment))
		{
			throw std::runtime_error("TrueTypeTileset: failed to parse 'align' attribute");
		}

		if (options.attributes.count(L"mode"))
		{
			std::wstring mode_str = options.attributes[L"mode"];
			if (mode_str == L"normal")
				m_render_mode = FT_RENDER_MODE_NORMAL;
			else if (mode_str == L"monochrome")
				m_render_mode = FT_RENDER_MODE_MONO;
			else if (mode_str == L"lcd")
				m_render_mode = FT_RENDER_MODE_LCD;
			else
				throw std::runtime_error("TrueTypeTileset: failed to parse 'mode' attribute");
		}

		if (!options.attributes.count(L"size"))
			throw std::runtime_error("TrueTypeTileset: missing 'size' attribute");

		std::wstring size_str = options.attributes[L"size"];
		if (size_str.find(L"x") != std::wstring::npos)
		{
			if (!try_parse(size_str, m_tile_size))
				throw std::runtime_error("TrueTypeTileset: failed to parse 'size' attribute");
		}
		else
		{
			if (!try_parse(size_str, m_tile_size.height))
				throw std::runtime_error("TrueTypeTileset: failed to parse 'size' attribute");
		}

		if (m_tile_size.width < 0 || m_tile_size.height < 1)
			throw std::runtime_error("TrueTypeTileset: size " + UTF8Encoding().Convert(options.attributes[L"size"]) + " is out of acceptable range");

		// Trying to initialize FreeType
		m_font_library = std::shared_ptr<FT_Library>(new FT_Library(), [](FT_Library* p){FT_Done_FreeType(*p);});
		if (FT_Init_FreeType(m_font_library.get()))
			throw std::runtime_error("TrueTypeTileset: can't initialize Freetype");

		m_font_face = std::shared_ptr<FT_Face>(new FT_Face, [](FT_Face* p){FT_Done_Face(*p);});
		if (FT_New_Memory_Face(*m_font_library, &m_font_data[0], m_font_data.size(), 0, m_font_face.get()))
			throw std::runtime_error("TrueTypeTileset: can't load font from buffer");

		int hres = 64;
		FT_Matrix matrix =
		{
			(int)((1.0/hres) * 0x10000L),
			(int)((0.0)      * 0x10000L),
			(int)((0.0)      * 0x10000L),
			(int)((1.0)      * 0x10000L)
		};
		FT_Set_Transform(*m_font_face, &matrix, NULL);

		auto get_metrics = [&](char32_t code) -> FT_Glyph_Metrics
		{
			if (FT_Load_Glyph(*m_font_face, FT_Get_Char_Index(*m_font_face, code), 0))
				throw std::runtime_error("TrueTypeTileset: glyph loading error");
			return (*m_font_face)->glyph->metrics;
		};

		char32_t reference_code = m_codepage->Convert((int)'@');
		if (reference_code == kUnicodeReplacementCharacter)
		{
			// Use first character for size reference if font has custom codepage w/o 0x40 code point
			reference_code = m_codepage->Convert(0);
		}

		if (options.attributes.count(L"size-reference") && !try_parse(options.attributes[L"size-reference"], reference_code))
			throw std::runtime_error("TrueTypeTileset: can't parse 'size-reference' attribute");

		if (m_tile_size.width == 0)
		{
			// Only height was specified, e. g. size=12

			if (FT_Set_Char_Size(*m_font_face, 0, (uint32_t)(m_tile_size.height*64), 96*hres, 96))
				throw std::runtime_error("TrueTypeTileset: can't setup font size");

			int dot_width = (int)std::ceil(get_metrics('.').horiAdvance/64.0f/64.0f);
			int at_width = (int)std::ceil(get_metrics(reference_code).horiAdvance/64.0f/64.0f);
			m_monospace = dot_width == at_width;

			int height = (*m_font_face)->size->metrics.height >> 6;
			int width = at_width;

			m_tile_size = Size(width, height);

			LOG(Trace, "Font tile size is " << m_tile_size << ", font is " << (m_monospace? "monospace": "not monospace"));
		}
		else
		{
			// Glyph bbox was specified, e. g. size=8x12
			// Must guess required character size

			auto get_size = [&](float height) -> Size
			{
				if (FT_Set_Char_Size(*m_font_face, 0, (uint32_t)(height*64), 96*hres, 96))
					throw std::runtime_error("TrueTypeTileset: can't setup font size");

				int w = (int)std::ceil(get_metrics(reference_code).horiAdvance/64.0f/64.0f);
				int h = (*m_font_face)->size->metrics.height >> 6;

				return Size{w, h};
			};

			float left = 1.0f;
			float right = 96.0f;
			float guessed = 10.0f;

			while (true)
			{
				Size size = get_size(guessed);
				if (size.width > m_tile_size.width || size.height > m_tile_size.height)
				{
					// This is too big by at least one dimension.
					float temp = guessed;
					right = guessed;
					guessed = (left + right) / 2.0f;
					LOG(Trace, "height " << temp << " was to big (" << size << " / " << m_tile_size << "), will try " << guessed << "; [" << left << ", " << right << "]");
				}
				else if (size.width <= m_tile_size.width && size.height <= m_tile_size.height)
				{
					// Smaller than or equal to requested size.
					if (right - guessed < 1.0f)
					{
						LOG(Trace, "height " << guessed << " is smaller or equal (" << size << " / " << m_tile_size << "), but right border " << right << " is too close, stopping");
						break;
					}
					else
					{
						float temp = guessed;
						left = guessed;
						guessed = (left + right) / 2.0f;
						LOG(Trace, "height " << temp << " is smaller or equal (" << size << " / " << m_tile_size << "), will try " << guessed << "; [" << left << ", " << right << "]");
					}
				}
			}

			int dot_width = (int)std::ceil(get_metrics('.').horiAdvance/64.0f/64.0f);
			int at_width = (int)std::ceil(get_metrics(reference_code).horiAdvance/64.0f/64.0f);
			m_monospace = dot_width == at_width;

			//int height = (*m_font_face)->size->metrics.height >> 6;
			//int width = at_width;

			LOG(Trace, "Font tile size is " << m_tile_size << ", font is " << (m_monospace? "monospace": "not monospace"));
		}

		if (m_render_mode == FT_RENDER_MODE_LCD)
		{
			FT_Library_SetLcdFilter(*m_font_library, FT_LCD_FILTER_DEFAULT);
			FT_Library_SetLcdFilterWeights(*m_font_library, (unsigned char*)"\x20\x70\x70\x70\x20");
		}

		if (m_alignment == TileAlignment::Unknown)
			m_alignment = TileAlignment::Center;
	}

	FT_UInt TrueTypeTileset::GetGlyphIndex(char32_t code)
	{
		if (code < m_offset)
			return 0;

		if (Tileset::IsFontOffset(m_offset))
		{
			// Font
			char32_t char_code = (code & Tileset::kCharOffsetMask);
			return FT_Get_Char_Index(*m_font_face, char_code);
		}
		else
		{
			// Tileset
			int index = code - m_offset;
			char32_t char_code = m_codepage->Convert(index);
			if (char_code == kUnicodeReplacementCharacter)
				return 0;
			return FT_Get_Char_Index(*m_font_face, char_code);
		}
	}

	bool TrueTypeTileset::Provides(char32_t code)
	{
		char32_t relative_code = (code & Tileset::kCharOffsetMask);
		if (relative_code >= 0x2500 && relative_code <= 0x259F)
			return false; // TrueType tilesets do not provide Box Drawing and Block Elements characters.

		return GetGlyphIndex(code) > 0;
	}

	std::shared_ptr<TileInfo> TrueTypeTileset::Get(char32_t code)
	{
		if (auto cached = Tileset::Get(code))
			return cached;

		FT_UInt index = GetGlyphIndex(code);
		if (index == 0)
			throw std::runtime_error("TrueTypeTileset: request for a tile that is not provided by the tileset");

		if (FT_Load_Glyph(*m_font_face, index, FT_LOAD_FORCE_AUTOHINT))
			throw std::runtime_error("TrueTypeTileset: can't load character glyph");

		if ((*m_font_face)->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Render_Mode render_mode = m_render_mode;

			if (FT_Render_Glyph((*m_font_face)->glyph, render_mode) != 0)
			{
				throw std::runtime_error("TrueTypeTileset: can't render glyph");
			}
		}

		FT_GlyphSlot& slot = (*m_font_face)->glyph;

		int rows = slot->bitmap.rows;
		int columns = 0;
		int pixel_size = 0;

		int height = (*m_font_face)->size->metrics.height >> 6;
		int descender = (*m_font_face)->size->metrics.descender >> 6;
		int bx = (slot->metrics.horiBearingX >> 6) / 64;
		int by = slot->metrics.horiBearingY >> 6;

		if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
		{
			columns = slot->bitmap.width;
			pixel_size = 1;
		}
		else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		{
			columns = slot->bitmap.width/3;
			pixel_size = 3;
		}
		else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
		{
			columns = slot->bitmap.width;
			pixel_size = 0;
		}

		Bitmap glyph(Size(columns, rows), Color(0, 0, 0, 0));

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < columns; x++)
			{
				uint8_t* p = slot->bitmap.buffer + y*slot->bitmap.pitch + x*pixel_size;
				if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
				{
					Color c(p[0], 255, 255, 255);
					glyph(x, y) = c;
				}
				else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
				{
					Color c(255, p[0], p[1], p[2]);
					glyph(x, y) = c;
				}
				else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
				{
					int j = x%8;
					int i = (x-j)/8;
					uint8_t byte = *(slot->bitmap.buffer + y*slot->bitmap.pitch + i);
					uint8_t alpha = (byte & (1 << (7-j)))? 255: 0;
					glyph(x, y) = Color(alpha, 255, 255, 255);
				}
			}
		}

		int descender2 = (*m_font_face)->size->metrics.descender >> 6;
		float wff = slot->metrics.horiAdvance / 4096.0f;
		float hff = (*m_font_face)->size->metrics.height / 64.0f;
		int dy = -((by-descender2) - hff/2);
		Point offset;
		if (m_alignment == TileAlignment::Center)
		{
			int dx = -std::round((wff + 0.5f) / 2.0f) + bx;
			offset = Point(dx, dy);
		}
		else if (m_alignment == TileAlignment::DeadCenter)
		{
			Point center = glyph.CenterOfMass();
			offset = Point(-center.x, -center.y);
		}
		else
		{
			if (m_monospace)
				offset = Point(bx, m_tile_size.height/2+dy);
			else
				offset = Point(m_tile_size.width/2-(columns+bx)/2, m_tile_size.height/2+dy);
		}

		auto tile = std::make_shared<TileInfo>();
		tile->tileset = this;
		tile->bitmap = glyph;
		tile->offset = offset;
		tile->alignment = m_alignment;
		tile->spacing = m_spacing;
		m_cache[code] = tile;

		return tile;
	}

	Size TrueTypeTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}
}

