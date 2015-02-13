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

#include "Platform.hpp"
#include "Utility.hpp"
#include "Encoding.hpp"
#include "OptionGroup.hpp"

#include <iostream>
#include <fstream>
#include "Config.hpp"

namespace BearLibTerminal
{
	namespace // anonymous
	{
		struct property_line
		{
			size_t offset;
			size_t length;
		};
	}

	Config::Config():
		m_initialized(false)
	{
		Init();
	}

	void Config::Init()
	{
		Load(L"./bearlibterminal.ini");
		m_initialized = true;
	}

	void Config::Load(std::wstring filename)
	{
		std::cout << "ConfigFile::Init()\n";

		// TODO: choose filename from available files: (bearlibterminal|appname|config).(ini|cfg|conf)
		m_filename = filename;

		auto stream = OpenFile(m_filename);

		// A name of the current section.
		std::wstring current_section;

		std::string line;
		while (std::getline(*stream, line))
		{
			line = trim(line);

			if (line.empty())
			{
				continue; // Well, empty line
			}
			else if (line[0] == ';' || line[0] == '#')
			{
				continue; // Comment
			}
			else if (line[0] == '[')
			{
				// Section header: "[name]"
				size_t rbracket = line.find(']');
				if (rbracket == std::string::npos || rbracket == 1)
				{
					current_section = L"";
					continue; // Malformed section header
				}

				// Save
				std::wstring section_name = UTF8Encoding().Convert(line.substr(1, rbracket-1));
				current_section = L"ini." + to_lower(section_name);
				Section& section = m_sections[current_section];
				section.m_case_sensitive_name = section_name;
				std::cout << "Got a new section: \"" << UTF8Encoding().Convert(current_section) << "\" (\"" << UTF8Encoding().Convert(section_name) << "\")\n";
				continue;
			}
			else if (!current_section.empty())
			{
				size_t pos = line.find_first_of("=:");
				if (pos == std::string::npos)
					continue; // Malformed property line: no equals or colon sign

				Section& section = m_sections[current_section];
				if (line[pos] == ':')
				{
					// Grouped property
					for (auto group: ParseOptions(UTF8Encoding().Convert(line)))
					{
						for (auto& attribute: group.attributes)
						{
							std::wstring key = group.name + L"." + attribute.first;
							std::wstring case_insensitive_key = to_lower(key);
							Property& property = section.m_properties[case_insensitive_key];
							property.m_case_sensitive_name = key;
							property.m_value = attribute.second;
							std::cout << "Got a new property: \"" << UTF8Encoding().Convert(case_insensitive_key) << "\" (\"" << UTF8Encoding().Convert(key) << "\") = \"" << UTF8Encoding().Convert(attribute.second) << "\"\n";
						}
					}
				}
				else
				{
					// Regular property
					if (pos == line.length()-1)
						continue; // Malformed property line

					std::wstring key = UTF8Encoding().Convert(trim(line.substr(0, pos)));
					std::wstring value = UTF8Encoding().Convert(trim(line.substr(pos+1)));

					if (key.empty() || value.empty())
						continue; // Malformed property line

					std::wstring case_insensitive_key = to_lower(key);
					Property& property = section.m_properties[case_insensitive_key];
					property.m_case_sensitive_name = key;
					property.m_value = value;
					std::cout << "Got a new property: \"" << UTF8Encoding().Convert(case_insensitive_key) << "\" (\"" << UTF8Encoding().Convert(key) << "\") = \"" << UTF8Encoding().Convert(value) << "\"\n";
				}
			}
		}
	}

