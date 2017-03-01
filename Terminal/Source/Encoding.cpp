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

#include "Encoding.hpp"
#include "Resource.hpp"
#include "Utility.hpp"
#include "Log.hpp"
#include "BOM.hpp"
#include "OptionGroup.hpp"
#include <unordered_map>
#include <stdint.h>
#include <istream>
//#include <iostream>
#include <fstream>

#if !defined(__SIZEOF_WCHAR_T__)
#  if defined(_WIN32)
#    define __SIZEOF_WCHAR_T__ 2
#  else
#    define __SIZEOF_WCHAR_T__ 4
#  endif
#endif

namespace BearLibTerminal
{
	/*
	 * This encoding class is used for both ANSI/OEM unibyte encodings
	 * and custom tileset encodings.
	 */
	struct CustomCodepage: Encoding8
	{
		CustomCodepage(const std::wstring& name, std::istream& stream);
		wchar_t Convert(int value) const;
		int Convert(wchar_t value) const;
		std::wstring Convert(const std::string& value) const;
		std::string Convert(const std::wstring& value) const;
		std::wstring GetName() const;
		std::unordered_map<int, wchar_t> m_forward;  // custom -> wchar_t
		std::unordered_map<wchar_t, int> m_backward; // wchar_t -> custom
		std::wstring m_name;
	};

	CustomCodepage::CustomCodepage(const std::wstring& name, std::istream& stream):
		m_name(name)
	{
		auto bom = DetectBOM(stream);
		LOG(Debug, "Custom codepage file encoding detected to be " << bom);

		switch (bom)
		{
		case BOM::None:
		case BOM::UTF8:
		case BOM::ASCII_UTF8:
			// Supported ones.
			break;
		default:
			throw std::runtime_error("Unsupported custom codepage file encoding " + UTF8Encoding().Convert(to_string<wchar_t>(bom)));
		}

		// Consume BOM
		stream.ignore(GetBOMSize(bom));

		int base = 0;

		auto add_code = [&](int base, wchar_t code)
		{
			m_forward[base] = code;
			m_backward[code] = base;
		};

		auto parse_point = [](const std::wstring& s) -> int
		{
			int result = 0;

			if (s.length() > 2 && (((s[0] == L'u' || s[0] == L'U') && s[1] == '+') || ((s[0] == L'0' && s[1] == L'x'))))
			{
				// Hexadecimal notation: U+1234 or 0x1234.
				std::wistringstream ss(s.substr(2));
				ss >> std::hex;
				ss >> result;
				return ss? result: -2;
			}
			else
			{
				// Decimal notation.
				return try_parse(s, result)? result: -2;
			}

			return -1;
		};

		auto save_single = [&](const std::wstring& point)
		{
			int code = parse_point(point);

			if (code < 0)
			{
				LOG(Warning, L"CustomCodepage: invalid codepoint \"" << point << L"\"");
				return;
			}

			add_code(base++, (wchar_t)code);
		};

		auto save_range = [&](const std::wstring& left, const std::wstring& right)
		{
			int left_code = parse_point(left);
			int right_code = parse_point(right);

			if (left_code < 0 || right_code <= left_code)
			{
				LOG(Warning, L"CustomCodepage: invalid range \"" << left << L"\" - \"" << right << L"\"");
				return;
			}

			for (int i = left_code; i <= right_code; i++)
				add_code(base++, (wchar_t)i);
		};

		auto read_set = [&](const wchar_t*& p)
		{
			auto left = read_until3(p, L"-,");
			if (*p == L'-')
				save_range(left, read_until3(++p, L","));
			else
				save_single(left);
		};

		for (std::string line_u8; std::getline(stream, line_u8);)
		{
			std::wstring line = UTF8Encoding().Convert(line_u8);
			for (const wchar_t* p = line.c_str(); ; p++)
			{
				if (read_set(p), *p == L'\0')
					break;
			}
		}
	}

	wchar_t CustomCodepage::Convert(int value) const
	{
		auto i = m_forward.find(value < 0? (int)((unsigned char)value): value);
		return (i == m_forward.end())? kUnicodeReplacementCharacter: i->second;
	}

	int CustomCodepage::Convert(wchar_t value) const
	{
		auto i = m_backward.find(value);
		return (i == m_backward.end())? -1: i->second; // Can't use ASCII substitute as it may not be ASCII
	}

	std::wstring CustomCodepage::Convert(const std::string& value) const
	{
		std::wstring result(value.length(), 0);
		for (size_t i=0; i<value.length(); i++)
		{
			auto c = value[i];
			auto j = m_forward.find(c < 0? (int)((unsigned char)c): (int)c);
			result[i] = (j == m_forward.end())? kUnicodeReplacementCharacter: (wchar_t)j->second;
		}
		return result;
	}

	std::string CustomCodepage::Convert(const std::wstring& value) const
	{
		std::string result(value.length(), 0);
		for (size_t i=0; i<value.length(); i++)
		{
			auto j = m_backward.find(value[i]);
			result[i] = (j == m_backward.end())? 0x1A: (char)j->second;
		}
		return result;
	}

