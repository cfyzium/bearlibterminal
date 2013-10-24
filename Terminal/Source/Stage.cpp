/*
 * Stage.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#include "Stage.hpp"

namespace BearLibTerminal
{
	void Stage::Resize(Size new_size)
	{
		size = new_size;

		// Background: reset to transparent
		backbuffer.background = std::vector<Color>(size.Area());

		// TODO: just remove all layers?
		for (auto& i: backbuffer.layers)
		{
			// Layer: reset to empty cells
			i.cells = std::vector<Cell>(size.Area());
		}

		// TODO: unnecessary?
		frontbuffer = backbuffer;
	}

	State::State():
		color(255, 255, 255, 255),
		bkcolor(),
		composition(0),
		layer(0)
	{ }
}
