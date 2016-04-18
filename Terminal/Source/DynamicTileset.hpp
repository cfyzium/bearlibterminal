/*
 * DynamicTileset.hpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Cfyz
 */

#ifndef DYNAMICTILESET_HPP_
#define DYNAMICTILESET_HPP_

#include "Tileset.hpp"

namespace BearLibTerminal
{
	class DynamicTileset: public Tileset
	{
	public:
		DynamicTileset(char32_t offset, OptionGroup& options);
		Size GetBoundingBoxSize();
		bool Provides(char32_t code);
		std::shared_ptr<TileInfo> Get(char32_t code);
	private:
		Size m_tile_size;
	};
}

#endif /* DYNAMICTILESET_HPP_ */
