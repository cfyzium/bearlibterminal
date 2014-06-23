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

#ifndef BEARLIBTERMINAL_ENCODING_HPP
#define BEARLIBTERMINAL_ENCODING_HPP

#include <string>
#include <memory>
#include <stdint.h>

namespace BearLibTerminal
{
	template<typename CharT> struct Encoding
	{
		virtual ~Encoding() { }
		virtual wchar_t Convert(int value) const = 0;
		virtual int Convert(wchar_t value) const = 0;
		virtual std::wstring Convert(const std::basic_string<CharT>& value) const = 0;
		virtual std::basic_string<CharT> Convert(const std::wstring& value) const = 0;
		virtual std::wstring GetName() const = 0;
		bool operator==(const Encoding& another) const {return GetName() == another.GetName();}
		bool operator!=(const Encoding& another) const {return GetName() != another.GetName();}
	};

	typedef Encoding<char> Encoding8;
	typedef Encoding<char16_t> Encoding16;
	typedef Encoding<char32_t> Encoding32;

	std::unique_ptr<Encoding8> GetUnibyteEncoding(const std::wstring& name);

	/*
	 * NOTE: these are not necessarily proper UTF codecs; only to the extent
	 * the terminal library understands UTF, i. e. UCS-2.
	 * FIXME: probably better name them UTF8/UCS2/UCS4
	 */
	extern std::unique_ptr<Encoding8>  UTF8;
	extern std::unique_ptr<Encoding16> UTF16;
	extern std::unique_ptr<Encoding32> UTF32;

	static const uint16_t kUnicodeReplacementCharacter = 0xFFFD;
}

#endif // BEARLIBTERMINAL_ENCODING_HPP
