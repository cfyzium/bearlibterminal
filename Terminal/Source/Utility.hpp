/*
 * Utility.hpp
 *
 *  Created on: Oct 16, 2013
 *      Author: Cfyz
 */

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <string>
#include <sstream>

namespace BearLibTerminal
{
	template<typename T, typename char_t> bool try_parse(const std::basic_string<char_t>& s, T& out)
	{
		std::basic_stringstream<char_t> stream(s);
		stream >> out;
		return !(stream.fail() || stream.bad());
	}

	template<typename char_t> bool ends_with(const std::basic_string<char_t>& what, const std::basic_string<char_t>& with)
	{
		// FIXME: does not handle multiple occurences properly
		return what.find(with) == what.length() - with.length();
	}
}

#endif /* UTILITY_HPP_ */
