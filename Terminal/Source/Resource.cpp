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

#include "Resource.hpp"
#include "Encoding.hpp"
#include "Platform.hpp"
#include "Utility.hpp"
#include "Base64.hpp"
#include "Log.hpp"
#include <sstream>
#include <fstream>
#include <map>
#include <string.h>

namespace BearLibTerminal
{
	#include "Builtin.Codepages.hpp"
	#include "Builtin.DefaultFont.hpp"

	struct BuiltinResource
	{
		BuiltinResource(const std::string& data, bool base64encoded):
			data(data),
			base64encoded(base64encoded)
		{ }

		const std::string& data;
		bool base64encoded;
	};

	static std::map<std::wstring, BuiltinResource> kBuiltinResources =
	{
		{L"codepage-ascii", BuiltinResource(kCodepageASCII, false)},
		{L"codepage-437", BuiltinResource(kCodepage437, false)},
		{L"codepage-866", BuiltinResource(kCodepage866, false)},
		{L"codepage-1250", BuiltinResource(kCodepage1250, false)},
		{L"codepage-1251", BuiltinResource(kCodepage1251, false)},
		{L"tileset-default", BuiltinResource(kDefaultFont, true)},
		{L"codepage-tileset-default", BuiltinResource(kDefaultFontCodepage, false)},
		{L"codepage-tcod", BuiltinResource(kCodepageTCOD, false)}
	};

	struct MemoryResource
	{
		MemoryResource():
			address(0),
			size(0)
		{ }
	
		uint64_t address;
		size_t size;
	};

	bool try_parse(const std::wstring& s, MemoryResource& out)
	{
		size_t colon_pos = s.find(L":");
		if (colon_pos == std::wstring::npos)
			return false;

		if (!try_parse(s.substr(0, colon_pos), out.address))
			return false;

		if (!try_parse(s.substr(colon_pos+1), out.size))
			return false;

		return true;
	}

	std::vector<uint8_t> Resource::Open(std::wstring name, std::wstring prefix)
	{
		LOG(Debug, "Requested resource \"" << name << "\" with possible prefix \"" << prefix << "\"");

		auto i = kBuiltinResources.find(prefix + name);
		MemoryResource mem;

		if (i != kBuiltinResources.end())
		{
			LOG(Debug, "Resource \"" << prefix << name << "\" is built-in");

			if (i->second.base64encoded)
			{
				return Base64::Decode(i->second.data);
			}
			else
			{
				auto data = (const uint8_t*)i->second.data.data();
				return std::vector<uint8_t>(data, data + i->second.data.length());
			}
		}
		else if (try_parse(name, mem))
		{
			// Memory source.
			LOG(Debug, "Loading resource from memory '" << name << "'");
			std::vector<uint8_t> result(mem.size);
			memcpy(&result[0], (uint8_t*)mem.address, mem.size);
			return std::move(result);
		}
		else
		{
			return ReadFile(name);
		}
	}
}
