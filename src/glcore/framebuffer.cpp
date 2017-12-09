#include "ca/cc/glcore/framebuffer.hpp"
#include "ca/cc/glcore/opengl.hpp"
#include "ca/cc/glcore/texture.hpp"
#include "ca/cc/glcore/shader.hpp"
#include "ca/cc/glcore/device.hpp"

namespace ca { namespace cc {

	FrameBuffer::FrameBuffer()
	{
		glCall(glCreateFramebuffers, 1, & m_fboID);
		for(int i = 0; i < 16; ++i)
		{
			m_drawBuffers[i] = GL_NONE;
			m_colorTextures[i] = 0;
		}
		m_depthTexture = 0;
		m_maxUsedIndex = -1;
	}

	FrameBuffer::~FrameBuffer()
	{
		glCall(glDeleteFramebuffers, 1, & m_fboID);
	}

	FrameBuffer::FrameBuffer(FrameBuffer && _other) :
		m_fboID(_other.m_fboID)
	{
		_other.m_fboID = 0;
	}

	FrameBuffer & FrameBuffer::operator=(FrameBuffer && _rhs)
	{
		this->~FrameBuffer();
		new (this) FrameBuffer (std::move(_rhs));
		return * this;
	}

	void FrameBuffer::bind()
	{
		glCall(glBindFramebuffer, GL_FRAMEBUFFER, m_fboID);
	}

	void FrameBuffer::unbind()
	{
		glCall(glBindFramebuffer, GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::attachDepth(Texture & _texture, int _mipLevel)
	{
		glCall(glNamedFramebufferTexture, m_fboID, GL_DEPTH_ATTACHMENT, _texture.getID(), _mipLevel);
		m_depthTexture = _texture.getID();
	}

	void FrameBuffer::attach(int _colorAttachmentIdx, Texture & _texture, int _mipLevel)
	{
		glCall(glNamedFramebufferTexture, m_fboID, GL_COLOR_ATTACHMENT0 + _colorAttachmentIdx,
			_texture.getID(), _mipLevel);
		m_drawBuffers[_colorAttachmentIdx] = GL_COLOR_ATTACHMENT0 + _colorAttachmentIdx;
		m_maxUsedIndex = ei::max(m_maxUsedIndex, _colorAttachmentIdx);
		glNamedFramebufferDrawBuffers(m_fboID, m_maxUsedIndex + 1, m_drawBuffers);
		m_colorTextures[_colorAttachmentIdx] = _texture.getID();
	}

	void FrameBuffer::show(uint _attachment)
	{
		static Program s_shader;
		// Compile the shader if necessary
		if(s_shader.getID() == 0)
		{
			const char * VS_FULLSCREEN_TRIANGLE = R"(
				#version 330

				void main()
				{
					gl_Position = vec4((gl_VertexID % 2) * 4.0 - 1.0,
									   (gl_VertexID / 2) * 4.0 - 1.0, 0.0, 1.0);
				}
			)";
			const char * FS_SHOW_TARGET = R"(
				#version 420
				layout(binding = 0) uniform sampler2D tex;
				layout(location = 0) out vec4 fragColor;

				void main()
				{
					fragColor = vec4(texelFetch(tex, ivec2(gl_FragCoord.xy), 0).rgb, 1.0);
				}
			)";
			s_shader.attach( cc::ShaderManager::get(VS_FULLSCREEN_TRIANGLE, ShaderType::VERTEX, false) );
			s_shader.attach( cc::ShaderManager::get(FS_SHOW_TARGET, ShaderType::FRAGMENT, false) );
			s_shader.link();
		}

		if(_attachment == GL_DEPTH_ATTACHMENT)
			glBindTextureUnit(0, m_depthTexture);
		else glBindTextureUnit(0, m_colorTextures[_attachment - GL_COLOR_ATTACHMENT0]);
		s_shader.use();
		Device::setZFunc(cc::ComparisonFunc::ALWAYS);
		Device::setZWrite(false);
		Device::setCullMode(CullMode::NONE);
		glCall(glDrawArrays, GL_TRIANGLE_STRIP, 0, 3);
	}

}}
