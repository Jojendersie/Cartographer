#pragma once

#include <ei/2dtypes.hpp>

namespace ca { namespace gui {

	/// Interface for elementary rendering opertions.
	/// \details The renderer must support the rendering of graphic primitives up to text. This
	///		abstraction layer allows that the GUI is independent of the API. Even a GPU renderer
	///		is thinkable.
	///
	///		The coordinate system in which the render works is a [0,width]x[0,height] pixelspace
	///		with the origin in the bottom left corner. To convert this into relative coordinates
	///		width and height can be queried from the GUIManager.
	///
	///		It is possible to implement special backends with more commands for special themes.
	///		However this is as minimalistic as possible to allow easier integration.
	class IRenderBackend
	{
	public:
		virtual ~IRenderBackend() {}

		/// The gradient mode defines how points A and B define the gradient.
		enum class GradientMode
		{
			LINEAR,			///< Interpolate linear between the two points and extend the same values to infinity on perpendicular lines between the points.
			CIRCULAR,		///< Take A as the center and B as a point on the circumcirle. Intepolate colors according to the radius
			RECTANGULAR,	///< A defines an overfill offset up to which distance ColorA should be used and B an underfill offset how fast it should end before reaching colorB.
		};

		/// Prepare the render pipeline for upcomming GUI draw-calls.
		virtual void beginDraw() = 0;
		/// The gui code has finished all its rendering. Restore pipeline or make real draw
		///	submissions if not already done.
		virtual void endDraw() = 0;

		/// Explicit barrier to guarantee draw order.
		/// \details Everything before beginLayer() must be visually behind everything which comes
		///		after the call. Due to transparancies in the GUI or at least in the font rendering
		///		a Z or Stencil buffer cannot be used to achieve this, except you know your theme
		///		allows at most binary alpha-tests and no transparency otherwise.
		///
		///		Additionally a clipping region is defined to allow correct clipping at container
		///		borders.
		/// \param [in] _clippingRect Left, right, bottom, top clipping coordinate (in this order).
		virtual void beginLayer(const ei::IVec4& _clippingRect) = 0;

		virtual void drawText(const ei::Vec2& _position, const char* _text, float _size, const ei::Vec4& _color, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f, bool _roundToPixel = false) = 0;
		/// Simulate a text rendering and get the bounding box of the resulting drawn text.
		virtual ei::Rect2D getTextBB(const ei::Vec2& _position, const char* _text, float _size, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f, bool _roundToPixel = false) = 0;
		/// Simulate a text rendering and get the closest character to the cursor.
		virtual uint getTextCharacterPosition(const ei::Vec2& _findPosition, const ei::Vec2& _textPosition, const char* _text, float _size, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f, bool _roundToPixel = false) = 0;

		virtual void drawRect(const ei::Rect2D& _rect, const ei::Vec4& _color) = 0;
		/// \param [in] _a Reference rect relative coordinate in [0,1]^2 as gradient source.
		/// \param [in] _b Reference rect relative coordinate in [0,1]^2 as gradient target.
		virtual void drawRect(const ei::Rect2D& _rect, const ei::Vec2& _a, const ei::Vec2& _b, const ei::Vec4& _colorA, const ei::Vec4& _colorB, GradientMode _mode = GradientMode::LINEAR) = 0;
		/// \param [in] _tiling Repeat the texture (pixel perfect without scale).
		virtual void drawTextureRect(const ei::Rect2D& _rect, uint64 _texture, float _opacity, bool _tiling) = 0;
		/// \param [in] _colorX Color of the vertex with index X.
		virtual void drawTriangle(const ei::Triangle2D& _triangle, const ei::Vec4& _color0, const ei::Vec4& _color1, const ei::Vec4& _color2) = 0;
		/// Draw a polygon with a color gradient from the first to the last vertex.
		virtual void drawLine(const ei::Vec2* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB) = 0;

		/// Load or find a loaded texture.
		/// \param [in] _smooth Use linear interpolation for the texture magnificiton/minification.
		///		If false point sampling is used instead.
		/// \return Some handle to reference the texture for the draw commands. The handle can
		///		be chosen implementation dependent but 0 must always be the null-reference.
		virtual uint64 getTexture(const char* _textureFile, bool _smooth) = 0;

		/// Get the size of a loaded texture.
		virtual ei::IVec2 getTextureSize(uint64 _handle) = 0;
	};

}} // namespace ca::gui