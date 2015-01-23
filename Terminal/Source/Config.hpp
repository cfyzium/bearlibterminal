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

namespace BearLibTerminal
{
	class Config
	{
	public:
		bool TryGet(std::wstring name, std::wstring& out);
		void Set(std::wstring name, std::wstring value);
		void Remove(std::wstring name);
		void Dispose();

	public:
		static Config& Instance();

	private:
		Config();
		void Init();
		void Load(std::wstring filename);

	private:
		struct Property
		{
			std::wstring m_case_sensitive_name;
			std::wstring m_value;
		};

		struct Section
		{
			std::wstring m_case_sensitive_name;
			std::map<std::wstring, Property> m_properties;
		};

	private:
		std::mutex m_lock;
		bool m_initialized;
		std::wstring m_filename;
		std::map<std::wstring, Section> m_sections;
	};
}

#endif // BEARLIBTERMINAL_CONFIGURATION_HPP
