#pragma once

namespace cag {

	/// Basic rectangular area which is used as reference from all widgets.
	class RefFrame
	{
	public:
		float left;
		float right;
		float bottom;
		float top;

		/// Check if the mouse cursor is on this reference frame.
		/// \param [in] _mousePos Position of the cursor in screenspace [0,1]^2.
		bool isMouseOver(const Vec2& _mousePos);
	};

} // namespace cag