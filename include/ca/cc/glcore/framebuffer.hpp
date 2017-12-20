#pragma once

#include <ei/vector.hpp>

namespace ca { namespace cc {

	class Texture2D;

	/// A framebuffer is a collection of textures to render on.
	/// Renderbuffers are currently not supported.
	class FrameBuffer
	{
	public:
		uint getID() const { return m_fboID; }

		FrameBuffer();
		~FrameBuffer();
		FrameBuffer(const FrameBuffer &) = delete;
		FrameBuffer(FrameBuffer &&);
		FrameBuffer & operator = (const FrameBuffer &) = delete;
		FrameBuffer & operator = (FrameBuffer &&);

		/// Binding a framebuffer to draw into its textures and set the viewport size.
		void bind();
		void unbind();

		/// Attach a 2D texture or a layer of another texure as depth buffer.
		///	The texture format must be compatible (e.g. D32F).
		void attachDepth(Texture2D & _texture, int _mipLevel = 0);

		void attach(int _colorAttachmentIdx, Texture2D & _texture, int _mipLevel = 0);

		/// Render one of the textures with a screenfilling quad.
		/// \param [in] _attachment GL_DEPTH_ATTACHMENT or GL_COLOR_ATTACHMENTi
		void show(uint _attachment);
	private:
		uint m_fboID;
		uint m_drawBuffers[16];
		int m_maxUsedIndex;
		ei::IVec2 m_resolution;
		// Following handles are stored for the show function
		uint m_depthTexture;
		uint m_colorTextures[16];
	};

}} // namespace ca::cc
