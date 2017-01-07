#pragma once

#include <ei/elementarytypes.hpp>
#include "ca/cc/core/scopedptr.hpp"
#include "ca/cc/core/manager.hpp"
#include "ca/cc/glcore/sampler.hpp"

namespace ca { namespace cc {

	/// Base class for different (2D, 3D, cube) textures.
	class Texture
	{
	public:
		uint getID() const { return m_textureID; }

	protected:
		uint m_textureID;
	};

	/// 2D Texture class
	class Texture2D: public Texture
	{
	public:
		typedef const Texture2D* Handle;

		static Handle load(const char* _fileName, const Sampler& _sampler, bool _srgb = true);
		static void unload(Handle _texture);

		/// Create a new empty texture.
		/// \details There is a support for LDR (8-bit) textures only.
		static Texture2D* create(int _width, int _height, int _numComponents, const Sampler& _sampler);
		/// Upload information for a single texture layer
		/// \param [in] _level Mip-map level starting with 0 for the higest resolution.
		/// \param [in] _data Pixel data with 8-bit per component and N components per pixel.
		void fillMipMap(int _level, const byte* _data, bool _srgb = false);
		/// Makes the texture resident and may compute mip-maps.
		Handle finalize(bool _createMipMaps = false, bool _makeResident = true);

		/// Bind 2D texture to given location
		void bind(unsigned _slot) const;

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; } 

		/// Get the bindless texture handle.
		uint64 getGPUHandle() const { return m_bindlessHandle; }

		/// Get the OpenGL handle
		uint getID() const { return m_textureID; }

		/// Change the sampler.
		void setSampler(const Sampler& _sampler);
	private:
		/// Create a 2D texture without data
		Texture2D(int _width, int _height, int _numComponents, const Sampler& _sampler);

		/// Load a 2D texture from file.
		/// \details Allowed file formats are JPG, PNG, TGA, BMP, PSD, GIF, (HDR), PIC.
		///
		///		The texture will be always resident and available for bindless access.
		/// \param [in] _textureFileName The file to load.
		/// \param [in] _sampler A sampler which will be used with this texture.
		Texture2D(const char* _textureFileName, const Sampler& _sampler, bool _srgb);

		int m_width;
		int m_height;
		int m_numComponents; ///< R, G, B, A?
		const Sampler* m_sampler;
		uint64 m_bindlessHandle;
	};

	/// The manager to load 2D textures to avoid loading the same texture twice.
	typedef Manager<Texture2D> Texture2DManager;

	/// Class to manage multiple 2D textures.
	/// \details This texture creates a 2D texture array with dynamic allocation. The inserted
	///		textures must not have the same size. They only need to be smaller than the maximum.
	///
	///		Multiple textures can reside side by side, so sampling modes like mirror... may fail.
	///		It does not fail when the textures have the maximum resolution.
	///
	///		A single texture should not be smaller than maxSize / 32, otherwise memory will be
	///		wasted. If you want too store many very small tiles use a smaller maxSize.
/*	class TextureAtlas: public Texture
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
	};*/

}} // namespace ca::cc
