/*
 * WindowsGlyphList4.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "WindowsGlyphList4.hpp"

static std::vector<UnicodeRange> Construct()
{
	std::vector<UnicodeRange> result;

	result.push_back(UnicodeRange("C0 Controls and Basic Latin", 0x0000, 0x007F));
	result.back()
		.Add(0x0020, 0x007F);

	result.push_back(UnicodeRange("C1 Controls and Latin-1 Supplement", 0x0080, 0x00FF));
	result.back()
		.Add(0x00A0, 0x00FF);

	result.push_back(UnicodeRange("Latin Extended-A", 0x0100, 0x017F));
	result.back()
		.Add(0x0100, 0x017F);

	result.push_back(UnicodeRange("Latin Extended-B", 0x0180, 0x024F));
	result.back()
		.Add(0x0192)
		.Add(0x01FA, 0x01FF);

	result.push_back(UnicodeRange("Spacing Modifier Letters", 0x02B0, 0x02FF));
	result.back()
		.Add(0x02C6)
		.Add(0x02C7)
		.Add(0x02C9)
		.Add(0x02D8, 0x02DD);

	result.push_back(UnicodeRange("Greek and Coptic", 0x0370, 0x03FF));
	result.back()
		.Add(0x037E)
		.Add(0x0384, 0x038A)
		.Add(0x038C)
		.Add(0x038E, 0x03A1)
		.Add(0x03A3, 0x03CE);

	result.push_back(UnicodeRange("Cyrillic", 0x0400, 0x04FF));
	result.back()
		.Add(0x0400, 0x045F)
		.Add(0x0490, 0x0491);

	result.push_back(UnicodeRange("Latin Extended Additional", 0x1E00, 0x1EFF));
	result.back()
		.Add(0x1E80, 0x1E85)
		.Add(0x1EF2, 0x1EF3);

	result.push_back(UnicodeRange("General Punctuation", 0x2000, 0x206F));
	result.back()
		.Add(0x2013, 0x2015)
		.Add(0x2017, 0x201E)
		.Add(0x2020, 0x2022)
		.Add(0x2026)
		.Add(0x2030)
		.Add(0x2032, 0x2033)
		.Add(0x2039, 0x203A)
		.Add(0x203C)
		.Add(0x203E)
		.Add(0x2044);

	result.push_back(UnicodeRange("Superscripts and Subscript", 0x2070, 0x209F));
	result.back()
		.Add(0x207F);

	result.push_back(UnicodeRange("Currency Symbols", 0x20A0, 0x20CF));
	result.back()
		.Add(0x20A3, 0x20A4)
		.Add(0x20A7)
		.Add(0x20AC);

	result.push_back(UnicodeRange("Letterlike Symbols", 0x2100, 0x214F));
	result.back()
		.Add(0x2105)
		.Add(0x2113)
		.Add(0x2116)
		.Add(0x2122)
		.Add(0x2126)
		.Add(0x212E);

	result.push_back(UnicodeRange("Number Forms", 0x2150, 0x218F));
	result.back()
		.Add(0x215B, 0x215E);

	result.push_back(UnicodeRange("Arrows", 0x2190, 0x21FF));
	result.back()
		.Add(0x2190, 0x2195)
		.Add(0x21A8);

	result.push_back(UnicodeRange("Mathematical Operators", 0x2200, 0x22FF));
	result.back()
		.Add(0x2202)
		.Add(0x2206)
		.Add(0x220F)
		.Add(0x2211, 0x2212)
		.Add(0x2215)
		.Add(0x2219, 0x221A)
		.Add(0x221E, 0x221F)
		.Add(0x2229)
		.Add(0x222B)
		.Add(0x2248)
		.Add(0x2260, 0x2261)
		.Add(0x2264, 0x2265);

	result.push_back(UnicodeRange("Miscellaneous Technical", 0x2300, 0x23FF));
	result.back()
		.Add(0x2302)
		.Add(0x2310)
		.Add(0x2320, 0x2321);

	result.push_back(UnicodeRange("Box Drawing", 0x2500, 0x257F));
	result.back()
		.Add(0x2500, 0x257F);

	result.push_back(UnicodeRange("Block Elements", 0x2580, 0x259F));
	result.back()
		.Add(0x2580, 0x259F);

	result.push_back(UnicodeRange("Geometric Shapes", 0x25A0, 0x25FF));
	result.back()
		.Add(0x25A0, 0x25A1)
		.Add(0x25AA, 0x25AC)
		.Add(0x25B2)
		.Add(0x25BA)
		.Add(0x25BC)
		.Add(0x25C4)
		.Add(0x25CA, 0x25CB)
		.Add(0x25CF)
		.Add(0x25D8, 0x25D9)
		.Add(0x25E6);

	result.push_back(UnicodeRange("Miscellaneous Symbols", 0x2600, 0x26FF));
	result.back()
		.Add(0x263A, 0x263C)
		.Add(0x2640)
		.Add(0x2642)
		.Add(0x2660)
		.Add(0x2663)
		.Add(0x2665, 0x2666)
		.Add(0x266A, 0x266B);

	result.push_back(UnicodeRange("Private Use Area", 0xF000, 0xF00F));
	result.back()
		.Add(0xF001, 0xF002);

	result.push_back(UnicodeRange("Alphabet presentation form", 0xFB00, 0xFB0F));
	result.back()
		.Add(0xFB01, 0xFB02);

	return result;
}

std::vector<UnicodeRange> g_wgl4_ranges = Construct();
