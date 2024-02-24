#pragma once

#include <ei/elementarytypes.hpp>
#include "ca/cc/core/scopedptr.hpp"
#include "ca/cc/core/manager.hpp"
#include "ca/cc/glcore/sampler.hpp"

namespace ca { namespace cc {

	/// A collection of usefull OpenGL texture formats
	enum class TexFormat
	{
		D32F = 0x8CAC,			// GL_DEPTH_COMPONENT32F
		D32 = 0x81A7,			// GL_DEPTH_COMPONENT32
		D24S8 = 0x88F0,			// GL_DEPTH24_STENCIL8
		D32FS8 = 0x8CAD,		// GL_DEPTH32F_STENCIL8

		RGBA32F = 0x8814,		// GL_RGBA32F
		RGBA32I = 0x8D82,		// GL_RGBA32I
		RGBA32U = 0x8D70,		// GL_RGBA32UI
		RG32F = 0x8230,			// GL_RG32F
		RG32I = 0x823B,			// GL_RG32I
		RG32U = 0x823C,			// GL_RG32UI
		R32F = 0x822E,			// GL_R32F
		R32I = 0x8235,			// GL_R32I
		R32U = 0x8236,			// GL_R32UI

		RGBA16 = 0x805B,		// GL_RGBA16
		RGBA16S = 0x8F9B,		// GL_RGBA16_SNORM
		RGBA16F = 0x881A,		// GL_RGBA16F
		RGBA16I = 0x8D88,		// GL_RGBA16I
		RGBA16U = 0x8D76,		// GL_RGBA16UI

		RG16 = 0x822C,			// GL_RG16
		RG16S = 0x8F99,			// GL_RG16_SNORM
		RG16F = 0x822F,			// GL_RG16F
		RG16I = 0x8239,			// GL_RG16I
		RG16U = 0x823A,			// GL_RG16UI

		R16 = 0x822A,			// GL_R16
		R16S = 0x8F98,			// GL_R16_SNORM
		R16F = 0x822D,			// GL_R16F
		R16I = 0x8233,			// GL_R16I
		R16U = 0x8234,			// GL_R16UI

		R8 = 0x8229,			// GL_R8
		R8S = 0x8F94,			// GL_R8_SNORM
		R8U = 0x8232,			// GL_R8UI
		R8I = 0x8231,			// GL_R8I

		RG8 = 0x822B,			// GL_RG8
		RG8S = 0x8F95,			// GL_RG8_SNORM
		RG8U = 0x8238,			// GL_RG8UI
		RG8I = 0x8237,			// GL_RG8I

		RGB8 = 0x8051,			// GL_RGB8
		RGB8S = 0x8F96,			// GL_RGB8_SNORM
		RGB8U = 0x8D7D,			// GL_RGB8UI
		RGB8I = 0x8D8F,			// GL_RGB8I

		RGBA8 = 0x8058,			// GL_RGBA8
		RGBA8S = 0x8F97,		// GL_RGBA8_SNORM
		RGBA8U = 0x8D7C,		// GL_RGBA8UI
		RGBA8I = 0x8D8E,		// GL_RGBA8I

		R11G11B10F = 0x8C3A,	// GL_R11F_G11F_B10F
		RGB9E5 = 0x8C3D,		// GL_RGB9_E5
		C_BPTC_RGB = 0x8E8C,	// GL_COMPRESSED_RGBA_BPTC_UNORM
		C_BPTC_sRGB = 0x8E8D,	// GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
		C_RGTC2_RGU = 0x8DBD,	// GL_COMPRESSED_RG_RGTC2
		C_RGTC2_RGI = 0x8DBE,	// GL_COMPRESSED_SIGNED_RG_RGTC2
		C_BC5_sRGBA = 0x8C4F,	// GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
		C_BC5_RGBA = 0x83F3,	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	};


	enum class TexType
	{
		U_8,
		I_8,
		U_16,
		I_16,
		U_32,
		I_32,
		F_32
	};


	/// Get a normalized texture format (not integer texture) from
	/// number of components and base type.
	TexFormat getFormat(TexType _type, int _numComponents);


	/// Base class for different (2D, 3D, cube) textures.
	class Texture
	{
	public:
		uint getID() const { return m_textureID; }

		Texture() : m_textureID(0) {}

	protected:
		uint m_textureID;
	};

	/// 2D Texture class
	class Texture2D: public Texture
	{
	public:
		typedef Texture2D* Handle;

		static Handle load(const char* _fileName, const Sampler& _sampler, bool _srgb = true);
		static void unload(Handle _texture);

		/// Create a new empty texture.
		/// \details There is a support for LDR (8-bit) textures only.
		static Handle create(int _width, int _height, int _numComponents, const Sampler& _sampler, bool _allocMipMap = true);
		/// More advanced creation function with arbitrary formats
		static Handle create(int _width, int _height, TexFormat _format, const Sampler& _sampler, bool _allocMipMap = true);
		/// Upload information for a single texture layer.
		/// \param [in] _level Mip-map level starting with 0 for the higest resolution.
		/// \param [in] _data Pixel data with 8-bit per component and N components per pixel.
		void fillMipMap(int _level, const byte* _data);
		/// Upload data for a single texture layer of a format that is not 8-bit or does
		/// not have the same number of components as the texture
		/// \param [in] _data Pixel data with the specified type and channel count.
		///		The resolution must match the mip level.
		/// \param [in] _type The actual type of channel in `_data`.
		void fillMipMap(int _level, const byte* _data, int _numComponents, TexType _type);
		/// Makes the texture resident and may compute mip-maps.
		Handle finalize(bool _createMipMaps = false, bool _makeResident = true);

		/// Bind 2D texture to given location
		void bind(unsigned _slot) const;

		// Bind as gl image for read/write access
		void bindImage(unsigned _slot, bool _read, bool _write);

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
		Texture2D(int _width, int _height, TexFormat _format, const Sampler& _sampler, bool _allocMipMap);

		/// Load a 2D texture from file.
		/// \details Allowed file formats are JPG, PNG, TGA, BMP, PSD, GIF, (HDR), PIC.
		///
		///		The texture will be always resident and available for bindless access.
		/// \param [in] _textureFileName The file to load.
		/// \param [in] _sampler A sampler which will be used with this texture.
		Texture2D(const char* _textureFileName, const Sampler& _sampler, bool _srgb, bool _allocMipMap);

		~Texture2D();

		int m_width;
		int m_height;
		TexFormat m_format; ///< R, G, B, A?
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
