#include "fontrenderer.hpp"
#include "core/error.hpp"
#include "glcore/opengl.hpp"
#include "glcore/vertexformat.hpp"

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace ei;

namespace cac {

	const int BASE_SIZE = 48;		///< Vertical size in pixels within largest bitmap
	const int MAP_WIDTH = 512;		///< Default width for textures (height depends on the required space)
	const int MIP_RANGE = 8;		///< Size factor between largest (BASE_SIZE) and smallest map

	/*FontRenderer::FontRenderer(const char* _fontFile)
	{
		/// Check file ending and load/create depending on it
		const char* typeSuffix = _fontFile;
		const char* tmp = _fontFile;
		while(*tmp) { if(*tmp == '.') typeSuffix = tmp+1; ++tmp; }
		if(strcmp(typeSuffix, "caf"))
			loadBinary(_fontFile);
		else createFont(_fontFile);
	}*/

	FontRenderer::FontRenderer()
	{
		glCall(glGenVertexArrays, 1, &m_vao);
		glCall(glBindVertexArray, m_vao);
		// Create buffer without data for now
		glCall(glGenBuffers, 1, &m_vbo);
		glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

		// 4 x uint16 for texture l, u, r and b
		glCall(glEnableVertexAttribArray, 0);
		glCall(glVertexAttribPointer, 0, 4, GLenum(PrimitiveFormat::UINT16), GL_TRUE, 32, (GLvoid*)(0));
		// 2 x float for size
		glCall(glEnableVertexAttribArray, 1);
		glCall(glVertexAttribPointer, 1, 2, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 32, (GLvoid*)(8));
		// 3 x float position in world space
		glCall(glEnableVertexAttribArray, 2);
		glCall(glVertexAttribPointer, 2, 3, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 32, (GLvoid*)(16));
		// 1 x float rotation
		glCall(glEnableVertexAttribArray, 3);
		glCall(glVertexAttribPointer, 3, 1, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 32, (GLvoid*)(28));

		m_sampler = new Sampler(Sampler::Filter::POINT, Sampler::Filter::POINT, Sampler::Filter::LINEAR, Sampler::Border::CLAMP);
	}

	FontRenderer::~FontRenderer()
	{
		// Make sure nothing is bound to release the two buffers
		glCall(glBindVertexArray, 0);
		glCall(glBindBuffer, GL_ARRAY_BUFFER, 0);
		glCall(glDeleteBuffers, 1, &m_vbo);
		glCall(glDeleteVertexArrays, 1, &m_vao);
	}

	void FontRenderer::draw(const ei::Vec3& _position, const char* _text, float _size, float _alignX, float _alignY)
	{
		m_instances.clear();
		// TEST-CODE
		// create test vertices which cover the whole texture
		CharacterVertex v;
		v.position = Vec3(0.0f, 0.0f, 0.1f);
		v.rotation = 0.0f;
		v.size = Vec2((float)m_texture->getWidth(), (float)m_texture->getHeight());
		v.texCoords = Vec<uint16, 4>(0, 0, 65535, 65535);
		m_instances.push_back(v);
		for(int i=1; i<4; ++i)
		{
			v.position.x += v.size.x;
			v.position.y += v.size.y;
			v.size /= 2.0f;
			m_instances.push_back(v);
		}

		/*Vec2 cursor(_position.x, _position.y);
		for(char32_t c = getNext(&_text); c; c = getNext(&_text))
		{
			auto charEntry = m_chars.find(c);
			if(charEntry != m_chars.end())
			{
				CharacterVertex v;
				v.position = Vec3(cursor.x + charEntry->second.baseX, cursor.y + charEntry->second.baseY, _position.z);
				v.rotation = 0.0f;
				v.size = charEntry->second.texSize * _size / BASE_SIZE;
				v.texCoords = charEntry->second.texCoords;
				m_instances.push_back(v);
			}
		}*/

		m_dirty = true;
	}

