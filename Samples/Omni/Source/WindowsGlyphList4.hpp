/*
 * WindowsGlyphList4.hpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#ifndef WINDOWSGLYPHLIST4_HPP_
#define WINDOWSGLYPHLIST4_HPP_

#include <vector>
#include <string>
#include <set>

struct UnicodeRange
{
	std::string name;
	int start, end;
	std::set<int> codes;

	UnicodeRange(const std::string& name, int start, int end):
		name(name),
		start(start),
		end(end)
	{ }

	UnicodeRange& Add(int code)
	{
		codes.insert(code);
		return *this;
	}

	UnicodeRange& Add(int from, int to)
	{
		for (int i=from; i<=to; i++) codes.insert(i);
		return *this;
	}
};

extern std::vector<UnicodeRange> g_wgl4_ranges;

#endif /* WINDOWSGLYPHLIST4_HPP_ */
