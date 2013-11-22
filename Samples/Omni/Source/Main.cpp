/*
 * Main.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#include "BearLibTerminal.h"
#if defined(_WIN32)
#include <Windows.h>
#endif
#if defined(__linux)
#include <unistd.h>
inline void Sleep(int ms) {usleep(ms*1000);}
#endif
#include <GL/gl.h>

TERMINAL_TAKE_CARE_OF_WINMAIN

void TestWGL4();

int main()
{
	terminal_open();
	terminal_setf("window.title='%s'", "Test title");
	terminal_set("window.cellsize=23x23");
	terminal_set("0xE000: default.png, size=16x16");
	terminal_set("0xE200: sample.jpeg");
	terminal_set("0xE201: sample2.jpg");
	terminal_set("0xE300: circle.png, align=top-left, bbox=2x2");
	terminal_set("0xE301: circle.png, align=top-right, bbox=2x2");
	terminal_set("0xE302: circle.png, align=bottom-left, bbox=2x2");
	terminal_set("0xE303: circle.png, align=bottom-right, bbox=2x2");
	terminal_set("0xE304: circle.png, align=center, bbox=2x2");
	terminal_set("0xE400: UbuntuMono-R.ttf, size=12");
	terminal_set("0xE500: Ubuntu-R.ttf, size=12");
	terminal_set("0xE600: UbuntuMono-RI.ttf, size=12");
	terminal_set("0xE700: UbuntuMono-R.ttf, size=24x24");
	terminal_color(0xFF000000);
	terminal_bkcolor(0xFFEE9000);
	terminal_print(2, 2, L"Hello, [color=white]world[/color].[U+2250] \x1234 {абв} [base=0xE000]abc");

	terminal_bkcolor("black");
	color_t corners[] = {0xFFFF0000, 0xFF00FF00, 0xFF6060FF, 0xFFFF00FF};
	terminal_put_ext(2, 4, 0, 0, L'я', corners);
	//terminal_bkcolor(0xFFFFFFFF);
	terminal_put_ext(3, 5, 0, 0, 0x2593, corners);
	terminal_put_ext(4, 4, 0, 0, 11*16+2, corners);
	terminal_bkcolor("black");

	terminal_color(0xFFFFFFFF);
	terminal_wprint(2, 3, L"a[+]ˆ, c[+][color=red]/[/color], d[+][color=orange][U+2044]");
	//terminal_bkcolor(0xFFEE9000);
	/*
	for (int y=0; y<16; y++)
	{
		for (int x=0; x<16; x++)
		{
			//terminal_put(6+x, 6+y, y*16+x);
			terminal_put(6+x, 6+y, 0x2500+y*16+x);
		}
	}
	/*/
	terminal_composition(TK_COMPOSITION_ON);
	int cw = terminal_state(TK_CELL_WIDTH);
	int ch = terminal_state(TK_CELL_HEIGHT);
	for (int y=0; y<16; y++)
	{
		for (int x=0; x<16; x++)
		{
			//terminal_put_ext(6, 6, x*(cw+1), y*(ch+1), 0x2500+y*16+x, NULL);
			terminal_put_ext(6, 6, x*(cw+2), y*(ch+2), 0x2500+y*16+x, NULL);
		}
	}
	//*/

	terminal_color(0xFFFFFFFF);
	terminal_put(28, 10, 0xE200);

	terminal_layer(1);
	terminal_put(27, 9, 0xE201);

	terminal_layer(0);
	terminal_composition(TK_COMPOSITION_ON);
	terminal_put(24, 9, 0xE300);
	terminal_put(24, 9, 0xE301);
	terminal_put(24, 9, 0xE302);
	terminal_put(24, 9, 0xE303);
	terminal_put(24, 9, 0xE304);

	terminal_put(24, 12, 0xE400+'A');
	terminal_put(25, 12, 0xE400+'b');
	terminal_put(26, 12, 0xE400+'c');
	for (int i=0; i<20; i++) terminal_put(24+i, 14, 0xFFFD);
	terminal_print(24, 14, "[base=0xE400]Hello, world!");

	for (int i=0; i<20; i++) terminal_put(24+i, 16, 0xFFFD);
	terminal_print(24, 16, "[base=0xE500]Hello, world!");

	for (int i=0; i<20; i++) terminal_put(24+i, 18, 0xFFFD);
	terminal_print(24, 18, "[base=0xE600]Hello, world!");

	for (int i=0; i<20; i++) terminal_put(24+i, 20, 0xFFFD);
	terminal_print(24, 20, "[base=0xE700]Hello, world!");

	for (int i=0; i<10; i++)
	{
		terminal_put(2+i, 23, 0xFFFD);
		terminal_put(2+i, 23, 0x2500+i);
	}

	terminal_refresh();
	terminal_read();

	TestWGL4();

	//*
	terminal_close();
	return 0;
	//*/

	wchar_t buffer[100] = {0};
	terminal_read_wstr(2, 22, buffer, 16);
	terminal_wprint(2, 23, buffer);
	terminal_refresh();
	terminal_read();

	//terminal_set("");
	terminal_set("output.vsync=false");
	terminal_set("window.cellsize=auto; font: UbuntuMono-R.ttf, size=12; output.asynchronous=false");
	for (int i=0; i<25; i++)
	{
		terminal_clear();
		terminal_put(1+i, 1, L'Ы');
		terminal_custom(1);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glVertex2i(10, 48);
		glVertex2i(10+i*8, 64);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		terminal_custom(0);
		terminal_refresh();
		Sleep(125);
	}

	terminal_close();
	return 0;
}

