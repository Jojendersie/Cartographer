#include "ca/cc/glcore/texture.hpp"
#include "ca/cc/glcore/opengl.hpp"
#include <ca/pa/log.hpp>
#include <memory.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb_image.h"

namespace ca { namespace cc {

	static GLenum NUM_COMPS_TO_PIXEL_FORMAT[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
	static GLenum NUM_COMPS_TO_INTERNAL_FORMAT[] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
	static GLenum NUM_COMPS_TO_INTERNAL_FORMAT_SRGB[] = {GL_SRGB8, GL_SRGB8_ALPHA8};

	static GLenum formatToDataFormat(TexFormat _format)
	{
		switch(_format)
		{
		case TexFormat::D32F:
		case TexFormat::D32:
			return GL_DEPTH_COMPONENT;
		case TexFormat::D24S8:
		case TexFormat::D32FS8:
			return GL_DEPTH_STENCIL;
		case TexFormat::RGBA32F:
		case TexFormat::RGBA32I:
		case TexFormat::RGBA32U:
		case TexFormat::RGBA16:
		case TexFormat::RGBA16S:
		case TexFormat::RGBA16F:
		case TexFormat::RGBA16I:
		case TexFormat::RGBA16U:
		case TexFormat::RGBA8:
		case TexFormat::RGBA8S:
		case TexFormat::RGBA8U:
		case TexFormat::RGBA8I:
		case TexFormat::C_BC5_sRGBA:
		case TexFormat::C_BC5_RGBA:
			return GL_RGBA;
		case TexFormat::RGB8:
		case TexFormat::RGB8S:
		case TexFormat::RGB8U:
		case TexFormat::RGB8I:
		case TexFormat::R11G11B10F:
		case TexFormat::RGB9E5:
		case TexFormat::C_BPTC_RGB:
		case TexFormat::C_BPTC_sRGB:
			return GL_RGB;
		case TexFormat::RG32F:
		case TexFormat::RG32I:
		case TexFormat::RG32U:
		case TexFormat::RG16:
		case TexFormat::RG16S:
		case TexFormat::RG16I:
		case TexFormat::RG16F:
		case TexFormat::RG16U:
		case TexFormat::RG8:
		case TexFormat::RG8S:
		case TexFormat::RG8U:
		case TexFormat::RG8I:
		case TexFormat::C_RGTC2_RGU:
		case TexFormat::C_RGTC2_RGI:
			return GL_RG;
		case TexFormat::R32F:
		case TexFormat::R32I:
		case TexFormat::R32U:
		case TexFormat::R16:
		case TexFormat::R16S:
		case TexFormat::R16F:
		case TexFormat::R16I:
		case TexFormat::R16U:
		case TexFormat::R8:
		case TexFormat::R8S:
		case TexFormat::R8U:
		case TexFormat::R8I:
			return GL_RED;
		}
		return 0;
	}


	TexFormat getFormat(TexType _type, int _numComponents)
	{
		constexpr TexFormat FF_1[] = { TexFormat::R8, TexFormat::R8S, TexFormat::R16, TexFormat::R16S, TexFormat{0}, TexFormat{0}, TexFormat::R32F };
		constexpr TexFormat FF_2[] = { TexFormat::RG8, TexFormat::RG8S, TexFormat::RG16, TexFormat::RG16S, TexFormat{0}, TexFormat{0}, TexFormat::RG32F };
		constexpr TexFormat FF_3[] = { TexFormat::RGB8, TexFormat::RGB8S, TexFormat::RGBA16, TexFormat::RGBA16S, TexFormat{0}, TexFormat{0}, TexFormat::RGBA32F };
		constexpr TexFormat FF_4[] = { TexFormat::RGBA8, TexFormat::RGBA8S, TexFormat::RGBA16, TexFormat::RGBA16S, TexFormat{0}, TexFormat{0}, TexFormat::RGBA32F };
		switch(_numComponents)
		{
		case 1: return FF_1[(int)_type];
		case 2: return FF_2[(int)_type];
		case 3: return FF_3[(int)_type];
		case 4: return FF_4[(int)_type];
		default: return TexFormat{0};
		}
	}


	Texture2D::Texture2D(int _width, int _height, TexFormat _format, const Sampler& _sampler, bool _allocMipMap) :
		m_width(_width),
		m_height(_height),
		m_format(_format),
		m_sampler(&_sampler)
	{
		// Create openGL - resource
		glGenTextures(1, &m_textureID);
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		int numLevels = _allocMipMap ? int(floor(log2(ei::max(_width, _height)))) + 1 : 1;
		//glTextureStorage2D(m_textureID, 1, GLenum(_format), _width, _height);
		glCall(glTexStorage2D, GL_TEXTURE_2D, numLevels, GLenum(_format), _width, _height);

		pa::logInfo("[ca::cc] Created raw texture ", m_textureID, " .");
	}

	Texture2D::Texture2D(const char* _textureFileName, const Sampler& _sampler, bool _srgb, bool _allocMipMap) :
		m_width(0),
		m_height(0),
		m_sampler(&_sampler),
		m_bindlessHandle(0)
	{
		// Load from file
		int numComponents = 0;
		stbi_uc* textureData = stbi_load(_textureFileName, &m_width, &m_height, &numComponents, 0);
		if(!textureData)
		{
			pa::logError("[ca::cc] Could not load texture '", _textureFileName, "'.");
			return;
		}

		// Force black alpha
		if(numComponents == 4)
		{
			for(int i = 0; i < m_width * m_height * 4; i += 4)
			{
				if(textureData[i + 3] == 0)
				{
					textureData[i] = 0;
					textureData[i+1] = 0;
					textureData[i+2] = 0;
				}
			}
		}

		// Create openGL - resource
		glGenTextures(1, &m_textureID);
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		m_format = TexFormat((_srgb && numComponents >= 3) ? NUM_COMPS_TO_INTERNAL_FORMAT_SRGB[numComponents-3] : NUM_COMPS_TO_INTERNAL_FORMAT[numComponents-1]);
		glCall(glTexImage2D, GL_TEXTURE_2D, 0, uint(m_format), m_width, m_height, 0, NUM_COMPS_TO_PIXEL_FORMAT[numComponents-1], GL_UNSIGNED_BYTE, textureData);
		if (_allocMipMap)
			glCall(glGenerateMipmap, GL_TEXTURE_2D);

		stbi_image_free(textureData);

		// Enable bindless access
		m_bindlessHandle = glCall(glGetTextureSamplerHandleARB, m_textureID, m_sampler->getID());
		glCall(glMakeTextureHandleResidentARB, m_bindlessHandle);

		pa::logInfo("[ca::cc] Loaded texture ", m_textureID, " from '", _textureFileName, "'.");
	}

	Texture2D::~Texture2D()
	{
		if(m_bindlessHandle) glCall(glMakeTextureHandleNonResidentARB, m_bindlessHandle);
		glCall(glBindTexture, GL_TEXTURE_2D, 0);
		glCall(glDeleteTextures, 1, &m_textureID);
		pa::logInfo("[ca::cc] Deleted texture ", m_textureID, " .");
	}

	Texture2D::Handle Texture2D::load(const char* _fileName, const Sampler& _sampler, bool _srgb)
	{
		return new Texture2D(_fileName, _sampler, _srgb, true);
	}

	void Texture2D::unload(Handle _texture)
	{
		// The handle is defined as const, so nobody can do damage, but now we need
		// the real address for deletion
		delete const_cast<Texture2D*>(_texture);
	}

	Texture2D* Texture2D::create(int _width, int _height, int _numComponents, const Sampler& _sampler, bool _allocMipMap)
	{
		return new Texture2D(_width, _height, TexFormat(NUM_COMPS_TO_INTERNAL_FORMAT[_numComponents - 1]), _sampler, _allocMipMap);
	}

	Texture2D * Texture2D::create(int _width, int _height, TexFormat _format, const Sampler & _sampler, bool _allocMipMap)
	{
		return new Texture2D(_width, _height, _format, _sampler, _allocMipMap);
	}

	void Texture2D::fillMipMap(int _level, const byte* _data)
	{
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		int divider = 1 << _level;
		int levelWidth = ei::max(1, m_width / divider);
		int levelHeight = ei::max(1, m_height / divider);
		glCall(glTexSubImage2D, GL_TEXTURE_2D, _level, 0, 0, levelWidth, levelHeight, formatToDataFormat(m_format), GL_UNSIGNED_BYTE, _data);
	}

	void Texture2D::fillMipMap(int _level, const byte* _data, int _numComponents, TexType _type)
	{
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		int divider = 1 << _level;
		int levelWidth = ei::max(1, m_width / divider);
		int levelHeight = ei::max(1, m_height / divider);
		constexpr GLenum TOGL_COMP[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		constexpr GLenum TOGL_TYPE[] = { GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT };
		const GLenum format = TOGL_COMP[_numComponents-1];
		const GLenum type = TOGL_TYPE[(int)_type];
		glCall(glTexSubImage2D, GL_TEXTURE_2D, _level, 0, 0, levelWidth, levelHeight, format, type, _data);
	}

	Texture2D::Handle Texture2D::finalize(bool _createMipMaps, bool _makeResident)
	{
		if(_createMipMaps)
			glCall(glGenerateTextureMipmap, m_textureID);

		if(_makeResident)
		{
			m_bindlessHandle = glCall(glGetTextureSamplerHandleARB, m_textureID, m_sampler->getID());
			glCall(glMakeTextureHandleResidentARB, m_bindlessHandle);
		}
		return this;
	}

	void Texture2D::bind(unsigned _slot) const
	{
		// TODO: check binding to avoid rebinds
		glCall(glActiveTexture, GL_TEXTURE0 + _slot);
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		m_sampler->bind(_slot);
	}

	void Texture2D::bindImage(unsigned _slot, bool _read, bool _write)
	{
		if (!(_read || _write))
			pa::logError("[ca::cc] bindImage with neither read nor write access.");
		glCall(glBindImageTexture, _slot,
			m_textureID, 0, false, 0,
			_read && _write ? GL_READ_WRITE : (_read ? GL_READ_ONLY : GL_WRITE_ONLY),
			(GLenum)m_format);
	}

	/*TextureAtlas::TextureAtlas(int _maxWidth, int _maxHeight) :
		m_width(_maxWidth),
		m_height(_maxHeight),
		m_quadTree(new uint8[341]) // Allocate fixed size quad tree
	{
		// Mark all nodes in the quad tree as empty
		memset(m_quadTree, 0, TREE_SIZE);
	}

	bool TextureAtlas::allocate(int _width, int _height, Entry& _location)
	{
		if(_width > m_width || _height > m_height)
		{
			error("The texture size exceeds the atlas size.");
			return false;
		}

		// Search in all layers
		for(int l=0; l<m_numRoots; ++l)
		{
			int w = m_width >> m_quadTree[uint(l * TREE_SIZE)];
			int h = m_height >> m_quadTree[uint(l * TREE_SIZE)];
			if(w >= _width && h >= _height)
			{
				// Enough space in this subtree.
				// Now use tree recursion to find the tile itself.
				_location.tileSize = 0;
				while((m_width >> _location.tileSize) > _width && (m_height >> _location.tileSize) > _height)
					_location.tileSize++;
				_location.texLayer = l;
				_location.texCoordX = 0;
				_location.texCoordY = 0;
				recursiveAllocate(uint(l * TREE_SIZE), 0, m_width, m_height, _location);
				
				return true;
			}
		}
		
		// No layer had enough space, weed need a new one.
		resize();
		return allocate(_width, _height, _location);
	}

	bool TextureAtlas::recursiveAllocate(uint _off, uint _idx, int _w, int _h, Entry& _location)
	{
		// Check if current level is finest resolution.
		int tileW = m_width >> _location.tileSize;
		int tileH = m_height >> _location.tileSize;
		if(_w == tileW || _h == tileH)
		{
			m_quadTree[_off+_idx] = 255;	// Mark es fully occupied
			// The parents already set full location information
			return true;
		} else {
			// Check the 4 children
			bool ret;
			uint childidx = _idx * 4 + 1; // heap order
			if((m_width >> m_quadTree[_off+childidx]) >= tileW && (m_height >> m_quadTree[_off+childidx]) >= tileH) {
				ret = recursiveAllocate(_off, childidx, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+1]) >= tileW && (m_height >> m_quadTree[_off+childidx+1]) >= tileH) {
				_location.texCoordX += _w/2;
				ret = recursiveAllocate(_off, childidx+1, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+2]) >= tileW && (m_height >> m_quadTree[_off+childidx+2]) >= tileH) {
				_location.texCoordY += _h/2;
				ret = recursiveAllocate(_off, childidx+2, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+3]) >= tileW && (m_height >> m_quadTree[_off+childidx+3]) >= tileH) {
				_location.texCoordX += _w/2;
				_location.texCoordY += _h/2;
				ret = recursiveAllocate(_off, childidx+3, _w/2, _h/2, _location);
			} else {
				// Error in allocation (current node to large and none of the children is large enough)
				return false;
			}

			// Update current max-free from the children
			m_quadTree[_off+_idx] = ei::min(m_quadTree[_off + childidx],
											m_quadTree[_off + childidx + 1],
											m_quadTree[_off + childidx + 2],
											m_quadTree[_off + childidx + 3]);
			return ret;
		}
	}*/

}} // namespace ca::cc
