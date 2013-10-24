/*
 * TilesetContainer.hpp
 *
 *  Created on: Sep 22, 2013
 *      Author: Cfyz
 */

#ifndef TILESETCONTAINER_HPP_
#define TILESETCONTAINER_HPP_

#include "Size.hpp"
#include "Point.hpp"
#include "Encoding.hpp"

#include "Bitmap.hpp"

#include <memory>
#include <set>
#include <map>

#include <typeinfo>

#include "Atlas.hpp"

namespace BearLibTerminal
{

	/*
	class TileContainer
	{
	public:
		// ...
	protected:
		std::unordered_map<uint16_t, std::shared_ptr<Slot>> m_tiles;
	};



	struct OptionsGroup
	{
		std::string name;
		std::map<std::string, std::string> attributes;
	};

	class Tileset
	{
	public:
		Tileset(Atlas& atlas);
		virtual ~Tileset();
		static std::unique_ptr<Tileset> Create(const OptionsGroup& options);

	protected:
		Atlas& m_atlas;
		std::unordered_map<uint16_t, TileDesctiption> m_tiles;
	};

	Tileset::Tileset(Atlas& atlas):
		m_atlas(atlas)
	{ }

	Tileset::~Tileset()
	{ }



	template<typename T> class ReloadableTileset: public Tileset
	{
	public:
		void Reload(Tileset&& tileset)
		{
			if (typeid(*this) != typeid(tileset))
			{
				throw std::runtime_error("ReloadableTileset::Reload(Tileset&&): type mismatch");
			}

			Reload((T&&)tileset);
		}

		virtual void Reload(T&& tileset) = 0;
	};




	class BitmapTileset: public ReloadableTileset<BitmapTileset>
	{
	public:
		BitmapTileset(Atlas& atlas, const OptionsGroup& options);
	};
	*/



	/*
	struct TileStack
	{
		// Index 0 is reserved for generic tile plane, indices 1..N are for
		// font planes with index = font+1.
		std::vector<TileDescription> descriptions;
	};
	*/

	/*
	 * auto stack = m_map[code];
	 * size_t size = stack.descriptions.size();
	 * int index = -1;
	 * if (size && stack.descriptions[0].is_present)
	 * {
	 *     index = 0;
	 * }
	 * else if (m_current_font+1 < size && stack.descriptions[m_current_font+1].is_present)
	 * {
	 *     index = m_current_font+1;
	 * }
	 * if (index >= 0)
	 * {
	 *     render(stack.descriptions[index]);
	 * }
	 */



	/*
	class TilesetContainer2;

	class Tileset
	{
	public:
		Tileset(TilesetContainer2& container, const OptionGroup& options);
		virtual ~Tileset() { }

	protected:
		TilesetContainer2& m_container;
	};

	class TilesetContainer2
	{
	public:
		void Add(std::shared_ptr<Tileset> tileset);
		void Remove(const std::string& tileset_tag);
	};
	*/

	/*
	 * Some object will hold:
	 * 1) std::unordered_map<uint16_t, Tile> generic;
	 * 2) std::vector<std::unordered_map<uint16_t, Tile>> fonts;
	 */

	/*
	class TileArray
	{
	public:
		//typedef enum class Placement_ {Normal, Cropped, Centered} Placement;
		//typedef enum class Rasterization_ {Normal, Monochrome, LCD} Rasterization;
	public:
		//Size m_source_size;
		//Size m_target_size;
		//Size m_bounds;
		//Placement m_placement;
		//Rasterization m_rasterization;
		//std::unique_ptr<Encoding<char>> m_encoding;

		//std::set<uint16_t> m_provided_tiles;
		std::map<uint16_t, Tile> m_cache;
	};
	*/

	/*
	 * BitmapTiles:
	 *     source_size, // size
	 *     target_size, // resize
	 *     bounds,
	 *     placement,
	 *     codepage
	 *
	 * TrueTypeTiles:
	 *     size,
	 *     bounds,
	 *     placement,
	 *     rasterization // rasterize
	 *
	 */
}



#endif /* TILESETCONTAINER_HPP_ */
