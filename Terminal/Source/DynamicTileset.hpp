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
	class DynamicTileset: public StronglyTypedReloadableTileset<DynamicTileset>
	{
	public:
		DynamicTileset(TileContainer& container, OptionGroup& group);
		void Remove();
		bool Save();
		void Reload(DynamicTileset&& tileset);
		Size GetBoundingBoxSize();
		Size GetSpacing();
		Type GetType();
		bool Provides(uint16_t code);
		void Prepare(uint16_t code);
	private:
		Size m_tile_size;
	};
}

#endif /* DYNAMICTILESET_HPP_ */
