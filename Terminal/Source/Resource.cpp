/*
 * Resource.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: cfyz
 */

#include "Resource.hpp"
#include "Encoding.hpp"
#include <sstream>
#include <fstream>
#include <map>

namespace BearLibTerminal
{
	static const std::string kCodepageASCII =
	"U+0000-U+007F";

	static const std::string kCodepage437 = // OEM 437
	"U+0000-U+007F,"
	"U+00C7, U+00FC, U+00E9, U+00E2, U+00E4, U+00E0, U+00E5, U+00E7,"
	"U+00EA, U+00EB, U+00E8, U+00EF, U+00EE, U+00EC, U+00C4, U+00C5,"
	"U+00C9, U+00E6, U+00C6, U+00F4, U+00F6, U+00F2, U+00FB, U+00F9,"
	"U+00FF, U+00D6, U+00DC, U+00A2, U+00A3, U+00A5, U+20A7, U+0192,"
	"U+00E1, U+00ED, U+00F3, U+00FA, U+00F1, U+00D1, U+00AA, U+00BA,"
	"U+00BF, U+2310, U+00AC, U+00BD, U+00BC, U+00A1, U+00AB, U+00BB,"
	"U+2591, U+2592, U+2593, U+2502, U+2524, U+2561, U+2562, U+2556,"
	"U+2555, U+2563, U+2551, U+2557, U+255D, U+255C, U+255B, U+2510,"
	"U+2514, U+2534, U+252C, U+251C, U+2500, U+253C, U+255E, U+255F,"
	"U+255A, U+2554, U+2569, U+2566, U+2560, U+2550, U+256C, U+2567,"
	"U+2568, U+2564, U+2565, U+2559, U+2558, U+2552, U+2553, U+256B,"
	"U+256A, U+2518, U+250C, U+2588, U+2584, U+258C, U+2590, U+2580,"
	"U+03B1, U+00DF, U+0393, U+03C0, U+03A3, U+03C3, U+00B5, U+03C4,"
	"U+03A6, U+0398, U+03A9, U+03B4, U+221E, U+03C6, U+03B5, U+2229,"
	"U+2261, U+00B1, U+2265, U+2264, U+2320, U+2321, U+00F7, U+2248,"
	"U+00B0, U+2219, U+00B7, U+221A, U+207F, U+00B2, U+25A0, U+00A0";

	static const std::string kCodepage1251 = // Windows Cyrillic
	"U+0000-U+007F,"
	"U+0402, U+0403, U+201A, U+0453, U+201E, U+2026, U+2020, U+2021,"
	"U+20AC, U+2030, U+0409, U+2039, U+040A, U+040C, U+040B, U+040F,"
	"U+0452, U+2018, U+2019, U+201C, U+201D, U+2022, U+2013, U+2014,"
	"U+003F, U+2122, U+0459, U+203A, U+045A, U+045C, U+045B, U+045F,"
	"U+00A0, U+040E, U+045E, U+0408, U+00A4, U+0490, U+00A6, U+00A7,"
	"U+0401, U+00A9, U+0404, U+00AB, U+00AC, U+00AD, U+00AE, U+0407,"
	"U+00B0, U+00B1, U+0406, U+0456, U+0491, U+00B5, U+00B6, U+00B7,"
	"U+0451, U+2116, U+0454, U+00BB, U+0458, U+0405, U+0455, U+0457,"
	"U+0410, U+0411, U+0412, U+0413, U+0414, U+0415, U+0416, U+0417,"
	"U+0418, U+0419, U+041A, U+041B, U+041C, U+041D, U+041E, U+041F,"
	"U+0420, U+0421, U+0422, U+0423, U+0424, U+0425, U+0426, U+0427,"
	"U+0428, U+0429, U+042A, U+042B, U+042C, U+042D, U+042E, U+042F,"
	"U+0430, U+0431, U+0432, U+0433, U+0434, U+0435, U+0436, U+0437,"
	"U+0438, U+0439, U+043A, U+043B, U+043C, U+043D, U+043E, U+043F,"
	"U+0440, U+0441, U+0442, U+0443, U+0444, U+0445, U+0446, U+0447,"
	"U+0448, U+0449, U+044A, U+044B, U+044C, U+044D, U+044E, U+044F";

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
		{L"ascii", BuiltinResource(kCodepageASCII, false)},
		{L"437", BuiltinResource(kCodepage437, false)},
		{L"1251", BuiltinResource(kCodepage1251, false)}
	};

	std::unique_ptr<std::istream> Resource::Open(std::wstring name)
	{
		auto i = kBuiltinResources.find(name);
		if (i != kBuiltinResources.end())
		{
			// Has built-in
			if (!i->second.base64encoded)
			{
				return std::unique_ptr<std::istream>(new std::istringstream(i->second.data));
			}
			else
			{
				throw std::runtime_error("base64-encoded built-in resources NYI"); // FIXME: NYI
			}
		}
		else
		{
			// Search on filesystem
#if defined(_WIN32)
			// Windows version: change slashes to backslashes
			for (auto& c: name) if (c == L'/') c = L'\\';
#endif
			std::unique_ptr<std::istream> result;
#if defined(_MSC_VER)
			result.reset(new std::ifstream(name, std::ios_base::in|std::ios_base::binary));
#else
			std::string name_u8 = UTF8->Convert(name);
			result.reset(new std::ifstream(name_u8, std::ios_base::in|std::ios_base::binary));
#endif
			if (result->fail())
			{
				throw std::runtime_error("Resource %name cannot be opened"); // FIXME: formatting
				result.reset();
			}
			return result;
		}
	}
}
