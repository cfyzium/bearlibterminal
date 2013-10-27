/*
 * Encoding.cpp
 *
 *  Created on: Oct 20, 2013
 *      Author: Cfyz
 */

#include "Encoding.hpp"

namespace BearLibTerminal
{
	/*
	template<typename CharT> struct Encoding
	{
		virtual ~Encoding() { }
		virtual wchar_t Convert(int value) const = 0;
		virtual int Convert(wchar_t value) const = 0;
		virtual std::wstring Convert(const std::basic_string<CharT>& value) const = 0;
		virtual std::basic_string<CharT> Convert(const std::wstring& value) const = 0;
		static std::unique_ptr<Encoding<char>> GetUnibyteEncoding(const std::wstring& name);
	};
	*/

	struct UTF8Encoding: Encoding<char>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::string& value) const override;
		std::string Convert(const std::wstring& value) const override;
	};

	wchar_t UTF8Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UTF8Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UTF8Encoding::Convert(const std::string& value) const
	{
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::string UTF8Encoding::Convert(const std::wstring& value) const
	{
		std::string result;
		for (auto c: value) result += (char)c;
		return result;
	}

	std::unique_ptr<Encoding<char>> UTF8(new UTF8Encoding());

	// ------------------------------------------------------------------------

	struct UCS2Encoding: Encoding<char16_t>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::u16string& value) const override;
		std::u16string Convert(const std::wstring& value) const override;
	};

	wchar_t UCS2Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UCS2Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UCS2Encoding::Convert(const std::u16string& value) const
	{
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::u16string UCS2Encoding::Convert(const std::wstring& value) const
	{
		std::u16string result;
		for (auto c: value) result += (char16_t)c;
		return result;
	}

	std::unique_ptr<Encoding<char16_t>> UTF16(new UCS2Encoding());

	// ------------------------------------------------------------------------

	struct UCS4Encoding: Encoding<char32_t>
	{
		wchar_t Convert(int value) const override;
		int Convert(wchar_t value) const override;
		std::wstring Convert(const std::u32string& value) const override;
		std::u32string Convert(const std::wstring& value) const override;
	};

	wchar_t UCS4Encoding::Convert(int value) const
	{
		return (wchar_t)value;
	}

	int UCS4Encoding::Convert(wchar_t value) const
	{
		return (int)value;
	}

	std::wstring UCS4Encoding::Convert(const std::u32string& value) const
	{
		std::wstring result;
		for (auto c: value) result += (wchar_t)c;
		return result;
	}

	std::u32string UCS4Encoding::Convert(const std::wstring& value) const
	{
		std::u32string result;
		for (auto c: value) result += (char32_t)c;
		return result;
	}

	std::unique_ptr<Encoding<char32_t>> UTF32(new UCS4Encoding());

	// ------------------------------------------------------------------------

	std::unique_ptr<Encoding<char>> GetUnibyteEncoding(const std::wstring& name)
	{
		// FIXME: various encodings
		return std::unique_ptr<Encoding<char>>(new UTF8Encoding());
	}
}
