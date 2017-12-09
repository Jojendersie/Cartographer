#pragma once

#include <ei/elementarytypes.hpp>

namespace ca { namespace cc {

	class Texture;

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

		/// Binding a framebuffer to draw into its textures.
		void bind();
		void unbind();

		/// Attach a 2D texture or a layer of another texure as depth buffer.
		///	The texture format must be compatible (e.g. D32F).
		void attachDepth(Texture & _texture, int _mipLevel = 0);

		void attach(int _colorAttachmentIdx, Texture & _texture, int _mipLevel = 0);

		/// Render one of the textures with a screenfilling quad.
		/// \param [in] _attachment GL_DEPTH_ATTACHMENT or GL_COLOR_ATTACHMENTi
		void show(uint _attachment);
	private:
		uint m_fboID;
		uint m_drawBuffers[16];
		int m_maxUsedIndex;
		// Following handles are stored for the show function
		uint m_depthTexture;
		uint m_colorTextures[16];
	};

}} // namespace ca::cc
