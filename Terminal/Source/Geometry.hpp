/*
 * Geometry.hpp
 *
 *  Created on: Oct 17, 2013
 *      Author: Cfyz
 */

#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include "Size.hpp"
#include "Point.hpp"
#include <istream>
#include <ostream>

namespace BearLibTerminal
{
	// stream << Size
	template<typename char_t, typename T> std::basic_ostream<char_t>& operator<< (std::basic_ostream<char_t>& stream, const BasicSize<T>& s)
	{
			stream << s.width << "x" << s.height;
			return stream;
	}

	// stream >> Size
	template<typename char_t, typename T> std::basic_istream<char_t>& operator>> (std::basic_istream<char_t>& stream, BasicSize<T>& s)
	{
			stream >> s.width;
			stream.ignore(1);
			stream >> s.height;
			return stream;
	}

	// stream << Point
	template<typename char_t, typename T> std::basic_ostream<char_t>& operator<< (std::basic_ostream<char_t>& stream, const BasicPoint<T>& s)
	{
			stream << s.x << "," << s.y;
			return stream;
	}

	// stream >> Point
	template<typename char_t, typename T> std::basic_istream<char_t>& operator>> (std::basic_istream<char_t>& stream, BasicPoint<T>& s)
	{
			stream >> s.x;
			stream.ignore(1);
			stream >> s.y;
			return stream;
	}
}

#endif /* GEOMETRY_HPP_ */
