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

#ifndef BEARLIBTERMINAL_CONFIGURATION_HPP
#define BEARLIBTERMINAL_CONFIGURATION_HPP

#include <map>
#include <string>
#include <mutex>
#include <algorithm>

namespace BearLibTerminal
{
	template<typename char_t> struct ci_less: // FIXME: --> Utility
		std::binary_function<std::basic_string<char_t>, std::basic_string<char_t>, bool>
	{
		struct nocase_compare:
			public std::binary_function<char_t, char_t, bool>
		{
			bool operator()(const char_t& c1, const char_t& c2) const
			{
				return std::tolower(c1) < std::tolower(c2);
			}
		};

		bool operator()(const std::basic_string<char_t>& s1, const std::basic_string<char_t>& s2) const
		{
			return std::lexicographical_compare // XXX: simple strcasecmp?
				(
					s1.begin(), s1.end(),
					s2.begin(), s2.end(),
					nocase_compare()
				);
		}
	};

	class Config
	{
	public:
		bool TryGet(std::wstring name, std::wstring& out);
		void Set(std::wstring name, std::wstring value);
		void Dispose();
		static Config& Instance();

	private:
		Config();
		void Init();
		std::wstring GuessConfigFilename();
		void Load();
		void Update(std::wstring section, std::wstring property, std::wstring value);

		struct Property
		{
			std::wstring m_case_sensitive_name;
			std::wstring m_value;
		};

		struct Section
		{
			std::wstring m_case_sensitive_name;
			std::map<std::wstring, Property, ci_less<wchar_t>> m_properties;
		};

		std::mutex m_lock;
		bool m_initialized;
		std::wstring m_filename;
		std::map<std::wstring, Section, ci_less<wchar_t>> m_sections;
	};
}

#endif // BEARLIBTERMINAL_CONFIGURATION_HPP
