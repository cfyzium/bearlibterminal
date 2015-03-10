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
#include "Config.hpp"
#include "Log.hpp"
#include <iostream>
#include <fstream>

namespace BearLibTerminal
{
	Config::Config():
		m_initialized(false)
	{
		Init();
	}

	void Config::Init()
	{
		m_filename = GuessConfigFilename();
		Load();
		m_initialized = true;
	}

	std::wstring Config::GuessConfigFilename()
	{
		std::wstring preferred_name = GetEnvironmentVariable(L"BEARLIB_INIFILE");
		// TODO: check if this is a fully qualified file name.

		// File name matching application name.
		std::wstring appconfig_name = GetAppName() + L".ini";

		int best_priority = 0;
		std::wstring best_filename;

		auto search_v2 = [&](std::wstring path, int priority_factor)
		{
			for (auto& file: EnumerateFiles(path))
			{
				int priority = 0;

				if (ci_compare(file, preferred_name))
				{
					priority = 3;
				}
				if (ci_compare(file, appconfig_name))
				{
					priority = 2;
				}
				else if (ends_with<wchar_t>(file, L".ini"))
				{
					priority = 1;
				}
				else
				{
					continue;
				}

				priority *= priority_factor;

				if (priority > best_priority)
				{
					best_filename = path + file;
					best_priority = priority;
				}
			}
		};

		search_v2(GetCurrentDirectory(), 2);
		search_v2(GetAppDirectory(), 1);

		if (best_filename.empty())
			best_filename = preferred_name.empty()? appconfig_name: preferred_name;

		return best_filename;
	}

	void Config::Load() // FIXME: does not parse the same way Update() does.
	{
		if (!FileExists(m_filename))
		{
			// No use trying to open nonexistent file.
			LOG(Info, L"Config::Load: file '" << m_filename << L"' does not exists, assuming empty config");
			return;
		}

		std::unique_ptr<std::istream> stream;
		try
		{
			stream = OpenFileReading(m_filename);
		}
		catch (...)
		{
			// Cannot read the file, though it does exist.
			LOG(Error, L"Config::Load: cannot open file '" << m_filename << L"'");
			return;
		}

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
					for (auto group: ParseOptions2(UTF8Encoding().Convert(line)))
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
		std::lock_guard<std::mutex> guard(m_lock);

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

		std::wstring section = name.substr(domain_name_length, section_end_pos-domain_name_length);
		std::wstring property = name.substr(section_end_pos+1);

		// Keep it in memory anyway
		auto i = m_sections.find(section);
		if (i == m_sections.end())
		{
			i = m_sections.insert({section, Section()}).first;
			i->second.m_case_sensitive_name = section;
		}
		auto j = i->second.m_properties.find(property);
		if (j == i->second.m_properties.end())
		{
			j = i->second.m_properties.insert({property, Property()}).first;
			j->second.m_case_sensitive_name = property;
		}
		j->second.m_value = value;

		if (ini_domain)
		{
			// Immediately update the file.
			Update(section, property, value);
		}

	}