// U+0020..U+007F: C0 Controls and Basic Latin
// U+00A0..U+00FF: C1 Controls and Latin-1 Supplement
// U+0100..U+017F: Latin Extended-A
// U+0190..U+019F: Latin Extended-B
// U+01F0..U+01FF: --||--
// U+02C0..U+02CF: Spacing Modifier Letters
// U+02D0..U+02DF: --||--
// U+0370..U+03CF: Greek
// U+0400..U+045F: Cyrillic
// U+0490..U+049F: --||--
// U+1E80..U+1E8F: Latin Extended Additional
// U+1EF0..U+1EFF: --||--
// U+2010..U+204F: General Punctuation
// U+2070..U+207F: Super/Subscripts
// U+20A0..U+20AF: Currency Symbols
// U+2100..U+212F: Letterlike symbols
// U+2150..U+215F: Number Forms
// U+2190..U+219F: Arrows
// U+21A0..U+21AF: --||--
// U+2200..U+222F: Mathematical Operators
// U+2240..U+224F: --||--
// U+2260..U+226F: --||--
// U+2300..U+232F: Miscellaneous Technical
// U+2500..U+257F: Box drawing characters
// U+2580..U+259F: Block Elements
// U+25A0..U+25EF: Geometric Shapes
// U+2630..U+264F: Miscellaneous Symbols
// U+2660..U+266F: --||--
// U+F000..U+F00F: Private Use Area
// U+FB00..U+FB00: Alphabetic Presentation Forms


#include <vector>
#include <string>
#include <cmath>
#include <set>
#include <iostream>

namespace
{
	struct UnicodeRange
	{
		std::string name;
		int start, end;
		std::set<int> codes;

		UnicodeRange(const std::string& name, int start, int end):
			name(name),
			start(start),
			end(end)
		{ }

		void Add(int code)
		{
			codes.insert(code);
		}

		void Add(int from, int to)
		{
			for (int i=from; i<=to; i++) codes.insert(i);
		}
	};

