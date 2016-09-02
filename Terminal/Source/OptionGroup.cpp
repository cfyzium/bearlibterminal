/*
* BearLibTerminal
* Copyright (C) 2013-2015 Cfyz
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

#include "OptionGroup.hpp"

#include "Encoding.hpp"
#include "Utility.hpp"
#include <iostream>

namespace BearLibTerminal
{
	static bool isquote(wchar_t c)
	{
		return c == L'\'' || c == L'"' || c == L'[' || c == '{';
	}

	static wchar_t get_closing_quote(wchar_t c)
	{
		switch (c)
		{
		case L'[':  return L']';
		case L'{':  return L'}';
		default:    return c;
		}
	}

	std::wstring read_until3(const wchar_t*& p, const std::wstring& until)
	{
		std::wstring value, space;
		wchar_t closing_quote = 0;

		while (*p != L'\0' && (closing_quote || until.find(*p) == std::wstring::npos))
		{
			if (*p == L'\n' || *p == L'\r')
			{
				// Just skip.
			}
			else if (std::isspace(*p) && !closing_quote)
			{
				// Accumulate space.
				space += *p;
			}
			else if (*p == closing_quote)
			{
				if (*(p+1) == closing_quote)
				{
					// Escaped quote
					value += *p++;
				}
				else
				{
					// End of quoted string.
					closing_quote = 0;
				}
			}
			else if (isquote(*p) && !closing_quote && value.empty())
			{
				// Start of quoted string.
				closing_quote = get_closing_quote(*p);
				space.clear();
			}
			else
			{
				// Release accumulated space.
				if (!value.empty())
					value += space;
				space.clear();

				// Append current symbol.
				value += *p;
			}

			// Advance
			p++;
		}

		return value;
	};

	std::list<OptionGroup> ParseOptions2(const std::wstring& s, bool semicolon_comments)
	{
		std::list<OptionGroup> result;
		std::map<std::wstring, decltype(result.begin())> lookup;

		const wchar_t* p = s.c_str();

		auto keep_property2 = [&](std::wstring name, std::wstring value)
		{
			// a       --> [a] ''       font.''
			// a.b     --> [a] b        window.title
			// a.b.c   --> [a.b] c      ini.custom.property
			// a.b.c.d --> [a.b] c.d    ini.bearlibterminal.window.title

			std::wstring section_name = L"_";

			size_t first_period_pos = name.find(L'.');
			if (first_period_pos == std::wstring::npos)
			{
				// Nameless property.
				section_name.swap(name);
			}
			else
			{
				size_t second_period_pos = name.find(L'.', first_period_pos+1);
				size_t section_end = (second_period_pos == std::wstring::npos)? first_period_pos: second_period_pos;

				if (section_end == 0 || section_end == name.length()-1)
				{
					// Surely invalid.
					return;
				}

				section_name = name.substr(0, section_end);
				name = name.substr(section_end+1);
			}

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
			auto name = read_until3(p, L":=;");

			if (*p == L'=')
			{
				// Regular property.
				p++;
				keep_property2(name, read_until3(p, L";"));
			}
			else if (*p == L':')
			{
				// Grouped property.
				while (*p != L'\0' && *p != L';')
				{
					p++;
					auto subname = read_until3(p, L"=,;");
					if (*p == L'=')
					{
						// Regular subproperty.
						p++;
						keep_property2(name + L"." + subname, read_until3(p, L",;"));
					}
					else if (*p == L',' || *p == L';' || *p == L'\0')
					{
						// Nameless subproperty (both continued or finale).
						keep_property2(name, subname);
					}
				}
			}
			else if (*p == L';')
			{
				if (semicolon_comments)
					break;

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
