/*
 * OptionGroup.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: Cfyz
 */

#include "OptionGroup.hpp"

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
		//std::list<OptionGroup> result;
		//std::map<std::wstring, decltype(result)::iterator> lookup;

		//auto save_pair = [&](std::wstring& name, std::wstring& value)
		//{
		//	auto i = lookup.find(name);
		//	if (i == lookup.end())
		//	{
		//		OptionGroup
		//	}
		//};

		std::map<std::wstring, std::wstring> properties;

		const wchar_t* p = s.c_str();

		auto read_group_name = [&]
		{
			std::wstring name;
			while (*p != L'\0' && *p != L'=' && *p != L':')
				name += *p++;
			return name; // TODO: trim
		};

		auto skip_whitespace = [&]
		{
			while (*p == L' ' || *p == L'\t')
				p++;
		};

		auto read_until = [&](wchar_t c) -> std::wstring
		{
			std::wstring value;
			while (*p != L'\0' && *p != c)
				value += *p++; // FIXME: escaped characters
			return value;
		};

		auto read_quoted = [&](wchar_t c) -> std::wstring
		{
			wchar_t e = (c == L'[')? L']': c;
			return read_until(e);
		};

		auto read_regular = [&](std::wstring& name)
		{
			p++; // Skip '='

			skip_whitespace();

			if (*p == L'\0')
			{
				// EOS
				return;
			}
			else if (*p == L'\'' || *p == L'"' || *p == L'[')
			{
				// Quoted string
				properties[name] = read_quoted(*p);
			}
			else
			{
				// Raw string
				properties[name] = read_until(L';');
			}
		};

		auto read_pair = [&]() -> std::pair<std::wstring, std::wstring>
		{

		};

		auto read_grouped = [&](std::wstring& name)
		{
			p++; // Skip ':'

			skip_whitespace();

			//if (*p == L'\0')
		};

		while (*p != L'\0')
		{
			if (!std::isalpha(*p))
			{
				p++;
				continue;
			}

			std::wstring name = read_group_name(); // It will stop on either '\0', '=' or ':'

			if (*p == L'=')
			{
				// Regular name=value pair.
				read_regular(name);
			}
			else if (*p == L':')
			{
				// Grouped list of name=value pairs.
			}
			else
			{
				// Whatever.
				break;
			}
		}
	}
}
