/*
 * Atlas.hpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#ifndef ATLAS_HPP_
#define ATLAS_HPP_

#include "Size.hpp"
#include "Bitmap.hpp"
#include "Texture.hpp"
#include "Rectangle.hpp"
#include <istream>
#include <ostream>
#include <memory>
#include <list>
#include <set>

namespace BearLibTerminal
{
	struct Leaf
	{
		Leaf();
		Color color[4];
		int16_t dx, dy;
		int16_t code;
		uint8_t flags;
		uint8_t reserved;
		static const uint8_t CornerColored = 0x01;
	};

	struct State;

	struct Slot
	{
		virtual ~Slot() { }
		Slot();
		virtual void BindTexture() = 0;
		virtual void Draw(const Leaf& leaf, int x, int y, int w2, int h2) = 0;
		uint64_t texture_id;
	};

	struct Tile
	{
		enum class Alignment {Unknown, Center, TopLeft, TopRight, BottomLeft, BottomRight};
		Tile();
		virtual ~Tile() { }
		virtual void Update(const Bitmap& bitmap) = 0;
		Alignment alignment;
		Point offset;
		Size bounds;
	};

	std::wostream& operator<<(std::wostream& s, const Tile::Alignment& value);
	std::wistream& operator>>(std::wistream& s, Tile::Alignment& value);

	class AtlasTexture;

	struct TexCoords
	{
		TexCoords();
		TexCoords(float tu1, float tv1, float tu2, float tv2);
		float tu1, tv1, tu2, tv2;
	};

	struct TileSlot: Slot, Tile, std::enable_shared_from_this<TileSlot>
	{
		AtlasTexture* texture;
		Size space_size;
		Rectangle texture_region;
		TexCoords texture_coords;

		TileSlot();
		void BindTexture();
		void Draw(const Leaf& leaf, int x, int y, int w2, int h2);
		void Update(const Bitmap& bitmap);
	};

	class AtlasTexture
	{
	public:
		enum class Type {Tile, Sprite};

		AtlasTexture(Type type, Size initial_size);
		Type GetType() const;
		std::shared_ptr<TileSlot> Add(const Bitmap& bitmap, Rectangle region);
		void Update(std::shared_ptr<TileSlot> slot, const Bitmap& bitmap);
		void Remove(std::shared_ptr<TileSlot> slot);
		bool IsEmpty() const;
		void Refresh();
		void Bind();
		void Dispose();
		Bitmap GetCanvasMap();

	private:
		bool TryGrow();
		void PostprocessSpaces();
		void MergeSpaces();
		TexCoords CalcTexCoords(Rectangle region);

		Type m_type;
		Texture m_texture;
		Bitmap m_canvas;
		bool m_is_dirty;
		std::list<Rectangle> m_spaces;
		std::list<std::shared_ptr<TileSlot>> m_slots;
	};

	class Atlas
	{
	public:
		std::shared_ptr<TileSlot> Add(const Bitmap& bitmap, Rectangle region);
		void Remove(std::shared_ptr<TileSlot> slot);
		void Refresh();
		void Dispose();
		void Dump();

	private:
		std::list<AtlasTexture> m_textures;
	};
}

#endif /* ATLAS_HPP_ */
