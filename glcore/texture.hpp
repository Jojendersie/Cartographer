#pragma once

#include <ei/elementarytypes.hpp>
#include "core/scopedptr.hpp"

namespace MiR {

	/// Base class for different (2D, 3D, cube) textures.
	class Texture
	{
	public:
		uint getID() const { return m_textureID; }
	protected:
		uint m_textureID;
	};

	/// Class to manage multiple 2D textures.
	/// \details This texture creates a 2D texture array with dynamic allocation. The inserted
	///		textures must not have the same size. They only need to be smaller than the maximum.
	///
	///		Multiple textures can reside side by side, so sampling modes like mirror... may fail.
	///		It does not fail when the textures have the maximum resolution.
	///
	///		A single texture should not be smaller than maxSize / 32, otherwise memory will be
	///		wasted. If you want too store many very small tiles use a smaller maxSize.
	class TextureAtlas: public Texture
	{
	public:
		/// Create an empty texture atlas.
		TextureAtlas(int _maxWidth, int _maxHeight);
		
		struct Entry
		{
			uint16 texLayer;
			uint16 texCoordX;
			uint16 texCoordY;
			uint16 tileSize;
		};
		
		/// Add a new texture from a file to the atlas.
		/// \returns A handle to address where the texture is in the atlas.
		Entry add(const char* _textureFileName);
		
		/// Load/Reload a texture tile.
		/// \details The texture must be smaller or equal the tileSize of the location.
		void load(Entry _location, const char* _textureFileName);
		
	private:
		/// A quad tree in heap order for "buddy" memory allocation. Each node contains the maximum
		/// available space in its subtree, where 0 = max. size level.
		/// The number of root nodes is the number of texture layers in the array.
		/// The order of children is top-left, top-right, bottom-left, bottom-right.
		///	The tree has always 4^5 + 4^4 + 4^3 ... + 4^0 = 1365 uint8 elements. This allows a
		///	total depth of 5 subdivisions leading to 32x32 tiles as smallest texture partitions.
		const int TREE_SIZE = 1365;
		ScopedPtr<uint8> m_quadTree;
		int m_numRoots;
		//uint8* m_maxFreeLevel; ///< Maximum available space for each layer where 0 = max. size level.
		uint16* m_kdTree;	///< Subdivision of each layer
		int m_width;
		int m_height;
		
		/// Find a new tile location
		bool allocate(int _width, int _height, Entry& _location);
		bool recursiveAllocate(uint _off, uint _idx, int _w, int _h, Entry& _location);
		
		/// Allocate at least one more layer
		//TODO: is there a GPU GPU texture copy? Yes: glCopyImageSubData in GL4.3
		void resize();
	};

} // namespace MiR