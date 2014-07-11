/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
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

#ifndef BEARLIBTERMINAL_STAGE_HPP
#define BEARLIBTERMINAL_STAGE_HPP

#include "Atlas.hpp"
#include "Color.hpp"
#include "Tileset.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

namespace BearLibTerminal
{
	struct Cell
	{
		std::vector<Leaf> leafs;
	};

	struct Layer
	{
		Layer(Size size);
		std::vector<Cell> cells;
	};

	struct Scene
	{
		std::vector<Layer> layers;
		std::vector<Color> background;
	};

	struct Stage
	{
		Size size;
		Scene frontbuffer;
		Scene backbuffer;
		void Resize(Size size);
	};

	struct State
	{
		Size cellsize;      // Current cellsize; different from Options.window_cellsize in that this one is always properly set.
		Size half_cellsize; // Cached value used in leaf drawing.
		Color color;
		Color bkcolor;
		int composition;
		int layer;
		State();
	};

	struct World
	{
		std::map<uint16_t, std::unique_ptr<Tileset>> tilesets;
		TileContainer tiles;
		Stage stage;
		State state;
	};
}

#endif // BEARLIBTERMINAL_STAGE_HPP
