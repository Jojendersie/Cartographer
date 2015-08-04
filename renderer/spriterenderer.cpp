#include "spriterenderer.hpp"
#include "glcore/opengl.hpp"
#include "glcore/vertexformat.hpp"
#include "core/error.hpp"

using namespace ei;

namespace MiR {

SpriteRenderer::SpriteRenderer()
{
	glCall(glGenVertexArrays, 1, &m_vao);
	glCall(glBindVertexArray, m_vao);
	// Create buffer without data for now
	glCall(glGenBuffers, 1, &m_vbo);
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

	// 4 x uint16 for texture l, u, r and b
	glCall(glEnableVertexAttribArray, 0);
	glCall(glVertexAttribPointer, 0, 4, GLenum(PrimitiveFormat::UINT16), GL_TRUE, 40, (GLvoid*)(0));
	// 1 x uint64 bindless handle
	glCall(glEnableVertexAttribArray, 1);
	glCall(glVertexAttribIPointer, 1, 2, GLenum(PrimitiveFormat::UINT32), 40, (GLvoid*)(8));
	// 3 x float position in world space
	glCall(glEnableVertexAttribArray, 2);
	glCall(glVertexAttribPointer, 2, 3, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 40, (GLvoid*)(16));
	// 1 x float rotation
	glCall(glEnableVertexAttribArray, 3);
	glCall(glVertexAttribPointer, 3, 1, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 40, (GLvoid*)(28));
	// 2 x float scale
	glCall(glEnableVertexAttribArray, 4);
	glCall(glVertexAttribPointer, 4, 2, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 40, (GLvoid*)(32));
	
	
	// TODO: test dividing the buffers into two (instancing)
}

int SpriteRenderer::defSprite(float _alignX, float _alignY,
			  Texture2D::Handle _textureHandle, int _texX, int _texY, int _texWidth, int _texHeight)
{
	int spriteID = (int)m_sprites.size();
	SpriteDef newSprite;

	float w = (float)_textureHandle->getWidth();
	float h = (float)_textureHandle->getHeight();

	newSprite.sprite.tex0.x = _texX / w;
	newSprite.sprite.tex0.y = _texY / h;
	newSprite.sprite.tex1.x = (_texX + _texWidth) / w;
	newSprite.sprite.tex1.y = (_texY + _texHeight) / h;
	newSprite.sprite.texture = _textureHandle->getHandle();
	newSprite.offset.x = _texWidth * _alignX / w;
	newSprite.offset.y = _texHeight * _alignY / h;

	return spriteID;
}

void SpriteRenderer::newInstance(int _spriteID, const ei::Vec3& _position, float _rotation, const ei::Vec2& _scale)
{
	SpriteInstance instance;
	const SpriteDef& sp = m_sprites[_spriteID];
	instance.sprite = sp.sprite;
	instance.position = _position + Vec3(sp.offset, 0.0f);
	instance.rotation = _rotation;
	instance.scale = _scale;
	m_dirty = true;
}

void SpriteRenderer::clearInstances()
{
	m_instances.clear();
	m_dirty = true;
}

void SpriteRenderer::draw() const
{
	// Update instance data each frame - it could be dynamic
	if(m_dirty)
	{
		glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
		glCall(glBufferData, GL_ARRAY_BUFFER, m_instances.size() * sizeof(SpriteInstance), m_instances.data(), GL_DYNAMIC_DRAW);
		m_dirty = false;
	}

	glCall(glBindVertexArray, m_vao);
	glCall(glDrawArrays, GL_POINTS, 0, m_instances.size());
	//glCall(glDrawArraysInstanced, GL_POINTS, 0, 1,
}

} // namespace MiR