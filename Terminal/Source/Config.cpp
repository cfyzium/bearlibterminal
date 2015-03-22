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
#include "BOM.hpp"
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
		LOG(Info, "Using configuration file '" << m_filename << "'");

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

	void Config::Load()
	{
		if (!FileExists(m_filename))
		{
			// No use trying to open nonexistent file.
			LOG(Info, L"Configuration file '" << m_filename << L"' does not exists, assuming empty config");
			return;
		}

		std::list<std::wstring> infile_lines;

		try
		{
			auto infile = OpenFileReading(m_filename);

			auto infile_bom = DetectBOM(*infile);
			LOG(Debug, "Configuration file encoding detected to be " << infile_bom);

			switch (infile_bom)
			{
			case BOM::None:
			case BOM::UTF8:
			case BOM::ASCII_UTF8:
				// Supported ones.
				break;
			default:
				// It will be no good to break user configuration file
				// by overwriting it in supported encoding.
				LOG(Error, "Unsupported configuration file encoding " << infile_bom);
				return;
			}

			// Consume BOM
			infile->ignore(GetBOMSize(infile_bom));

			std::string line;
			while (std::getline(*infile, line))
				infile_lines.push_back(UTF8Encoding().Convert(line));
		}
		catch (...)
		{
			// Cannot read the file, though it does exist.
			LOG(Error, L"Cannot open configuration file '" << m_filename << L"'");
			return;
		}

		// A name of the current section.
		std::wstring current_section;

		LOG(Trace, L"Configuration file contents:");
		for (auto& line: infile_lines)
		{
			if (line.empty() || std::isspace(line[0]))
			{
				continue; // Еmpty line or one staring with space.
			}
			else if (line[0] == ';' || line[0] == '#')
			{
				continue; // Comment
			}
			else if (line[0] == '[')
			{
				// Section header: "[name]"
				size_t rbracket = line.find(']');
				if (rbracket != std::wstring::npos)
				{
					// Save
					std::wstring name = line.substr(1, rbracket-1);
					LOG(Trace, L"[" << name << L"]");
					Section& section = m_sections[(current_section = L"ini." + name)];
				}
				else
				{
					// Reset
					current_section = L"";
				}
			}
			else if (!current_section.empty())
			{
				// Regular property, subname or grouped
				size_t pos = line.find_first_of(L"=.:");
				if (pos == std::wstring::npos || pos == 0)
				{
					// Malformed property line, ignore.
					continue;
				}

				Section& section = m_sections[current_section];

				std::wstring name = trim(line.substr(0, pos));
				line[pos] = L':';

				for (const auto& group: ParseOptions2(line, true))
				{
					for (auto& i: group.attributes)
					{
						std::wstring key = i.first;
						std::wstring value = i.second;

						key = key.empty()? name: (name + L"." + key);

						Property& property = section.m_properties[key];
						property.m_value = value;

						LOG(Trace, L"'" << key << L"' = '" << value << L"'");
					}
				}
			}
		}
	}

	bool Config::TryGet(std::wstring name, std::wstring& out)
	{
		std::lock_guard<std::mutex> guard(m_lock);

		if (!m_initialized)
			Init();

		if (name.empty())
			return false;

		if (!starts_with<wchar_t>(name, L"sys.") && !starts_with<wchar_t>(name, L"ini."))
			name = L"sys." + name;

		std::wstring section_name, property_name;

		const size_t domain_name_length = 4;
		size_t section_end_pos = name.find(L'.', domain_name_length+1);
		if (section_end_pos == std::wstring::npos)
		{
			// The name is in form 'ini.section'.
			section_name = name.substr(domain_name_length);
			property_name = L"";
		}
		else
		{
			section_name = name.substr(0, section_end_pos);
			property_name = name.substr(section_end_pos+1);
		}

		if (section_name.empty())
		{
			// Malformed name
			return false;
		}

		auto i = m_sections.find(section_name);
		if (i == m_sections.end())
		{
			// No such section.
			return false;
		}

		auto j = i->second.m_properties.find(property_name);
		if (j == i->second.m_properties.end())
		{
			// No such property.
			return false;
		}

		out = j->second.m_value;
		return true;
	}

	std::map<std::wstring, std::wstring> Config::List(const std::wstring& section)
	{
		std::map<std::wstring, std::wstring> result;
		std::lock_guard<std::mutex> guard(m_lock);
		auto i = m_sections.find(section);
		if (i != m_sections.end())
		{
			for (auto j: i->second.m_properties)
				result[j.first] = j.second.m_value;
		}
		return result;
	}

	void Config::Set(std::wstring name, std::wstring value)
	{
		std::lock_guard<std::mutex> gurad(m_lock);

		if (!m_initialized)
			Init();

		if (name.empty())
			return;

		bool ini_domain = starts_with<wchar_t>(name, L"ini.");

		// If this property does not belong to configuration, it is a system property.
		if (!ini_domain && !starts_with<wchar_t>(name, L"sys."))
			name = L"sys." + name;

		std::wstring section_name, property_name;

		const size_t domain_name_length = 4;
		size_t section_end_pos = name.find(L'.', domain_name_length+1);
		if (section_end_pos == std::wstring::npos)
		{
			// The name is in form 'ini.section'.
			section_name = name.substr(domain_name_length);
			property_name = L"";
		}
		else
		{
			section_name = name.substr(domain_name_length, section_end_pos-domain_name_length);
			property_name = name.substr(section_end_pos+1);
		}

		if (section_name.empty())
		{
			// Malformed name.
			return;
		}

		// Keep it in memory in any case.
		Section& section = m_sections[name.substr(0, domain_name_length)+section_name];
		Property& property = section.m_properties[property_name];
		property.m_value = value;

		if (ini_domain)
		{
			// Immediately update the file.
			Update(section_name, property_name, value);
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

		BOM infile_bom = BOM::None;
		std::list<std::string> infile_lines;

		if (FileExists(m_filename))
		{
			try
			{
				auto infile = OpenFileReading(m_filename);

				infile_bom = DetectBOM(*infile);
				LOG(Debug, "Configuration file encoding detected to be '" << infile_bom << "'");

				switch (infile_bom)
				{
				case BOM::None:
				case BOM::UTF8:
				case BOM::ASCII_UTF8:
					// Supported ones.
					break;
				default:
					// It will be no good to break user configuration file
					// by overwriting it in supported encoding.
					LOG(Error, "Unsupported configuration file encoding '" << infile_bom << "'");
					return;
				}

				// Consume BOM
				infile->ignore(GetBOMSize(infile_bom));

				// Read entire file line-by-line.
				std::string line;
				while(std::getline(*infile, line))
					infile_lines.push_back(line);
			}
			catch (std::exception& e)
			{
				LOG(Error, L"Cannot open configuration file '" << m_filename << L"' for reading");
				return;
			}
		}

		std::list<std::string> lines;
		auto si = lines.end(); // Iterator to last line in matching section
		auto pi = lines.end(); // Iterator to the found property line
		bool target_section = false;

		for (auto& line: infile_lines)
		{
			lines.push_back(line);

			if (line.empty() || std::isspace(line[0]))
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
				if (!ci_compare(name, property_name))
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

				// Overwriting name separator will make the line suitable for
				// universal parsing by ParseOptions function.
				line[pos] = ':';

				for (const auto& group: ParseOptions2(UTF8Encoding().Convert(line), true))
				{
					for (auto& i: group.attributes)
					{
						std::string subname = UTF8Encoding().Convert(i.first);
						std::string subvalue = UTF8Encoding().Convert(i.second);
						pieces[subname] = subvalue;
					}
				}
			}
		}

		// Overwrite
		pieces[piece_name] = piece_value;

		// Sanitize
		for (auto i = pieces.begin(); i != pieces.end();)
		{
			i->second.empty()? pieces.erase(i++): i++;
		}

		auto append_escaped = [](std::ostringstream& stream, const std::string& value)
		{
			if (value.find('\'') != std::string::npos ||
			    (!value.empty() && (std::isspace(value.front()) || std::isspace(value.back()))))
			{
				const char quote_mark = '\'';
				stream << quote_mark;
				for (auto c: value)
				{
					if (c == quote_mark)
						stream << quote_mark;
					stream << c;
				}
				stream << quote_mark;
			}
			else
			{
				stream << value;
			}
		};

		auto construct_line = [&]() -> std::string
		{
			std::ostringstream ss;
			ss << property_name;
			if (pieces.size() == 1 && pieces.begin()->first.empty())
			{
				// One entry and its subname is empty --> "foo=bar"
				ss << "=";
				append_escaped(ss, pieces.begin()->second);
			}
			else
			{
				// Miltiple-entry property --> "foo: bar, baz=doge"
				for (auto i = pieces.begin(); i != pieces.end(); i++)
				{
					ss << (i == pieces.begin()? ": ": ", ");
					if (i->first.empty())
					{
						append_escaped(ss, i->second);
					}
					else
					{
						ss << i->first << "=";
						append_escaped(ss, i->second);
					}
				}
			}
			return ss.str();
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
		try
		{
			auto outfile = OpenFileWriting(m_filename);

			// Only compatible ASCII/UTF-8 encodings are used.
			PlaceBOM(*outfile, infile_bom);

			for (auto& line: lines)
			{
				*outfile << line;
				if (uses_crlf && (line.empty() || line.back() != '\r'))
					*outfile << '\r';
				*outfile << '\n';
			}
		}
		catch (std::exception& e)
		{
			LOG(Error, L"Cannot open configuration file '" << m_filename << "' for writing");
			return;
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
