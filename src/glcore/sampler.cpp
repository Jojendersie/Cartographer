#include "ca/cc/glcore/opengl.hpp"
#include "ca/cc/glcore/sampler.hpp"

namespace ca { namespace cc {

	Sampler::Sampler(Filter _minFilter, Filter _magFilter, Filter _mipFilter, Border _borderHandling, uint _maxAnisotropy) :
		m_minFilter(_minFilter),
		m_magFilter(_magFilter),
		m_mipFilter(_mipFilter),
		m_borderHandling(_borderHandling),
		m_maxAnisotropy(_maxAnisotropy)
	{
		glCall(glGenSamplers, 1, &m_samplerID);
		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_WRAP_S, static_cast<GLint>(_borderHandling));
		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_WRAP_T, static_cast<GLint>(_borderHandling));
		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_WRAP_R, static_cast<GLint>(_borderHandling));

		GLint minFilterGl;
		if (_minFilter == Filter::POINT)
			minFilterGl = _mipFilter == Filter::POINT ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
		else
			minFilterGl = _mipFilter == Filter::POINT ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;

		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_MIN_FILTER, minFilterGl);
		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_MAG_FILTER, _magFilter == Filter::POINT ? GL_NEAREST : GL_LINEAR);

		//GL_CALL(glSamplerParameterf, m_samplerId, GL_TEXTURE_MIN_LOD, desc.minLod);
		//GL_CALL(glSamplerParameterf, m_samplerId, GL_TEXTURE_MAX_LOD, desc.maxLod);

		glCall(glSamplerParameteri, m_samplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);

		//GL_CALL(glSamplerParameterfv, m_samplerId, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&desc.borderColor));

		/*if (desc.compareMode != CompareMode::NONE)
		{
			GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(desc.compareMode));
		}*/
	}

	Sampler::~Sampler()
	{
		glCall(glDeleteSamplers, 1, &m_samplerID);
	}

	void Sampler::bind(uint _textureSlot) const
	{
		// TODO: check binding
		glCall(glBindSampler, _textureSlot, m_samplerID);
	}

}} // namespace ca::cc
