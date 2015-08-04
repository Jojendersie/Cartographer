#pragma once

#include "glcore/texture.hpp"

#include <ei/vector.hpp>
#include <vector>

namespace MiR {

	/// Batch renderer for sprites.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class SpriteRenderer
	{
	public:
		/// Initialize the renderer once.
		SpriteRenderer();

		/// \param [in] _maxTexSize Maximum size of a single texture in the atlas. This can be larger
		///		than any texture which is added, but should never be smaller. 
		//void init(int _maxTexSize);
		
		/// Add a texture to the internal atlas.
		/// \returns The ID of the texture in the atlas. Use this id when defining sprites.
		//int addTexture(const char* _textureName);
		
		/// Create a new sprite definition.
		/// \details Creates a sprite of a certain size and positioning which covers a (region) of a
		///		texture.
		/// \param [in] _alignX Defines which point of the sprite is set to the position when
		///		instances are added.
		///		0: the sprite coordinate is on the left, 0.5: centered in X, 1: on the right side.
		/// \param [in] _alignY Defines which point of the sprite is set to the position when
		///		instances are added.
		///		0: the sprite coordinate is on the bottom, 0.5: centered in Y, 1: on the top side.
		/// \param [in] _textureHandle Bindless handle of a texture.
		/// \param [in] _texX Left pixel coordinate in the texture. The default is 0.
		/// \param [in] _texY Bottom pixel coordinate in the texture. The default is 0.
		/// \param [in] _texWidth The width of the texture region in pixels. The default -1 uses the
		///		full texture.
		/// \param [in] _texWidth The height of the texture region in pixels. The default -1 uses the 
		///		full texture.
		/// \returns The ID of the new sprite which must be used to create instances.
		int defSprite(float _alignX, float _alignY,
					  Texture2D::Handle _textureHandle, int _texX=0, int _texY=0, int _texWidth=-1, int _texHeight=-1);
		
		/// \param [in] _position Position in world (x,y) and z for the "layer". You may also use
		///		the sprites in a 3D environment as billboards. Dependent on the camera z is also
		///		used for perspective division (not if orthographic).
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		/// \param [in] _scale A relative scale where 1.0 renders the sprite pixel perfect
		///		(1 screen-pixel = 1 texture-pixel) at a distance of 1.0** or with orthographic projection.
		void newInstance(int _spriteID, const ei::Vec3& _position, float _rotation, const ei::Vec2& _scale);

		/// Clear all existing instances (recommended for fully dynamic buffers)
		void clearInstances();
		
		/// Single instanced draw call for all instances.
		void draw() const;

	private:
		struct Sprite
		{
			ei::Vec<uint16, 4> texCoords;
			uint64 texture;
		};

		struct SpriteDef
		{
			Sprite sprite;
			ei::Vec2 offset;
			ei::IVec2 size;
		};

		struct SpriteInstance
		{
			Sprite sprite;
			ei::Vec3 position;
			float rotation;
			ei::Vec2 scale;
		};

		unsigned m_vao;		///< OpenGL vertex array object
		unsigned m_vbo;		///< OpenGL vertex buffer object

		std::vector<SpriteDef> m_sprites;
		std::vector<SpriteInstance> m_instances;
		mutable bool m_dirty;
	};

} // namespace MiR