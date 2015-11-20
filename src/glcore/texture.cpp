#include "glcore/texture.hpp"
#include "core/error.hpp"
#include "glcore/opengl.hpp"
#include <memory.h>

#define STB_IMAGE_IMPLEMENTATION
#include "dependencies/stb_image.h"

namespace ca { namespace cc {

	static GLenum NUM_COMPS_TO_PIXEL_FORMAT[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
	static GLenum NUM_COMPS_TO_INTERNAL_FORMAT[] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
	static GLenum NUM_COMPS_TO_INTERNAL_FORMAT_SRGB[] = {GL_SRGB8, GL_SRGB8_ALPHA8};

	Texture2D::Texture2D(int _width, int _height, int _numComponents, const Sampler& _sampler) :
		m_width(_width),
		m_height(_height),
		m_numComponents(_numComponents),
		m_sampler(&_sampler)
	{
		// Create openGL - resource
		glGenTextures(1, &m_textureID);
	}

	Texture2D::Texture2D(const char* _textureFileName, const Sampler& _sampler, bool _srgb) : 
		m_bindlessHandle(0),
		m_sampler(&_sampler)
	{
		// Load from file
		stbi_uc* textureData = stbi_load(_textureFileName, &m_width, &m_height, &m_numComponents, 0);
		// Create openGL - resource
		glGenTextures(1, &m_textureID);
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		if(_srgb && m_numComponents >= 3)
			glCall(glTexImage2D, GL_TEXTURE_2D, 0, NUM_COMPS_TO_INTERNAL_FORMAT_SRGB[m_numComponents-3], m_width, m_height, 0, NUM_COMPS_TO_PIXEL_FORMAT[m_numComponents-1], GL_UNSIGNED_BYTE, textureData);
		else
			glCall(glTexImage2D, GL_TEXTURE_2D, 0, NUM_COMPS_TO_INTERNAL_FORMAT[m_numComponents-1], m_width, m_height, 0, NUM_COMPS_TO_PIXEL_FORMAT[m_numComponents-1], GL_UNSIGNED_BYTE, textureData);
		glCall(glGenerateMipmap, GL_TEXTURE_2D);

		stbi_image_free(textureData);

		// Enable bindless access
		m_bindlessHandle = glCall(glGetTextureSamplerHandleARB, m_textureID, m_sampler->getID());
		glCall(glMakeTextureHandleResidentARB, m_bindlessHandle);
	}

	Texture2D::Handle Texture2D::load(const char* _fileName, const Sampler& _sampler, bool _srgb)
	{
		return new Texture2D(_fileName, _sampler, _srgb);
	}

	void Texture2D::unload(Handle _texture)
	{
		// The handle is defined as const, so nobody can do damage, but now we need
		// the real address for deletion
		delete const_cast<Texture2D*>(_texture);
	}

	Texture2D* Texture2D::create(int _width, int _height, int _numComponents, const Sampler& _sampler)
	{
		return new Texture2D(_width, _height, _numComponents, _sampler);
	}

	void Texture2D::fillMipMap(int _level, const byte* _data, bool _srgb)
	{
		glCall(glBindTexture, GL_TEXTURE_2D, m_textureID);
		int divider = 1 << _level;
		int levelWidth = ei::max(1, m_width / divider);
		int levelHeight = ei::max(1, m_height / divider);
		if(_srgb && m_numComponents >= 3)
			glCall(glTexImage2D, GL_TEXTURE_2D, _level, NUM_COMPS_TO_INTERNAL_FORMAT_SRGB[m_numComponents-3], levelWidth, levelHeight, 0, NUM_COMPS_TO_PIXEL_FORMAT[m_numComponents-1], GL_UNSIGNED_BYTE, _data);
		else
			glCall(glTexImage2D, GL_TEXTURE_2D, _level, NUM_COMPS_TO_INTERNAL_FORMAT[m_numComponents-1], levelWidth, levelHeight, 0, NUM_COMPS_TO_PIXEL_FORMAT[m_numComponents-1], GL_UNSIGNED_BYTE, _data);
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
