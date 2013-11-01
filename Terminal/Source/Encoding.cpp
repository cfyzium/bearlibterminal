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

#include <iostream>
#include <fstream>

namespace BearLibTerminal
{
	/*
	 * This encoding class is used for both ANSI/OEM unibyte encodings
	 * and custom tileset encodings.
	 */
	struct CustomCodepage: Encoding<char>
	{
		CustomCodepage(std::istream& stream);
		wchar_t Convert(int value) const;
		int Convert(wchar_t value) const;
		std::wstring Convert(const std::string& value) const;
		std::string Convert(const std::wstring& value) const;
		std::unordered_map<int, wchar_t> m_forward;  // custom -> wchar_t
		std::unordered_map<wchar_t, int> m_backward; // wchar_t -> custom
	};

	CustomCodepage::CustomCodepage(std::istream& stream)
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
			else if (s.length() > 0 && try_parse<uint16_t>(s, result))
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
		return (i == m_forward.end())? 0xFFFD: i->second; // U+FFFD is a Unicode character replacement
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
			result[i] = (j == m_forward.end())? 0xFFFD: (wchar_t)j->second;
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

	// ------------------------------------------------------------------------

	struct UTF8Encoding: Encoding<char>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::string& value) const override;
		std::string Convert(const std::wstring& value) const override;
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
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::string UTF8Encoding::Convert(const std::wstring& value) const
	{
		std::string result;
		for (auto c: value) result += (char)c;
		return result;
	}

	std::unique_ptr<Encoding<char>> UTF8(new UTF8Encoding());

	// ------------------------------------------------------------------------

	struct UCS2Encoding: Encoding<char16_t>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::u16string& value) const override;
		std::u16string Convert(const std::wstring& value) const override;
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
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::u16string UCS2Encoding::Convert(const std::wstring& value) const
	{
		std::u16string result;
		for (auto c: value) result += (char16_t)c;
		return result;
	}

	std::unique_ptr<Encoding<char16_t>> UTF16(new UCS2Encoding());

	// ------------------------------------------------------------------------

	struct UCS4Encoding: Encoding<char32_t>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::u32string& value) const override;
		std::u32string Convert(const std::wstring& value) const override;
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
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::u32string UCS4Encoding::Convert(const std::wstring& value) const
	{
		std::u32string result;
		for (auto c: value) result += (char32_t)c;
		return result;
	}

	std::unique_ptr<Encoding<char32_t>> UTF32(new UCS4Encoding());

	// ------------------------------------------------------------------------

	std::unique_ptr<Encoding<char>> GetUnibyteEncoding(const std::wstring& name)
	{
		if (name == L"utf8")
		{
			return std::unique_ptr<Encoding<char>>(new UTF8Encoding());
		}
		else
		{
			auto stream = Resource::Open(name);
			if (!stream)
			{
				throw std::runtime_error("Failed to locate codepage resource for %name"); // FIXME: formatting
			}
			return std::unique_ptr<Encoding<char>>(new CustomCodepage(*stream));
		}
	}
}
