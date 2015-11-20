#pragma once

#include <ei/2dtypes.hpp>

namespace cag {

	/// Interface for elementary rendering opertions.
	class IRenderBackend
	{
	public:
		/// The gradient mode defines how points A and B define the gradient.
		enum class GradientMode
		{
			LINEAR,			///< Interpolate linear between the two points and extend the same values to infinity on perpendicular lines between the points.
			CIRCULAR,		///< Take A as the center and B as a point on the circumcirle. Intepolate colors according to the radius
			RECTANGULAR,	///< A defines the center and B an arbitrary corner of a rectangle. Interpolation goes linear towards edges.
		};

		// TODO: think about coordinate system for the renderer!
		// [0,1] -> intuitive rendering implementation, theme (or widgets) must handle conversion
		// Internal system -> render-backend must handle conversion

		// TODO: think about overdraw handling (enable z and draw compontents front to back could be better, but confusing within a component)
		// Transparency would not stack (semi transparent button on opaque background is transparant).
		// Maybe use back to front along hierarchy? and z inside compontents?

		virtual void setClippingRegion(int _l, int _r, int _b, int _t) = 0;
		virtual void drawText(const ei::Vec2& _position, const char* _text, float _size, const ei::Vec4& _color, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f, bool _roundToPixel = false) = 0;
		virtual void drawRect(const class RefFrame& _rect, const ei::Vec4& _color) = 0;
		virtual void drawRect(const class RefFrame& _rect, const ei::Vec3& _a, const ei::Vec3& _b, const ei::Vec4& _colorA, const ei::Vec4& _colorB, GradientMode _mode = GradientMode::LINEAR) = 0;
		virtual void drawRect(const class RefFrame& _rect, uint64 _customShader) = 0;
		virtual void drawTextureRect(const class RefFrame& _rect, uint64 _texture, float _opacity) = 0;
		virtual void drawTriangle(const ei::Triangle2D& _triangle, const ei::Vec4& _color) = 0;

		/// Load or find a loaded texture.
		/// \return Some handle to reference the texture for the draw commands. The handle can
		///		be chosen implementation dependent but 0 must always be the null-reference.
		virtual uint64 getTexture(const char* _textureFile) = 0;

		/// Create a program with a custom (pixel) shader
		virtual uint64 getShader(const char* _shader) = 0;
	};

} // namespace cag