	std::vector<UnicodeRange> FillRanges()
	{
		std::vector<UnicodeRange> ranges;
		ranges.push_back(UnicodeRange("C0 Controls and Basic Latin", 0x0000, 0x007F));
		ranges.back().Add(0x0020, 0x007F);
		ranges.push_back(UnicodeRange("C1 Controls and Latin-1 Supplement", 0x0080, 0x00FF));
		ranges.back().Add(0x00A0, 0x00FF);
		ranges.push_back(UnicodeRange("Latin Extended-A", 0x0100, 0x017F));
		ranges.back().Add(0x0100, 0x017F);
		ranges.push_back(UnicodeRange("Latin Extended-B", 0x0180, 0x024F));
		ranges.back().Add(0x0192);
		ranges.back().Add(0x01FA, 0x01FF);
		ranges.push_back(UnicodeRange("Spacing Modifier Letters", 0x02B0, 0x02FF));
		ranges.back().Add(0x02C6);
		ranges.back().Add(0x02C7);
		ranges.back().Add(0x02C9);
		ranges.back().Add(0x02D8, 0x02DD);
		ranges.push_back(UnicodeRange("Greek and Coptic", 0x0370, 0x03FF));
		ranges.back().Add(0x037E);
		ranges.back().Add(0x0384, 0x038A);
		ranges.back().Add(0x038C);
		ranges.back().Add(0x038E, 0x03A1);
		ranges.back().Add(0x03A3, 0x03CE);
		ranges.push_back(UnicodeRange("Cyrillic", 0x0400, 0x04FF));
		ranges.back().Add(0x0400, 0x045F);
		ranges.back().Add(0x0490, 0x0491);
		ranges.push_back(UnicodeRange("Latin Extended Additional", 0x1E00, 0x1EFF));
		ranges.back().Add(0x1E80, 0x1E85);
		ranges.back().Add(0x1EF2, 0x1EF3);
		ranges.push_back(UnicodeRange("General Punctuation", 0x2000, 0x206F));
		ranges.back().Add(0x2013, 0x2015);
		ranges.back().Add(0x2017, 0x201E);
		ranges.back().Add(0x2020, 0x2022);
		ranges.back().Add(0x2026);
		ranges.back().Add(0x2030);
		ranges.back().Add(0x2032, 0x2033);
		ranges.back().Add(0x2039, 0x203A);
		ranges.back().Add(0x203C);
		ranges.back().Add(0x203E);
		ranges.back().Add(0x2044);
		ranges.push_back(UnicodeRange("Superscripts and Subscript", 0x2070, 0x209F));
		ranges.back().Add(0x207F);
		ranges.push_back(UnicodeRange("Currency Symbols", 0x20A0, 0x20CF));
		ranges.back().Add(0x20A3, 0x20A4);
		ranges.back().Add(0x20A7);
		ranges.back().Add(0x20AC);
		ranges.push_back(UnicodeRange("Letterlike Symbols", 0x2100, 0x214F));
		ranges.back().Add(0x2105);
		ranges.back().Add(0x2113);
		ranges.back().Add(0x2116);
		ranges.back().Add(0x2122);
		ranges.back().Add(0x2126);
		ranges.back().Add(0x212E);
		ranges.push_back(UnicodeRange("Number Forms", 0x2150, 0x218F));
		ranges.back().Add(0x215B, 0x215E);
		ranges.push_back(UnicodeRange("Arrows", 0x2190, 0x21FF));
		ranges.back().Add(0x2190, 0x2195);
		ranges.back().Add(0x21A8);
		ranges.push_back(UnicodeRange("Mathematical Operators", 0x2200, 0x22FF));
		ranges.back().Add(0x2202);
		ranges.back().Add(0x2206);
		ranges.back().Add(0x220F);
		ranges.back().Add(0x2211, 0x2212);
		ranges.back().Add(0x2215);
		ranges.back().Add(0x2219, 0x221A);
		ranges.back().Add(0x221E, 0x221F);
		ranges.back().Add(0x2229);
		ranges.back().Add(0x222B);
		ranges.back().Add(0x2248);
		ranges.back().Add(0x2260, 0x2261);
		ranges.back().Add(0x2264, 0x2265);
		ranges.push_back(UnicodeRange("Miscellaneous Technical", 0x2300, 0x23FF));
		ranges.back().Add(0x2302);
		ranges.back().Add(0x2310);
		ranges.back().Add(0x2320, 0x2321);
		ranges.push_back(UnicodeRange("Box Drawing", 0x2500, 0x257F));
		ranges.back().Add(0x2500, 0x257F);
		ranges.push_back(UnicodeRange("Block Elements", 0x2580, 0x259F));
		ranges.back().Add(0x2580, 0x259F);
		ranges.push_back(UnicodeRange("Geometric Shapes", 0x25A0, 0x25FF));
		ranges.back().Add(0x25A0, 0x25A1);
		ranges.back().Add(0x25AA, 0x25AC);
		ranges.back().Add(0x25B2);
		ranges.back().Add(0x25BA);
		ranges.back().Add(0x25BC);
		ranges.back().Add(0x25C4);
		ranges.back().Add(0x25CA, 0x25CB);
		ranges.back().Add(0x25CF);
		ranges.back().Add(0x25D8, 0x25D9);
		ranges.back().Add(0x25E6);
		ranges.push_back(UnicodeRange("Miscellaneous Symbols", 0x2600, 0x26FF));
		ranges.back().Add(0x263A, 0x263C);
		ranges.back().Add(0x2640);
		ranges.back().Add(0x2642);
		ranges.back().Add(0x2660);
		ranges.back().Add(0x2663);
		ranges.back().Add(0x2665, 0x2666);
		ranges.back().Add(0x266A, 0x266B);
		ranges.push_back(UnicodeRange("Private Use Area", 0xF000, 0xF00F));
		ranges.back().Add(0xF001, 0xF002);
		ranges.push_back(UnicodeRange("Alphabet presentation form", 0xFB00, 0xFB0F));
		ranges.back().Add(0xFB01, 0xFB02);
		return ranges;
	}
}

