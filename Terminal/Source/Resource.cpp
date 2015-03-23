/*
 * Resource.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: cfyz
 */

#include "Resource.hpp"
#include "Encoding.hpp"
#include "Platform.hpp"
#include "Base64.hpp"
#include "Log.hpp"
#include <sstream>
#include <fstream>
#include <map>

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

	std::unique_ptr<std::istream> Resource::Open(std::wstring name, std::wstring prefix)
	{
		LOG(Debug, "Requested resource \"" << name << "\" with possible prefix \"" << prefix << "\"");
		auto i = kBuiltinResources.find(prefix+name);
		if (i != kBuiltinResources.end())
		{
			// Has built-in
			LOG(Debug, "Resource \"" << prefix << name << "\" is built-in");
			if (!i->second.base64encoded)
			{
				return std::unique_ptr<std::istream>(new std::istringstream(i->second.data));
			}
			else
			{
				auto decoded = Base64::Decode(i->second.data);
				return std::unique_ptr<std::istream>(new std::istringstream(std::string((char*)decoded.data(), decoded.size())));
			}
		}
		else
		{
			return OpenFileReading(name);
		}
	}
}
