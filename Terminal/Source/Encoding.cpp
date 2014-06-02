/*
 * Encoding.cpp
 *
 *  Created on: Oct 20, 2013
 *      Author: Cfyz
 */

#include "Encoding.hpp"
#include "Resource.hpp"
#include "Utility.hpp"
#include "Log.hpp"
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
	struct CustomCodepage: Encoding<char>
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
		int base = 0;

		auto add_code = [&](int base, wchar_t code)
		{
			m_forward[base] = code;
			m_backward[code] = base;
		};

		auto parse_code = [](std::string s) -> int
		{
			if (s.empty()) return -1;

			size_t left = s.find_first_not_of(" \t");
			size_t right = s.find_last_not_of(" \t\r");
			if (right == std::string::npos) return -1;

			s = s.substr(left, right-left+1);
			uint16_t result = 0;

			if (s.length() == 6 && (((s[0] == 'U' || s[0] == 'u') && s[1] == '+') || (s[0] == '0' && s[1] == 'x')))
			{
				// Unicode codepoint: U+XXXX
				std::istringstream ss(s.substr(2));
				ss >> std::hex;
				ss >> result;
				return result; // TODO: wchar_t try_parse specialization
			}
			else if (s.length() > 0 && try_parse(s, result))
			{
				// Decimal codepaoint
				return result;
			}
			else
			{
				return -1;
			}
		};

		auto parse_codes = [&](std::string s)
		{
			// codes ::= <single> | <range>
			// range ::= <single> "-" <single>
			// single ::= <decimal> | <unicode>
			// decimal ::= N
			// unicode ::= U+XXXX

			if (s.empty()) return;
			size_t n = s.find("-");

			if (n == std::string::npos) // Not present in string
			{
				// Single
				int code = parse_code(s);
				if (code >= 0) add_code(base++, (wchar_t)code);
			}
			else if (n < s.length()-1) // Not a last character of string
			{
				// Range
				int from = parse_code(s.substr(0, n));
				int to = parse_code(s.substr(n+1, s.length()-n));
				if (from >= 0 && to > from)
				{
					for (wchar_t code=from; code <= to; code++)
					{
						add_code(base++, code);
					}
				}
			}
		};

		for (std::string line; std::getline(stream, line);)
		{
			size_t i = 0, j = std::string::npos;
			while (i < line.length() && (j=line.find(",", i)) != std::string::npos)
			{
				parse_codes(line.substr(i, j-i));
				i = j+1;
			}
			if (i < line.length()) parse_codes(line.substr(i));
		}
	}

	wchar_t CustomCodepage::Convert(int value) const
	{
		auto i = m_forward.find(value);
		return (i == m_forward.end())? kUnicodeReplacementCharacter: i->second;
	}

	int CustomCodepage::Convert(wchar_t value) const
	{
		auto i = m_backward.find(value);
		return (i == m_backward.end())? 0x1A: i->second; // 0x1A is an ASCII substitute character
	}

	std::wstring CustomCodepage::Convert(const std::string& value) const
	{
		std::wstring result(value.length(), 0);
		for (size_t i=0; i<value.length(); i++)
		{
			auto j = m_forward.find(value[i]);
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

	struct UTF8Encoding: Encoding<char>
	{
		wchar_t Convert(int value) const;
		int Convert(wchar_t value) const;
		std::wstring Convert(const std::string& value) const;
		std::string Convert(const std::wstring& value) const;
		std::wstring GetName() const;
	};

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
			/*
			switch (extraBytesToRead)
			{
				case 5: ch += (uint8_t)value[index++]; ch <<= 6; // illegal UTF-8
				case 4: ch += (uint8_t)value[index++]; ch <<= 6; // illegal UTF-8
				case 3: ch += (uint8_t)value[index++]; ch <<= 6;
				case 2: ch += (uint8_t)value[index++]; ch <<= 6;
				case 1: ch += (uint8_t)value[index++]; ch <<= 6;
				case 0: ch += (uint8_t)value[index++];
			}
			/*/
			for (int i=extraBytesToRead; i>=0; i--)
			{
				ch += (uint8_t)value[index++];
				if (i > 0) ch <<= 6;
			}
			//*/
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

	std::unique_ptr<Encoding<char>> UTF8(new UTF8Encoding());

	// ------------------------------------------------------------------------

	struct UCS2Encoding: Encoding<char16_t>
	{
		wchar_t Convert(int value) const;
		int Convert(wchar_t value) const;
		std::wstring Convert(const std::u16string& value) const;
		std::u16string Convert(const std::wstring& value) const;
		std::wstring GetName() const;
	};

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

	std::unique_ptr<Encoding<char16_t>> UTF16(new UCS2Encoding());

	// ------------------------------------------------------------------------

	struct UCS4Encoding: Encoding<char32_t>
	{
		wchar_t Convert(int value) const;
		int Convert(wchar_t value) const;
		std::wstring Convert(const std::u32string& value) const;
		std::u32string Convert(const std::wstring& value) const;
		std::wstring GetName() const;
	};

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

	std::unique_ptr<Encoding<char32_t>> UTF32(new UCS4Encoding());

	// ------------------------------------------------------------------------

	std::unique_ptr<Encoding<char>> GetUnibyteEncoding(const std::wstring& name)
	{
		if (name == L"utf8" || name == L"utf-8")
		{
			return std::unique_ptr<Encoding<char>>(new UTF8Encoding());
		}
		else
		{
			auto stream = Resource::Open(name, L"codepage-");
			if (!stream)
			{
				throw std::runtime_error("failed to locate codepage resource for \"" + UTF8->Convert(name) + "\"");
			}
			return std::unique_ptr<Encoding<char>>(new CustomCodepage(name, *stream));
		}
	}
}
