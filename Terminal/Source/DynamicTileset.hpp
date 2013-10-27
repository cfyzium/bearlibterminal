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
		void Remove() override;
		bool Save() override;
		void Reload(DynamicTileset&& tileset) override;
		Size GetBoundingBoxSize() override;
		bool Provides(uint16_t code) override;
		void Prepare(uint16_t code) override;
	private:
		Size m_tile_size;
	};
}

#endif /* DYNAMICTILESET_HPP_ */
