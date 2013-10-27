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
#include <memory>
#include <list>
#include <set>

namespace BearLibTerminal
{
	struct Leaf
	{
		Color color[4];
		uint16_t dx, dy;
		uint16_t code;
		uint8_t flags;
		uint8_t reserved;
	};

	struct State;

	struct Slot
	{
		virtual ~Slot() { }
		Slot();
		virtual void BindTexture() = 0;
		virtual void Draw(const Leaf& leaf, int x, int y) = 0;
		uint64_t texture_id;
	};

	struct Tile
	{
		enum class Placement {Normal, Cropped, Centered};
		Tile();
		virtual ~Tile() { }
		virtual void Update(const Bitmap& bitmap) = 0;
		Placement placement;
		Point offset;
		Size bounds;
	};

	class AtlasTexture3;

	struct TexCoords
	{
		TexCoords();
		TexCoords(float tu1, float tv1, float tu2, float tv2);
		float tu1, tv1, tu2, tv2;
	};

	struct TileSlot: Slot, Tile, std::enable_shared_from_this<TileSlot>
	{
		AtlasTexture3* texture;
		Size space_size;
		Rectangle texture_region;
		TexCoords texture_coords;
		Point shift;

		TileSlot();
		void BindTexture() override;
		void Draw(const Leaf& leaf, int x, int y) override;
		void Update(const Bitmap& bitmap) override;
	};

	class AtlasTexture3
	{
	public:
		enum class Type {Tile, Sprite};

		AtlasTexture3(Type type, Size initial_size);
		Type GetType() const;
		std::shared_ptr<TileSlot> Add(const Bitmap& bitmap, Rectangle region);
		void Update(std::shared_ptr<TileSlot> slot, const Bitmap& bitmap);
		void Remove(std::shared_ptr<TileSlot> slot);
		bool IsEmpty() const;
		void Refresh();
		void Bind();
		void Dispose();

	private:
		bool TryGrow();
		void PostprocessSpaces();
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

	private:
		std::list<AtlasTexture3> m_textures;
	};
}

#endif /* ATLAS_HPP_ */
