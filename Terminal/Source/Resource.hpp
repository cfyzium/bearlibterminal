/*
 * Resources.hpp
 *
 *  Created on: Oct 30, 2013
 *      Author: cfyz
 */

#ifndef RESOURCES_HPP_
#define RESOURCES_HPP_

#include <istream>
#include <string>
#include <memory>

namespace BearLibTerminal
{
	class Resource
	{
	public:
		static std::unique_ptr<std::istream> Open(std::wstring name, std::wstring prefix=L"");
	};
}

#endif /* RESOURCES_HPP_ */