	bool Config::TryGet(std::wstring name, std::wstring& out)
	{
		std::lock_guard<std::mutex> gurad(m_lock);

		if (!m_initialized)
			Init();

		std::cout << "Trying to get \"" << UTF8Encoding().Convert(name) << "\"\n";

		const size_t domain_name_length = 4;

		if (name.empty())
		{
			return false;
		}

		if (!starts_with(name, std::wstring(L"sys.")) && !starts_with(name, std::wstring(L"ini.")))
		{
			name = L"sys." + name;
		}

		size_t section_end_pos = name.find(L'.', domain_name_length+1);
		if (section_end_pos == std::wstring::npos)
		{
			return false; // Querying section value is not supported.
		}

		if (section_end_pos == domain_name_length+1)
		{
			return false; // Malformed section name.
		}

		if (section_end_pos == name.length()-1)
		{
			return false; // Malformed property name.
		}

		std::wstring section = to_lower(name.substr(0, section_end_pos));
		std::wstring property = to_lower(name.substr(section_end_pos+1));

		//auto config = g_Configuration.Instance();

		auto i = m_sections.find(section);
		if (i == m_sections.end())
		{
			return false; // No such section.
		}

		auto j = i->second.m_properties.find(property);
		if (j == i->second.m_properties.end())
		{
			return false; // No such property.
		}

		out = j->second.m_value;
		std::cout << "Got \"" << UTF8Encoding().Convert(out) << "\" for \"" << UTF8Encoding().Convert(name) << "\"\n";
		return true;
	}

