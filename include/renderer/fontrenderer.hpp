#pragma once

#include "glcore/texture.hpp"
#include "core/scopedptr.hpp"
#include "utilities/half.hpp"

#include <ei/vector.hpp>
#include <vector>
#include <unordered_map>

// Predeclarations
typedef struct FT_FaceRec_*  FT_Face;

namespace ca { namespace cc {

	/// Renderer for text.
	/// \details This renderer represents a single font type and a set of texts. It buffers all
	///		draw-text calls until present. The text buffer is not cleared automaticall. For
	///		dynamic text call clearText() once at the beginning of a frame.
	class FontRenderer
	{
	public:
		/// Create OpenGL-resources
		FontRenderer();
		/// Destroy OpenGL-resources
		~FontRenderer();

		/// Initialize the renderer for a specific font.
		/// \details This will create a freetype-instance internally, load the font and render
		///		textures. Because of distance computations this is a heavyweight operation.
		///		Avoid creation of unessesary renderers.
		///
		///		If you stored the rendered font to a .caf (Cartographer font) file, then loading
		///		becomes much faster. To load caf-files use \ref loadCaf.
		void createFont(const char* _fontFile, const char* _characters);

		/// Store the font in a native format for faster loading
		void storeCaf(const char* _fontFile);

		/// Load a native Cartographer font file (.caf)
		void loadCaf(const char* _fontFile);

		/// Queue a new string for rendering.
		/// \param [in] _position Position of the reference point (see _alignX and _alignY).
		///		Use the .z coordinate as layer for depth sorting.
		/// \param [in] _text Unicode text (UTF8). Type literals as u8"blabla" or risk invalid
		///		chars.
		/// \param [in] _size Height of a text line in pixels.
		/// \param [in] _alignX Relative position of the reference point in the string.
		///		0.0 is on the left and 1.0 is on the right.
		/// \param [in] _alignY Relative position of the reference point in the string.
		///		0.0 is on the bottom and 1.0 is on the top.
		/// \param [in] _roundToPixel Move the cursor to an integral pixel position for sharper
		///		text. Not rounding is better for dynamic moving text (smoother).
		void draw(const ei::Vec3& _position, const char* _text, float _size, const ei::Vec4& _color, float _rotation = 0.0f, float _alignX = 0.0f, float _alignY = 0.0f, bool _roundToPixel = false);

		/// Remove all previously drawn texts
		void clearText();
		
		/// Single instanced draw call for all characters.
		void present() const;

	private:
#pragma pack(push, 2)
		struct CharacterDef
		{
			uint16 advance;					///< Standard space from the beginning to the next character, if kerning does not define something else. In 1/64 pixels.
			int8 baseX, baseY;				///< Offsets which must be added to the pen position to place the lower left corner of the sprite on largest bitmap size (pixels).
			ei::Vec<uint16, 4> texCoords;	///< Texture coordinates in the atlas for lower-left and top-right corner
			ei::Vec2 texSize;				///< Size of the sprite in the texture atlas (pixels)
			struct KerningPair {
				char32_t character;
				int16 kern;					///< Signed 9.6 distance to the default advance
				bool operator < (const KerningPair& _rhs) const { return character < _rhs.character; }
			};
			std::vector<KerningPair> kerning;	///< Table with spacings for all following chars where spacing is unequal the advance.
		};

		struct CharacterVertex
		{
			ei::Vec<uint16, 4> texCoords;	/// l, u, r and b texture coordinates.
			ei::Vec<half, 2> size;
			ei::Vec<uint8, 4> color;
			ei::Vec3 position;
			float rotation;
		};
#pragma pack(pop)
		Texture2D::Handle m_texture;
		ScopedPtr<Sampler> m_sampler;
		unsigned m_vao;		///< OpenGL vertex array object
		unsigned m_vbo;		///< OpenGL vertex buffer for sprites
		mutable bool m_dirty;

		std::unordered_map<char32_t, CharacterDef> m_chars;
		std::vector<CharacterVertex> m_instances;
		int m_baseLineOffset;				///< Offset to the original text base line (normalization lifts all characters)

		/// Create smallest mipmap.
		/// \param [in] _fontSize Font height in pixels.
		/// \param [in] _mapWidth Basic width for the target map. Height is determined dynamically.
		/// \param [in] _padding Amount of space between two characters
		/// \param [in] _characters The set of characters to be drawn.
		/// \return The height of the created map
		int createMap(std::vector<byte>& _target, const char* _characters, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _padding);
		/// Modified version of creatMap, which assumes an existing char-atlas and fits new renderings into it.
		void createMagMap(std::vector<byte>& _target, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _mapHeight, int _padding, int _mipFactor);
		/// Compute final character metrics in texture coordinates
		void normalizeCharacters(const FT_Face _fontFace, const char* _characters);

		/// Iterate through utf8 code points. Returns unicodes of iterated characters.
		/// \param [in] _textit Iterator for an UTF8-string. For iteration the given pointer will
		///		be modified. If a string reaches its end '\0' getNext will not change the pointer
		///		further.
		char32_t getNext(const char** _textit);
	};

}} // namespace ca::cc
