#pragma once

#include "widgets/widget.hpp"

namespace cag {

	/// Manages global properties like look-and-feel and forwards input to the GUIs.
	class GUIManagar
	{
	public:
		/// Convert a pixel position to [0,1]^2 screen position.
		static ei::Vec2 pixelToScreen(const ei::Vec2& _pixel);

		/// Convert a [0,1]^2 screen position to pixel position.
		static ei::Vec2 screenToPixel(const ei::Vec2& _screenPos);

		/// Resize/Repositioninng of all GUI components.
		static void onResize(int _width, int _height);

		/// Does a specific element has the focus?
		static bool hasFocus(WidgetPtr& _widget);
		/// Get the element with the focus. Can return a nullptr.
		static WidgetPtr getFocussed();
	};

} // namespace cag