void TestWGL4()
{
	terminal_set("window: size=80x25, cellsize=auto, title='WGL4'; font=default");

	std::vector<UnicodeRange> ranges = FillRanges();

	const int hoffset = 40;
	int current_range = 0;

	while (true)
	{
		terminal_clear();
		//terminal_wprint(1, 1, L"[color=darker green]TIP: Use ↑/↓ keys to select range:");
		terminal_wprint(1, 1, L"[color=white]Select unicode character range:");

		for (int i=0; i<ranges.size(); i++)
		{
			bool selected = i == current_range;
			terminal_color(selected? color_from_name("orange"): color_from_name("light gray"));
			terminal_printf(1, 2+i, "[U+2219] %s", ranges[i].name.c_str());
		}

		UnicodeRange& range = ranges[current_range];
		for (int j=0; j<16; j++)
		{
			terminal_printf(hoffset+6+j*2, 1, "[color=orange]%X", j);
		}
		for (int code=range.start, y=0; code<=range.end; code++)
		{
			if (code%16 == 0) terminal_printf(hoffset, 2+y*1, "[color=orange]%04X:", code);

			bool included = range.codes.count(code);
			terminal_color(included? color_from_name("white"): color_from_name("dark gray"));
			terminal_put(hoffset+6+(code%16)*2, 2+y*1, code);

			if ((code+1)%16 == 0) y += 1;
		}

		terminal_refresh();

		bool exit = false;
		do
		{
			int key = terminal_read();
			if (key == TK_ESCAPE || key == TK_CLOSE)
			{
				exit = true;
				break;
			}
			else if (key == TK_UP)
			{
				if (current_range > 0) current_range -= 1;
			}
			else if (key == TK_DOWN)
			{
				if (current_range < ranges.size()-1) current_range += 1;
			}
		}
		while (terminal_has_input());

		if (exit) break;
	}
}

