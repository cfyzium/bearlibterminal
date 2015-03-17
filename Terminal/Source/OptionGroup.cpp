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
			else if ((*p == L'\'' || *p == L'"' || *p == L'[') && !closing_quote)
			{
				// Start of quoted string.
				closing_quote = (*p == L'['? L']': *p);
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

	std::list<OptionGroup> ParseOptions2(const std::wstring& s)
	{
		std::list<OptionGroup> result;
		std::map<std::wstring, decltype(result.begin())> lookup;

		const wchar_t* p = s.c_str();

		auto keep_property2 = [&](std::wstring name, std::wstring value)
		{
			if (name.empty())
			{
				// Ignore empty names. Empty value, on the other hand, triggers removal.
				return;
			}

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
