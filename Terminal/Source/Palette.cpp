/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
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

#include "Palette.hpp"
#include "Utility.hpp"

namespace BearLibTerminal
{
	Palette Palette::Instance;

	Palette::Palette()
	{
		m_items[L"transparent"] = m_items[L"none"] = Color(0, 0, 0, 0);
		m_items[L"black"] = Color(0, 0, 0);
		m_items[L"white"] = Color(255,255,255);

		auto f = [&](const std::wstring& base, Color shades[]) -> void
		{
			static std::wstring shade_names[7] =
			{
				L"lightest ", L"lighter ", L"light ", L"", L"dark ", L"darker ", L"darkest "
			};

			for ( size_t i = 0; i < 7; i++ )
			{
				m_items[shade_names[i]+base] = shades[i];
			}
		};

		Color grey[]		= { Color(223,223,223), Color(191,191,191), Color(159,159,159), Color(127,127,127), Color(95,95,95),  Color(63,63,63),  Color(31,31,31) };
		Color red[]			= { Color(255,191,191), Color(255,127,127), Color(255,63,63),   Color(255,0,0),     Color(191,0,0),   Color(127,0,0),   Color(63,0,0) };
		Color flame[]		= { Color(255,207,191), Color(255,159,127), Color(255,111,63),  Color(255,63,0),    Color(191,47,0),  Color(127,31,0),  Color(63,15,0) };
		Color orange[]		= { Color(255,223,191), Color(255,191,127), Color(255,159,63),  Color(255,127,0),   Color(191,95,0),  Color(127,63,0),  Color(63,31,0) };
		Color amber[]		= { Color(255,239,191), Color(255,223,127), Color(255,207,63),  Color(255,191,0),   Color(191,143,0), Color(127,95,0),  Color(63,47,0) };
		Color yellow[]		= { Color(255,255,191), Color(255,255,127), Color(255,255,63),  Color(255,255,0),   Color(191,191,0), Color(127,127,0), Color(63,63,0) };
		Color lime[]		= { Color(239,255,191), Color(223,255,127), Color(207,255,63),  Color(191,255,0),   Color(143,191,0), Color(95,127,0),  Color(47,63,0) };
		Color chartreuse[]	= { Color(223,255,191), Color(191,255,127), Color(159,255,63),  Color(127,255,0),   Color(95,191,0),  Color(63,127,0),  Color(31,63,0) };
		Color green[]		= { Color(191,255,191), Color(127,255,127), Color(63,255,63),   Color(0,255,0),     Color(0,191,0),   Color(0,127,0),   Color(0,63,0) };
		Color sea[]			= { Color(191,255,223), Color(127,255,191), Color(63,255,159),  Color(0,255,127),   Color(0,191,95),  Color(0,127,63),  Color(0,63,31) };
		Color turquoise[]	= { Color(191,255,239), Color(127,255,223), Color(63,255,207),  Color(0,255,191),   Color(0,191,143), Color(0,127,95),  Color(0,63,47) };
		Color cyan[]		= { Color(191,255,255), Color(127,255,255), Color(63,255,255),  Color(0,255,255),   Color(0,191,191), Color(0,127,127), Color(0,63,63) };
		Color sky[]			= { Color(191,239,255), Color(127,223,255), Color(63,207,255),  Color(0,191,255),   Color(0,143,191), Color(0,95,127),  Color(0,47,63) };
		Color azure[]		= { Color(191,223,255), Color(127,191,255), Color(63,159,255),  Color(0,127,255),   Color(0,95,191),  Color(0,63,127),  Color(0,31,63) };
		Color blue[]		= { Color(191,191,255), Color(127,127,255), Color(63,63,255),   Color(0,0,255),     Color(0,0,191),   Color(0,0,127),   Color(0,0,63) };
		Color han[]			= { Color(207,191,255), Color(159,127,255), Color(111,63,255),  Color(63,0,255),    Color(47,0,191),  Color(31,0,127),  Color(15,0,63) };
		Color violet[]		= { Color(223,191,255), Color(191,127,255), Color(159,63,255),  Color(127,0,255),   Color(95,0,191),  Color(63,0,127),  Color(31,0,63) };
		Color purple[]		= { Color(239,191,255), Color(223,127,255), Color(207,63,255),  Color(191,0,255),   Color(143,0,191), Color(95,0,127),  Color(47,0,63) };
		Color fuchsia[]		= { Color(255,191,255), Color(255,127,255), Color(255,63,255),  Color(255,0,255),   Color(191,0,191), Color(127,0,127), Color(63,0,63) };
		Color magenta[]		= { Color(255,191,239), Color(255,127,223), Color(255,63,207),  Color(255,0,191),   Color(191,0,143), Color(127,0,95),  Color(63,0,47) };
		Color pink[]		= { Color(255,191,223), Color(255,127,191), Color(255,63,159),  Color(255,0,127),   Color(191,0,95),  Color(127,0,63),  Color(63,0,31) };
		Color crimson[]		= { Color(255,191,207), Color(255,127,159), Color(255,63,111),  Color(255,0,63),    Color(191,0,47),  Color(127,0,31),  Color(63,0,15) };

		f(L"grey", grey);
		f(L"gray", grey);
		f(L"red", red);
		f(L"flame", flame);
		f(L"orange", orange);
		f(L"amber", amber);
		f(L"yellow", yellow);
		f(L"lime", lime);
		f(L"chartreuse", chartreuse);
		f(L"green", green);
		f(L"sea", sea);
		f(L"turquoise", turquoise);
		f(L"cyan", cyan);
		f(L"sky", sky);
		f(L"azure", azure);
		f(L"blue", blue);
		f(L"han", han);
		f(L"violet", violet);
		f(L"purple", purple);
		f(L"fuchsia", fuchsia);
		f(L"magenta", magenta);
		f(L"pink", pink);
		f(L"crimson", crimson);
	}

	Color Palette::operator[] (const std::wstring& name)
	{
		size_t length = name.length();

		if ( length > 0 )
		{
			const wchar_t c = name[0];
			try
			{
				if ( c == L'#' )
				{
					// Color is in hexadecimal numeric format
					// Either #AARRGGBB or #RRGGBB

					if ( length < 7 ) return m_items[L"white"];
					uint32_t value = parse<uint32_t>(name.substr(1), std::hex);
					if ( (value & 0xFF000000) == 0 ) value |= 0xFF000000;
					return Color(value);
				}
				else if ( c == L'-' || (c >= L'0' && c <= L'9') )
				{
					// Color is in decimal numeric format
					uint32_t value = (uint32_t)parse<int32_t>(name, std::dec);
					if ( (value & 0xFF000000) == 0 ) value |= 0xFF000000;
					return Color(value);
				}
			}
			catch ( std::exception& e )
			{
				return m_items[L"white"];
			}
		}

		auto i = m_items.find(name);
		return (i != m_items.end()? i->second: m_items[L"white"]);
	}
}
