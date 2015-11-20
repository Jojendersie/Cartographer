#include "spriterenderer.hpp"
#include "glcore/opengl.hpp"
#include "glcore/vertexformat.hpp"
#include "core/error.hpp"

using namespace ei;

namespace ca { namespace cc {

SpriteRenderer::SpriteRenderer()
{
	glCall(glGenVertexArrays, 1, &m_vao);
	glCall(glBindVertexArray, m_vao);
	// Create buffer without data for now
	glCall(glGenBuffers, 1, &m_vbo);
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

	// 4 x uint16 for texture l, u, r and b
	glCall(glEnableVertexAttribArray, 0);
	glCall(glVertexAttribPointer, 0, 4, GLenum(PrimitiveFormat::UINT16), GL_TRUE, 52, (GLvoid*)(0));
	// 1 x uint64 bindless handle
	glCall(glEnableVertexAttribArray, 1);
	glCall(glVertexAttribIPointer, 1, 2, GLenum(PrimitiveFormat::UINT32), 52, (GLvoid*)(8));
	// 2 x int16 for num-tiles
	glCall(glEnableVertexAttribArray, 2);
	glCall(glVertexAttribIPointer, 2, 2, GLenum(PrimitiveFormat::UINT16), 52, (GLvoid*)(16));
	// 3 x float position in world space
	glCall(glEnableVertexAttribArray, 3);
	glCall(glVertexAttribPointer, 3, 3, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 52, (GLvoid*)(20));
	// 1 x float rotation
	glCall(glEnableVertexAttribArray, 4);
	glCall(glVertexAttribPointer, 4, 1, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 52, (GLvoid*)(32));
	// 4 x half float scale
	glCall(glEnableVertexAttribArray, 5);
	glCall(glVertexAttribPointer, 5, 4, GLenum(PrimitiveFormat::HALF), GL_FALSE, 52, (GLvoid*)(36));
	// 2 x float animation
	glCall(glEnableVertexAttribArray, 6);
	glCall(glVertexAttribPointer, 6, 2, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 52, (GLvoid*)(44));
	
	
	// TODO: test dividing the buffers into two (instancing)
}

int SpriteRenderer::defSprite(float _alignX, float _alignY,
			  Texture2D::Handle _textureHandle, int _texX, int _texY, int _texWidth, int _texHeight,
			  int _numX, int _numY)
{
	// TODO: error handling for parameters

	int spriteID = (int)m_sprites.size();
	SpriteDef newSprite;

	float w = (float)_textureHandle->getWidth();
	float h = (float)_textureHandle->getHeight();

	if(_texWidth == -1) _texWidth = _textureHandle->getWidth();
	if(_texHeight == -1) _texHeight = _textureHandle->getHeight();

	newSprite.sprite.texCoords.x = uint16((_texX + 0.0f) / w * 65535);
	newSprite.sprite.texCoords.w = uint16((_texY + 0.0f) / h * 65535);
	newSprite.sprite.texCoords.z = uint16((_texX - 0.0f + _texWidth) / w * 65535);
	newSprite.sprite.texCoords.y = uint16((_texY - 0.0f + _texHeight) / h * 65535);
	newSprite.sprite.texture = _textureHandle->getGPUHandle();
	newSprite.sprite.numTiles.x = _numX;
	newSprite.sprite.numTiles.y = _numY;
	newSprite.offset.x = -_texWidth * _alignX;
	newSprite.offset.y = -_texHeight * _alignY;
	newSprite.size.x = _texWidth;
	newSprite.size.y = _texHeight;

	m_sprites.push_back(newSprite);
	return spriteID;
}

void SpriteRenderer::newInstance(int _spriteID, const ei::Vec3& _position, float _rotation, const ei::Vec2& _scale, float _animX, float _animY)
{
	int t = sizeof(SpriteRenderer::Sprite);
	SpriteInstance instance;
	const SpriteDef& sp = m_sprites[_spriteID];
	instance.sprite = sp.sprite;
	instance.position = _position;// + Vec3(sp.offset * _scale, 0.0f);
	//instance.position.z = -1.0f + instance.position.z;
	instance.rotation = _rotation;
	Vec2 minPos = _scale * (sp.offset);
	Vec2 maxPos = _scale * (sp.size + sp.offset);
	instance.scale = Vec<half,4>(toHalf(minPos.x), toHalf(minPos.y), toHalf(maxPos.x), toHalf(maxPos.y));
	if(sp.sprite.numTiles.x > 1) instance.animation.x = fmod(_animX, (float)sp.sprite.numTiles.x);
	else instance.animation.x = 0.0f;
	if(sp.sprite.numTiles.y > 1) instance.animation.y = fmod(_animY, (float)sp.sprite.numTiles.y);
	else instance.animation.y = 0.0f;

	m_instances.push_back(instance);
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

}} // namespace ca::cc
