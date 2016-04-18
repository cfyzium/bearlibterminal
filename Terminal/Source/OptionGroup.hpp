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

#ifndef OPTIONGROUP_HPP_
#define OPTIONGROUP_HPP_

#include <string>
#include <list>
#include <map>

namespace BearLibTerminal
{
	struct OptionGroup
	{
		std::wstring name;
		std::map<std::wstring, std::wstring> attributes;
	};

	std::list<OptionGroup> ParseOptions2(const std::wstring& s, bool semicolon_comments = false);

	std::wstring read_until3(const wchar_t*& p, const std::wstring& until);
}

#endif /* OPTIONGROUP_HPP_ */
