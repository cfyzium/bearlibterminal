/*
 * Stage.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Cfyz
 */

#include "Stage.hpp"
#include "BearLibTerminal.h"

namespace BearLibTerminal
{
	Leaf::Leaf():
		dx(0),
		dy(0),
		code(0),
		flags(0),
		reserved(0)
	{ }

	Layer::Layer(Size size):
		cells(size.Area())
	{ }

	void Stage::Resize(Size new_size)
	{
		size = new_size;

		// Background: reset to transparent
		backbuffer.background = std::vector<Color>(size.Area());

		if (backbuffer.layers.empty())
		{
			// Scene must have at least one layer
			backbuffer.layers.emplace_back(size);
		}
		else
		{
			// Must preserve number of layers since who knows what layer is currently selected
			for (auto& i: backbuffer.layers) i = Layer(size);
		}

		// TODO: unnecessary?
		if (frontbuffer.background.size() != backbuffer.background.size())
		{
			frontbuffer = backbuffer;
		}
	}

	State::State():
		color(255, 255, 255, 255),
		bkcolor(),
		composition(TK_OFF),
		layer(0)
	{ }
}