	void Config::Update(std::wstring section, std::wstring property, std::wstring value)
	{
		// Convert to UTF-8
		std::string section_name = UTF8Encoding().Convert(section);
		std::string property_name = UTF8Encoding().Convert(property);
		std::string piece_value = UTF8Encoding().Convert(value);

		// Prepare property value for merging
		std::map<std::string, std::string> pieces;
		std::string piece_name;
		{
			size_t period_pos = property_name.find(".");
			if (period_pos != std::string::npos)
			{
				piece_name = property_name.substr(period_pos+1);
				property_name = property_name.substr(0, period_pos);
			}
		}

		std::unique_ptr<std::istream> infile;
		try
		{
			infile = OpenFileReading(m_filename);
		}
		catch (std::exception& e)
		{
			LOG(Error, L"Config::Update: cannot open file '" << m_filename << L"' for reading");
			return;
		}
		// TODO: BOM matching.

		std::list<std::string> lines;
		auto si = lines.end(); // Iterator to last line in matching section
		auto pi = lines.end(); // Iterator to the found property line

		std::string line;
		bool target_section = false;

		while(std::getline(*infile, line))
		{
			lines.push_back(line);

			if (line.empty() || line[0] == ' ' || line[0] == '\t')
			{
				// Empty line or one starting with whitespace.
				continue;
			}
			else if (line[0] == ';' || line[0] == '#')
			{
				// Comment line.
				continue;
			}
			else if (line[0] == '[')
			{
				// Section header
				size_t rbracket_pos = line.find(']');
				if (rbracket_pos != std::string::npos)
				{
					std::string name = line.substr(1, rbracket_pos-1);
					target_section = ci_compare(name, section_name);
					if (target_section && si == lines.end())
					{
						// Keep section position in mind, even it is only a header.
						si = (--lines.end());
					}
				}

				continue;
			}

			// Now it looks like a property line
			if (target_section)
			{
				// It's okay to keep iterator to this line despite the fact it may be removed
				// from the list later. Only matching property duplicates are removed and by
				// that time property line iterator (pi) is already set.
				si = (--lines.end());

				size_t pos = line.find_first_of("=.:"); // Regular property, subname or grouped
				if (pos == std::string::npos || pos == 0)
				{
					// Malformed property line, ignore.
					continue;
				}

				std::string name = trim(line.substr(0, pos));
				if (ci_compare(name, property_name))
				{
					// Not the property we are searching for.
					continue;
				}

				if (pi == lines.end())
				{
					// If the first section.property match.
					pi = (--lines.end());

					// Try to preserve original casing.
					property_name = name;
				}
				else
				{
					// Remove duplicates.
					lines.pop_back();
				}

				line[pos] = ':';

				// TODO: lightweight parse version
				// TODO: utf8-friendly
				for (const auto& group: ParseOptions2(UTF8Encoding().Convert(line)))
				{
					for (auto& i: group.attributes)
					{
						std::string subname = UTF8Encoding().Convert(i.first);
						std::string subvalue = UTF8Encoding().Convert(i.second);

						if (subname == "name")
							subname = ""; // FIXME: use empty attribute name consistently

						pieces[subname] = subvalue;
					}
				}
			}
		}

		// Done reading
		infile.reset();

		// Overwrite
		pieces[piece_name] = piece_value;

		// Sanitize
		for (auto i = pieces.begin(); i != pieces.end();)
		{
			i->second.empty()? pieces.erase(i++): i++;
		}

		auto construct_line = [&]() -> std::string
		{
			if (pieces.size() == 1 && pieces.begin()->first.empty())
			{
				// One entry and its subname is empty --> "foo=bar"
				return property_name + "=" + pieces.begin()->second; // TODO: escape values
			}
			else
			{
				// Miltiple-entry property --> "foo: bar, baz=doge"
				std::ostringstream ss;
				ss << property_name;
				for (auto i = pieces.begin(); i != pieces.end(); i++)
				{
					ss << (i == pieces.begin()? ": ": ", ");
					if (i->first.empty())
						ss << i->second;
					else
						ss << i->first << "=" << i->second; // TODO: escape values
				}
				return ss.str();
			}
		};

		if (pieces.empty())
		{
			// Remove whole property line, if found. If not found there is nothing to remove.
			if (pi != lines.end())
				lines.erase(pi);
		}
		else
		{
			// Replace exising or add new property line. We do know it is not empty.
			if (pi != lines.end())
			{
				// Exact property line.
				(*pi) = construct_line();
			}
			else if (si != lines.end())
			{
				// Section line.
				lines.insert(++si, construct_line());
			}
			else
			{
				// Add new section to the end of file.
				if (!lines.empty() && !trim(lines.back()).empty())
					lines.emplace_back();
				lines.emplace_back("[" + section_name + "]");
				lines.emplace_back(construct_line());
			}
		}

		// Look at the first line to determine line endings used.
		// std::getline removes '\n' but leaves '\r'.
		bool uses_crlf = (!lines.front().empty() && lines.front().back() == '\r');

		// Now, write back to file
		std::unique_ptr<std::ostream> outfile;
		try
		{
			outfile = OpenFileWriting(m_filename);
		}
		catch (std::exception& e)
		{
			LOG(Error, L"Config::Update: cannot open file '" << m_filename << "' for writing");
			return;
		}

		for (auto& line: lines)
		{
			*outfile << line;
			if (uses_crlf && (line.empty() || line.back() != '\r'))
				*outfile << '\r';
			*outfile << '\n';
		}
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
