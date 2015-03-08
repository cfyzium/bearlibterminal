/*
* BearLibTerminal
* Copyright (C) 2014 Cfyz
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

#ifndef BEARLIBTERMINAL_PLATFORM_HPP
#define BEARLIBTERMINAL_PLATFORM_HPP

#include <string>
#include <memory>
#include <list>

namespace BearLibTerminal
{
	class Module
	{
	public:
		typedef void* Handle;
		Module();
		Module(std::wstring name);
		Module(Module&& from);
		explicit Module(Handle handle);
		~Module();
		Handle GetHandle() const;
		Module& operator=(Module&& from);
		void* operator[](std::string name) const;
		explicit operator bool() const;
		void* Probe(std::string name) const;
		static Module GetProviding(std::string name);

	private:
		Module(const Module&);
		Module& operator=(const Module&);
		Handle m_handle;
		bool m_owner;
	};

	std::unique_ptr<std::istream> OpenFileReading(std::wstring name);

	std::unique_ptr<std::ostream> OpenFileWriting(std::wstring name);

	std::wstring GetEnvironmentVariable(const std::wstring& name);

	bool FileExists(std::wstring name);

	std::wstring GetAppName();

	std::wstring GetAppDirectory();

	std::wstring GetCurrentDirectory();

	std::list<std::wstring> EnumerateFiles(std::wstring path);
}

#endif // BEARLIBTERMINAL_PLATFORM_HPP
