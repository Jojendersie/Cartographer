#pragma once

namespace MiR {

	/// Batch renderer for sprites.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class SpriteRenderer
	{
	public:
		/// Initialize the renderer once.
		/// \param [in] _maxTexSize Maximum size of a single texture in the atlas. This can be larger
		///		than any texture which is added, but should never be smaller. 
		void init(int _maxTexSize);
		
		/// Add a texture to the internal atlas.
		/// \returns The ID of the texture in the atlas. Use this id when defining sprites.
		int addTexture(const char* _textureName);
		
		/// Create a new sprite definition.
		/// \details Creates a sprite of a certain size and positioning which covers a (region) of a
		///		texture.
		/// \param [in] _width Width of the sprite.
		/// \param [in] _height Height of the sprite.
		/// \param [in] _alignX -1: the sprite coordinate is on the left, 0: centred in X, 1: on the
		///		right side.
		/// \param [in] _alignY -1: the sprite coordinate is on the top, 0: centred in Y, 1: on the
		///		bottom side.
		/// \param [in] _textureID ID of the texture in the internal atlas.
		/// \param [in] _texX Left pixel coordinate in the texture. The default is 0.
		/// \param [in] _texY Top pixel coordinate in the texture. The default is 0.
		/// \param [in] _texWidth The width of the texture region in pixels. The default -1 uses the
		///		full texture.
		/// \param [in] _texWidth The height of the texture region in pixels. The default -1 uses the 
		///		full texture.
		/// \returns The ID of the new sprite which must be used to create instances.
		int defSprite(float _width, float _height, int _alignX, int _alignY,
					  int _textureID, int _texX=0, int _texY=0, int _texWidth=-1, int _texHeight=-1);
		
		/// \param [in] _position Position in world (x,y) and z for the "layer".
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		void newInstance(int _spriteID, const ei::Vec3& _position, float _rotation);
		
		/// Single instanced draw call for all instances.
		void draw() const;
	};

} // namespace MiR