	std::wstring CustomCodepage::GetName() const
	{
		return m_name;
	}

	// ------------------------------------------------------------------------

	static const wchar_t kSurrogateHighStart	= 0xD800;
	static const wchar_t kSurrogateHighEnd		= 0xDBFF;
	static const wchar_t kUnicodeMaxBmp			= 0xFFFF;

	// Number of trailing bytes that are supposed to follow the first byte
	// of a UTF-8 sequence
	static const uint8_t kTrailingBytesForUTF8[256] =
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
	};

	// Magic values subtracted from a buffer value during UTF8 conversion.
	// This table contains as many values as there might be trailing bytes
	// in a UTF-8 sequence.
	static const uint32_t kOffsetsFromUTF8[6] =
	{
		0x00000000UL,
		0x00003080UL,
		0x000E2080UL,
		0x03C82080UL,
		0xFA082080UL,
		0x82082080UL
	};

	static const wchar_t kReplacementChar = 0x1A; // ASCII 'replacement' character

	wchar_t UTF8Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UTF8Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UTF8Encoding::Convert(const std::string& value) const
	{
		std::wstring result;

		size_t index = 0;
		while (index < value.length())
		{
			size_t extraBytesToRead = kTrailingBytesForUTF8[(uint8_t)value[index]];
			if (index+extraBytesToRead >= value.length())
			{
				// Stop here
				return result;
			}

			// TODO: Do UTF-8 check

			uint32_t ch = 0;
			for (int i=extraBytesToRead; i>=0; i--)
			{
				ch += (uint8_t)value[index++];
				if (i > 0) ch <<= 6;
			}
			ch -= kOffsetsFromUTF8[extraBytesToRead];

			if (ch <= kUnicodeMaxBmp)
			{
				// Target is a character <= 0xFFFF
				if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd)
				{
					result.push_back(kReplacementChar);
				}
				else
				{
					result.push_back((wchar_t)ch); // Normal case
				}
			}
			else // Above 0xFFFF
			{
				result.push_back(kReplacementChar);
			}
		}

		return result;
	}

	std::string UTF8Encoding::Convert(const std::wstring& value) const
	{
		std::string result;

		for (wchar_t c: value)
		{
			if (c < 0x80)
			{
				result.push_back(c & 0x7F);
			}
			else if (c < 0x0800)
			{
				result.push_back(((c >> 6) & 0x1F) | 0xC0);
				result.push_back(((c >> 0) & 0x3F) | 0x80);
			}
			else if (c < 0x10000)
			{
				result.push_back(((c >> 12) & 0x0F) | 0xE0);
				result.push_back(((c >>  6) & 0x3F) | 0x80);
				result.push_back(((c >>  0) & 0x3F) | 0x80);
			}
		}

		return result;
	}

	std::wstring UTF8Encoding::GetName() const
	{
		return L"utf-8";
	}

	// ------------------------------------------------------------------------

	wchar_t UCS2Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UCS2Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UCS2Encoding::Convert(const std::u16string& value) const
	{
#if __SIZEOF_WCHAR_T__ == 2
		return std::wstring((const wchar_t*)value.data(), value.size());
#else
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
#endif
	}

	std::u16string UCS2Encoding::Convert(const std::wstring& value) const
	{
#if __SIZEOF_WCHAR_T__ == 2
		return std::u16string((const char16_t*)value.data(), value.size());
#else
		std::u16string result;
		for (auto c: value) result += (char16_t)c;
		return result;
#endif
	}

	std::wstring UCS2Encoding::GetName() const
	{
		return L"ucs-2";
	}

	// ------------------------------------------------------------------------

	wchar_t UCS4Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UCS4Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UCS4Encoding::Convert(const std::u32string& value) const
	{
#if __SIZEOF_WCHAR_T__ == 4
		return std::wstring((const wchar_t*)value.data(), value.size());
#else
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
#endif
	}

	std::u32string UCS4Encoding::Convert(const std::wstring& value) const
	{
#if __SIZEOF_WCHAR_T__ == 4
		return std::u32string((const char32_t*)value.data(), value.size());
#else
		std::u32string result;
		for (auto c: value) result += (char32_t)c;
		return result;
#endif
	}

	std::wstring UCS4Encoding::GetName() const
	{
		return L"ucs-4";
	}

	// ------------------------------------------------------------------------

	std::unique_ptr<Encoding8> GetUnibyteEncoding(const std::wstring& name)
	{
		if (name == L"utf8" || name == L"utf-8")
		{
			return std::unique_ptr<Encoding8>(new UTF8Encoding());
		}
		else
		{
			auto data = Resource::Open(name, L"codepage-");
			// FIXME: load from string directly.
			std::istringstream stream{std::string{(const char*)&data[0], data.size()}};
			return std::unique_ptr<Encoding8>(new CustomCodepage(name, stream));
		}
	}
}