	void FontRenderer::present() const
	{
		// Update buffer
		// TODO: ringbuffer
		if(m_dirty)
		{
			glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
			glCall(glBufferData, GL_ARRAY_BUFFER, m_instances.size() * sizeof(CharacterVertex), m_instances.data(), GL_DYNAMIC_DRAW);
			m_dirty = false;
		}
		// Enable alpha blending (permultiplied)
		glCall(glEnable, GL_BLEND);
		glCall(glBlendFunci, 0, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		// Draw
		m_texture->bind(0);
		glCall(glBindVertexArray, m_vao);
		glCall(glDrawArrays, GL_POINTS, 0, m_instances.size());
		// Clear for next frame
//		m_instances.clear();
	}

	void FontRenderer::createFont(const char* _fontFile, const char* _characters)
	{
		// Init library
		FT_Library ftlib;
		if( FT_Init_FreeType( &ftlib ) )
		{
			error("Cannot initalize freetype!");
			return;
		}

		// Load regular face (oposed to italic...)
		FT_Face fontFace;
		if( FT_New_Face(ftlib, "calibri.ttf", 0, &fontFace) )
		{
			error("Could not open font!");
			return;
		}

		// Create MipMaps beginning with the smallest defined one (halfing may round, doubling doesn't).
		int mipFactor = MIP_RANGE, mipLevel = 3;
		std::vector<byte> map[4];
		int texHeight = MIP_RANGE * createMap(map[mipLevel--], _characters, fontFace, BASE_SIZE / MIP_RANGE, MAP_WIDTH / MIP_RANGE, MIP_RANGE / mipFactor);
		if(!texHeight)
		{
			error("Cancel font creation because basic mipmap could not be created!");
			return;
		}
		while(mipFactor > 1)
		{
			mipFactor /= 2;
			createMagMap(map[mipLevel--], fontFace, BASE_SIZE / mipFactor, MAP_WIDTH / mipFactor, texHeight / mipFactor, MIP_RANGE / mipFactor, MIP_RANGE / mipFactor);
		}
		// Upload texture data
		// Add dummy mip-maps at the lower end
		Texture2D* texture = Texture2D::create(MAP_WIDTH, texHeight, 1, *m_sampler);
		mipLevel = 0;
		while((1<<mipLevel) <= max(MAP_WIDTH, texHeight))
		{
			texture->fillMipMap(mipLevel, map[min(3, mipLevel)].data());
			++mipLevel;
		}
		m_texture = texture->finalize(false, false);
		normalizeCharacters();

		// Free resources
		if( FT_Done_FreeType(ftlib) )
		{
			error("Cannot free all Freetype resources!");
			return;
		}
	}

	void FontRenderer::storeCaf(const char* _fontFile)
	{
	}

	void FontRenderer::loadCaf(const char* _fontFile)
	{
	}

	/// Copy a rectangle into the larger target rectangle
	/// \param [in] _left left position in the target map
	/// \param [in] _top top position in the target map
	static void copyGlyph(std::vector<byte>& _target, const FT_Bitmap& _glyph, int _left, int _top, int _mapWidth)
	{
		for(unsigned y=0; y<_glyph.rows; ++y)
		{
			for(unsigned x=0; x<_glyph.width; ++x)
			{
				unsigned idx = _left+x + _mapWidth*(_top+y);
				if(idx < _target.size())
					_target[idx] = _glyph.buffer[x+_glyph.width*y];
			}
		}
	}

	int FontRenderer::createMap(std::vector<byte>& _target, const char* _characters, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _padding)
	{
		unsigned mapHeight = _padding*2 + _fontSize;
		_target.resize(_mapWidth * mapHeight);

		// Set a fixed font size
		if( FT_Set_Pixel_Sizes(_fontFace, 0, _fontSize) )
		{
			error(("Cannot set size to " + std::to_string(_fontSize) + "px!").c_str());
			return 0;
		}

		// Padding is expected to be a potence of 2 including 1
		int offX = _padding;
		int offY = _padding;
		int maxY = 0;
		for(char32_t c = getNext(&_characters); c; c = getNext(&_characters))
		{
			int idx = FT_Get_Char_Index(_fontFace, c);
			FT_Load_Glyph(_fontFace, idx, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			// Get direct access pointer to the glyph slot
			FT_Bitmap& bmp = _fontFace->glyph->bitmap;
			if(offX + bmp.width + _padding >= (unsigned)_mapWidth)
			{
				offY += maxY + _padding;
				offX = _padding;
				maxY = 0;
			}
			if(offY + bmp.rows + _padding > mapHeight)
			{
				mapHeight += _padding + _fontSize;
				_target.resize(_mapWidth * mapHeight);
			}
			// Add metrics to the hashmap.
			auto charEntry = m_chars.find(c);
			if(charEntry == m_chars.end())
			{
				CharacterDef bmpChar;
				// Store absolute coordinates now and normalize to texture coordinates later because
				// the total height is unknown.
				bmpChar.texSize.x = (float)bmp.width;
				bmpChar.texSize.y = (float)bmp.rows;
				bmpChar.texCoords.x = offX;
				bmpChar.texCoords.y = offY;
				charEntry = m_chars.emplace(c, bmpChar).first;
			} else {
				// Character was already there -> do nothing
				error("Input string contains a character repetition.");
				continue;
			}
			// While copying center the glyph within its padding width
			copyGlyph(_target, bmp, offX, offY, _mapWidth);
			offX += bmp.width + _padding;
			maxY = max((int)bmp.rows, maxY);
		}

		return mapHeight;
	}

	void FontRenderer::createMagMap(std::vector<unsigned char>& _target, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _mapHeight, int _padding, int _mipFactor)
	{
		_target.resize(_mapWidth * _mapHeight);

		// Set a fixed font size
		if( FT_Set_Pixel_Sizes(_fontFace, 0, _fontSize) )
		{
			error(("Cannot set size to " + std::to_string(_fontSize) + "px!").c_str());
			return;
		}

		for(auto cEntry : m_chars)
		{
			int idx = FT_Get_Char_Index(_fontFace, cEntry.first);
			FT_Load_Glyph(_fontFace, idx, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			// Get direct access pointer to the glyph slot
			FT_Bitmap& bmp = _fontFace->glyph->bitmap;
			int targetCharWidth = int(cEntry.second.texSize.x)*_mipFactor;
			int centeringOff = (targetCharWidth - (int)bmp.width) / 2;
			cEntry.second.baseX = -centeringOff - _padding + _fontFace->glyph->bitmap_left;
			cEntry.second.baseY = -_padding - bmp.rows + _fontFace->glyph->bitmap_top;
			cEntry.second.advance = (uint16)_fontFace->glyph->advance.x;
			// While copying center the glyph within its padding width
			copyGlyph(_target, bmp, int(cEntry.second.texCoords.x)*_mipFactor + centeringOff,
				int(cEntry.second.texCoords.y)*_mipFactor, _mapWidth);
		}
	}

	void FontRenderer::normalizeCharacters()
	{
		for(auto cEntry : m_chars)
		{
			// TODO: test half pixel stuff
			//cEntry.second.texOffset *= MIP_RANGE / float(m_texture->getWidth());
			//cEntry.second.texSize *= MIP_RANGE / float(m_texture->getHeight());
			cEntry.second.texCoords.x = (cEntry.second.texCoords.x * MIP_RANGE * 0xffff) / m_texture->getWidth();
			cEntry.second.texCoords.y = (cEntry.second.texCoords.y * MIP_RANGE * 0xffff) / m_texture->getHeight();
			cEntry.second.texCoords.z = cEntry.second.texCoords.x + int(cEntry.second.texSize.x * MIP_RANGE * 0xffff / m_texture->getWidth());
			cEntry.second.texCoords.w = cEntry.second.texCoords.y + int(cEntry.second.texSize.y * MIP_RANGE * 0xffff / m_texture->getHeight());
			cEntry.second.texSize *= MIP_RANGE;
		}
	}

	char32_t FontRenderer::getNext(const char** _textit)
	{
		char32_t c;
		const char* ptr = *_textit;
		// First bit set -> multibyte
		if(ptr[0] & 0x80)
		{
			if((ptr[0] & 0xf8) == 0xf0)
			{
				// All three following characters must start with 10xxx...
				eiAssert(((ptr[1] & 0xc0) == 0x80) && ((ptr[2] & 0xc0) == 0x80) && ((ptr[3] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				// Take all the xxxx from the bytes and put them into one character
				c = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3f) << 12) | ((ptr[2] & 0x3f) << 6) | (ptr[3] & 0x3f);
				*_textit = ptr + 4;
			} else if((ptr[0] & 0xf0) == 0xe0) {
				eiAssert(((ptr[1] & 0xc0) == 0x80) && ((ptr[2] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				c = ((ptr[0] & 0x0f) << 12) | ((ptr[1] & 0x3f) << 6) | (ptr[2] & 0x3f);
				*_textit = ptr + 3;
			} else if((ptr[0] & 0xe0) == 0xc0) {
				eiAssert(((ptr[1] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				c = ((ptr[0] & 0x1f) << 6) | (ptr[1] & 0x3f);
				*_textit = ptr + 2;
			} else
				eiAssert(false, "Invalid utf8 codepoint! The first byte of an utf8 character cannot start with 10xxx.");
		} else {
			c = ptr[0];
			*_textit = ptr + 1;
		}
		return c;
	}

} // namespace cac