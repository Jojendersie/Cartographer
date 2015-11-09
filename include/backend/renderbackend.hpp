#pragma once

#include <ei/vector.hpp>

namespace cag {

	/// Interface for elementary rendering opertions.
	class IRenderBackend
	{
	public:
		virtual void setClippingRegion(int _l, int _r, int _b, int _t) = 0;
		virtual void drawText(const ei::Vec2& _position, const char* _text, float _size, const ei::Vec4& _color, float _rotation = 0.0f, float _alignX = 0.0f, float _alignY = 0.0f, bool _roundToPixel = false) = 0;
		virtual void drawRect(const class RefFrame& _rect, uint64 _customShader) = 0;
		virtual void drawTextureRect(const class RefFrame& _rect, uint64 _texture) = 0;

		/// Load or find a loaded texture.
		/// \return Some handle to reference the texture for the draw commands. The handle can
		///		be chosen implementation dependent but 0 must always be the null-reference.
		virtual uint64 getTexture(const char* _textureFile) = 0;

		/// Create a program with a custom (pixel) shader
		virtual uint64 getShader(const char* _shader) = 0;
	};

} // namespace cag