/*
* BearLibTerminal
* Copyright (C) 2013-2014 Cfyz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