	void Config::Set(std::wstring name, std::wstring value)
	{
		std::lock_guard<std::mutex> gurad(m_lock);

		if (!m_initialized)
			Init();

		std::cout << "Trying to set \"" << UTF8Encoding().Convert(name) << "\" to \"" << UTF8Encoding().Convert(value) << "\"\n";

		if (name.empty())
		{
			return;
		}

		const size_t domain_name_length = 4;
		bool ini_domain = false; // TODO: enum ftw

		if (starts_with(name, std::wstring(L"ini.")))
		{
			ini_domain = true;
		}
		else if (!starts_with(name, std::wstring(L"sys.")))
		{
			name = L"sys." + name;
		}

		size_t section_end_pos = name.find(L'.', domain_name_length+1);
		if (section_end_pos == std::wstring::npos)
		{
			return; // Querying section value is not supported.
		}

		if (section_end_pos == domain_name_length+1)
		{
			return; // Malformed section name.
		}

		if (section_end_pos == name.length()-1)
		{
			return; // Malformed property name.
		}

		std::wstring section_cs = name.substr(0, section_end_pos);
		std::wstring section = to_lower(section_cs);
		std::wstring property_cs = name.substr(section_end_pos+1);
		std::wstring property = to_lower(property_cs);

		// Keep it in memory anyway
		auto i = m_sections.find(section);
		if (i == m_sections.end())
		{
			i = m_sections.insert({section, Section()}).first;
			i->second.m_case_sensitive_name = section_cs;
		}
		auto j = i->second.m_properties.find(property);
		if (j == i->second.m_properties.end())
		{
			j = i->second.m_properties.insert({property, Property()}).first;
			j->second.m_case_sensitive_name = property_cs;
		}
		j->second.m_value = value;

		if (ini_domain)
		{
			// Immediately update the file
			// FIXME: NYI

			// 1. Open the file
			// 2. Find the line this property is at.
			// 3. Replace the property value in the line, trying to keep comments intact
			// -or-
			// 2. Find the line the section ends.
			// 3. Add the property to the end of the section.
			// -or-
			// 3. Add the section with such property to the end of file.
			//
			// !!!
			// Several subnames must be grouped to one:
			// foo.bar=1, foo.baz=2 --> foo: bar=1, baz=2
			//
			// do.se.name.x=10;
			// adding do.se.name.y=20 --> [se]name: x=10, y=20
			//
			// [section]
			// foo.bar=1
			// foo.baz=2
			//

			std::string name8 = UTF8Encoding().Convert(property_cs);
			std::string value8 = UTF8Encoding().Convert(value);
			std::string section8 = UTF8Encoding().Convert(section_cs);

			std::map<std::string, std::string> pieces;

			std::string subname = "";
			size_t period_pos = name8.find(".");
			if (period_pos != std::string::npos)
			{
				std::string subname = name8.substr(period_pos+1);
				name8 = name8.substr(0, period_pos);
			}
			pieces[subname] = value8;



			std::ifstream file(UTF8Encoding().Convert(m_filename).c_str(), std::ios_base::in); // XXX: linux version
			if (!file.is_open())
			{
				std::cerr << "Failed to open \"" + UTF8Encoding().Convert(m_filename) + "\"\n";
				return;
			}

			std::list<std::string> lines;
			auto si = lines.end();
			auto pi = lines.end();

			std::string current_section;

			std::string line;
			while(std::getline(file, line))
			{
				lines.push_back(line);

				if (line.empty() || line[0] == ' ' || line == '\t')
				{
					// Must not be counted as belonging to the section
				}
				else if (line[0] == ';' || line[0] == '#')
				{
					// Comment line
				}
				else if (line[0] == '[')
				{
					// Section header
					size_t rbracket_pos = line.find(']');
					if (rbracket_pos != std::string::npos)
					{
						// Looks okay
						current_section = line.substr(1, rbracket_pos-1);
					}
					else
						continue; // ?
				}
				else
				{
					// Must be a property
					size_t pos = line.find_first_of("=.:");
					if (pos != std::string::npos)
					{
						// Name legible
						std::string name = line.substr(0, pos); // trim?
						if (strcasecmp(name8.c_str(), name.c_str()) == 0)
						{
							// That's what we're looking for
							if (line[pos] != '=')
							{
								// Grouped property
								line[pos] = ':';
								for (auto group: ParseOptions(UTF8Encoding().Convert(line))) // TODO: ParseValues
								{
									for (auto i: group.attributes)
									{
										pieces[UTF8Encoding().Convert(i.first)] = UTF8Encoding().Convert(i.second);
									}
								}
							}

							if (pi == lines.end())
							{
								// Remember first location.
								pi = (--lines.end());
							}
							else
							{
								// Remove duplicates
								lines.pop_back();
							}
						}
					}
				}

				if (strcasecmp(section8.c_str(), current_section.c_str()) == 0)
				{
					si = (--lines.end());
				}
			}

			// Overwrite property value
			pieces[subname] = value8;

			if (pi != lines.empty())
			{
				// Property has been found, overwrite it.
				if (pieces.size() == 1 && pieces.begin()->first.empty()) // TODO: lambda-construct property line
				{
					// Regular property: foo=bar
				}
				else
				{
					// Grouped property: foo: bar, baz=doge
				}
			}
			else if (si != lines.empty())
			{
				// Only section was found, append.
			}
			else
			{
				// Brand new entry
			}

			/*
			std::list<property_line> property_lines;
			std::string line;
			size_t line_offset = 0;
			size_t section_end = std::string::npos;
			std::wstring current_section;

			for (; std::getline(file, line); line_offset = file.tellg())
			{
				auto trimmed_line = trim(line);

				if (trimmed_line.empty())
				{
					continue; // Well, empty line
				}
				else if (trimmed_line[0] == ';' || trimmed_line[0] == '#')
				{
					continue; // Comment
				}
				else if (trimmed_line[0] == '[')
				{
					// Section header: "[name]"
					size_t rbracket = line.find(']');
					if (rbracket == std::string::npos || rbracket == 1)
					{
						current_section = L"";
						continue; // Malformed section header
					}

					// Keep
					current_section = to_lower(UTF8Encoding().Convert(line.substr(1, rbracket-1)));
					if (current_section == section)
					{
						section_end = file.tellg();
					}
				}
				else if (current_section == section)
				{
					// Property line

				}
			}
			//*/
		}
	}

	void Config::Remove(std::wstring name)
	{
		// If ini domain, immediately remove the property or a section from file
		// FIXME: NYI
	}

	void Config::Dispose()
	{
		std::lock_guard<std::mutex> gurad(m_lock);
		m_sections.clear();
		m_filename.clear();
		m_initialized = false;
	}

	Config& Config::Instance()
	{
		static Config instance;
		return instance;
	}
}
