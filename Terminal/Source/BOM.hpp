/*
* BearLibTerminal
* Copyright (C) 2015 Cfyz
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

#ifndef BOM_HPP_
#define BOM_HPP_

#include <istream>
#include <ostream>

namespace BearLibTerminal
{
	enum class BOM
	{
		None = -1,
		UTF8 = 0,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE,
		ASCII_Flag = 0x10,
		ASCII_UTF8 = ASCII_Flag,
		ASCII_UTF16LE,
		ASCII_UTF16BE,
		ASCII_UTF32LE,
		ASCII_UTF32BE
	};

	BOM DetectBOM(std::istream& stream);

	void PlaceBOM(std::ostream& stream, BOM bom);

	size_t GetBOMSize(BOM bom);

	std::wostream& operator<<(std::wostream& stream, BOM bom);
}

#endif /* BOM_HPP_ */
