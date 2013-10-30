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

#ifndef BEARLIBTERMINAL_TILESET_HPP
#define BEARLIBTERMINAL_TILESET_HPP

#include "Atlas.hpp"
#include "OptionGroup.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <map>

namespace BearLibTerminal
{
	struct TileContainer
	{
		Atlas atlas;
		std::unordered_map<uint16_t, std::shared_ptr<Slot>> slots;
	};

	class Tileset
	{
	public:
		Tileset(TileContainer& container);
		virtual ~Tileset();
		virtual bool Save() = 0;
		virtual void Remove() = 0;
		virtual Size GetBoundingBoxSize() = 0;
		virtual void Reload(Tileset&& tileset) = 0;
		virtual bool Provides(uint16_t code) = 0;
		virtual void Prepare(uint16_t code) = 0;
		static std::unique_ptr<Tileset> Create(TileContainer& container, OptionGroup& options);

	protected:
		TileContainer& m_container;
		std::unordered_map<uint16_t, std::shared_ptr<TileSlot>> m_tiles;
	};

	template<typename T> class StronglyTypedReloadableTileset: public Tileset
	{
	public:
		StronglyTypedReloadableTileset(TileContainer& container):
			Tileset(container)
		{ }

		void Reload(Tileset&& tileset) override
		{
			if (typeid(*this) != typeid(tileset))
			{
				throw std::runtime_error("ReloadableTilesetImpl::Reload(Tileset&&): type mismatch");
			}

			Reload((T&&)tileset);
		}

		virtual void Reload(T&& tileset) = 0;
	};
}

#endif // BEARLIBTERMINAL_TILESET_HPP
