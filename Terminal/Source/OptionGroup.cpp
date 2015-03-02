/*
 * OptionGroup.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: Cfyz
 */

#include "OptionGroup.hpp"

#include "Encoding.hpp"
#include "Utility.hpp"
#include <iostream>

namespace BearLibTerminal
{
	enum class HighState
	{
		Group,
		Attribute,
		Value
	};

	enum class LowState
	{
		Whitespace,
		Token,
		String
	};

	static bool IsWhitespace(wchar_t c)
	{
	    return c == L' ' || c == L'\t';
	}

	static bool IsGroupSeparator(wchar_t c)
	{
	    return c == L'.' || c == L':';
	}

	static bool IsTokenSeparator(wchar_t c)
	{
	    return c == L',' || c == L';' || c == L'=';
	}

	static bool IsQuote(wchar_t c)
	{
	    return c == L'\'' || c == L'"' || c == L'[';
	}

	static wchar_t ClosingQuoteCharacter(wchar_t opening)
	{
		static std::map<wchar_t, wchar_t> mapping =
		{
			{L'\'', L'\''},
			{L'"', L'"'},
			{L'[', L']'}
		};

		return mapping[opening];
	}

	std::list<OptionGroup> ParseOptions(const std::wstring& s)
	{
		std::list<OptionGroup> result;
		std::map<std::wstring, decltype(result.begin())> lookup;

		std::wstring group_name;
		std::wstring attr_name;
		std::wstring token;
		std::wstring top;
		HighState high_state = HighState::Group;
		LowState low_state = LowState::Token;
		wchar_t quote_character;

		auto save_attribute = [&](const std::wstring& name, const std::wstring& value)
		{
			auto i = lookup.find(group_name);
			if (i == lookup.end())
			{
				result.emplace_back();
				result.back().name = group_name;
				i = lookup.insert({group_name, --result.end()}).first;
			}
			i->second->attributes[name] = value;
		};

		auto process_token = [&](const std::wstring& value)
		{
			switch (high_state)
			{
			case HighState::Group:
				if (value == L"." || value == L":")
				{
					group_name = top;
					high_state = HighState::Attribute;
					return;
				}
				break;

			case HighState::Attribute:
				if (value == L"," || value == L";")
				{
					save_attribute(L"name", top);
					if (value == L";") high_state = HighState::Group;
					return;
				}
				else if (value == L"=")
				{
					attr_name = top;
					high_state = HighState::Value;
					return;
				}
				break;

			case HighState::Value:
				if (value == L"," || value == L";")
				{
					save_attribute(attr_name, top);
					high_state = value == L";"? HighState::Group: HighState::Attribute;
					return;
				}
				break;
			}

			top = value;
		};

		auto process_char = [&](wchar_t c)
		{
			switch (low_state)
			{
			case LowState::Whitespace:
				if (!IsWhitespace(c))
				{
					low_state = LowState::Token;
				}
				else
				{
					break;
				}

			case LowState::Token:
				if (IsWhitespace(c))
				{
					low_state = LowState::Whitespace;
				}
				else if (IsQuote(c))
				{
					low_state = LowState::String;
					quote_character = ClosingQuoteCharacter(c);
				}
				else if ((IsGroupSeparator(c) && high_state == HighState::Group) || IsTokenSeparator(c))
				{
					process_token(token);
					process_token(std::wstring(1, c));
					token = L"";
				}
				else
				{
					token += c;
				}
				break;

			case LowState::String:
				if (c == quote_character)
				{
					low_state = LowState::Token;
				}
				else
				{
					token += c;
				}
				break;
			}
		};

		for (wchar_t c: s) process_char(c);

		process_char(L';'); // Flush

		return result;
	}

	std::list<OptionGroup> ParseOptions2(const std::wstring& s)
	{
		std::list<OptionGroup> result;
		std::map<std::wstring, decltype(result.begin())> lookup;

		const wchar_t* p = s.c_str();

		auto read_until2 = [&](std::wstring s) -> std::wstring
		{
			std::wstring value, space;
			wchar_t closing_quote = 0;

			while (*p != L'\0' && (closing_quote || s.find(*p) == std::wstring::npos))
			{
				if (std::isspace(*p) && !closing_quote)
				{
					// Accumulate space.
					space += *p++;
				}
				else if (*p == closing_quote)
				{
					// End of quoted string.
					closing_quote = 0;
					p++;
					continue;
				}
				else if (*p == L'\'' || *p == L'"' || *p == L'[')
				{
					// Start of quoted string.
					closing_quote = (*p == L'['? L']': *p);
					space.clear();
					p++;
					continue;
				}
				else
				{
					// Handle escaped characters.
					if (*p == L'\\' && *(++p) == L'\0')
						break;

					// Release accumulated space.
					if (!value.empty())
						value += space;
					space.clear();

					// Append current symbol.
					value += *p++;
				}
			}

			return value;
		};

		auto keep_property2 = [&](std::wstring name, std::wstring value)
		{
			if (name.empty() || value.empty())
			{
				// Ignore empty ones.
				return;
			}

			//std::cout << "*** '" << UTF8Encoding().Convert(name) << "' = '" << UTF8Encoding().Convert(value) << "'\n";

			// a       --> invalid?
			// a.b     --> [a] b        window.title
			// a.b.c   --> [a.b] c      ini.custom.property
			// a.b.c.d --> [a.b] c.d    ini.bearlibterminal.window.title

			size_t first_period_pos = name.find(L'.');
			if (first_period_pos == std::wstring::npos)
			{
				// Consider it invalid for now.
				return;
			}

			size_t second_period_pos = name.find(L'.', first_period_pos+1);
			size_t section_end = (second_period_pos == std::wstring::npos)? first_period_pos: second_period_pos;

			if (section_end == 0 || section_end == name.length()-1)
			{
				// Surely invalid.
				return;
			}

			std::wstring section_name = name.substr(0, section_end);
			name = name.substr(section_end+1);

			auto i = lookup.find(section_name);
			if (i == lookup.end())
			{
				result.emplace_back();
				i = lookup.insert({section_name, (--result.end())}).first;
				i->second->name = section_name;
			}
			i->second->attributes[name] = value;
		};

		while (*p != L'\0')
		{
			// Next property name.
			auto name = read_until2(L":=;");

			if (*p == L'=')
			{
				// Regular property.
				p++;
				keep_property2(name, read_until2(L";"));
			}
			else if (*p == L':')
			{
				// Grouped property.
				while (*p != L'\0' && *p != L';')
				{
					p++;
					auto subname = read_until2(L"=,;");
					if (*p == L'=')
					{
						// Regular subproperty.
						p++;
						keep_property2(name + L"." + subname, read_until2(L",;"));
					}
					else if (*p == L',' || *p == L';' || *p == L'\0')
					{
						// Nameless subproperty (both continued or finale).
						keep_property2(name + L".name", subname);
					}
				}
			}
			else if (*p == L';')
			{
				// Discard.
				p++;
			}
			else
			{
				// Whatever.
				break;
			}
		}

		return result;
	}
}
