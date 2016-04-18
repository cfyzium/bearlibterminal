/*
 * BitmapTileset.hpp
 *
 *  Created on: Oct 17, 2013
 *      Author: Cfyz
 */

#ifndef BITMAPTILESET_HPP_
#define BITMAPTILESET_HPP_

#include "Tileset.hpp"

namespace BearLibTerminal
{
	class BitmapTileset: public Tileset
	{
	public:
		BitmapTileset(char32_t offset, OptionGroup& options);
		Size GetBoundingBoxSize();

	private:
		Size m_bounding_box_size;
	};
}

#endif /* BITMAPTILESET_HPP_ */
