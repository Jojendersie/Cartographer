#pragma once

#include "widgets/widget.hpp"

namespace cag {

	/// Manages global properties like look-and-feel and forwards input to the GUIs.
	class GUIManagar
	{
	public:
		/// Convert a pixel position to [0,1]^2 screen position.
		//static ei::Vec2 pixelToScreen(const ei::Vec2& _pixel);

		/// Convert a [0,1]^2 screen position to pixel position.
		//static ei::Vec2 screenToPixel(const ei::Vec2& _screenPos);

		/// Resize/Repositioninng of all GUI components.
		static void onResize(int _width, int _height);

		/// Does a specific element has the focus?
		/// \details Only one element can have the focus at a time. The focus can be changed by
		///		by mouse clicks or by different key events. For keys the GUI must define handlers
		///		on its own (Tab, Arrowkeys...).
		static bool hasFocus(WidgetPtr& _widget);
		static bool hasFocus(Widget* _widget);
		/// Get the element with the focus. Can return a nullptr.
		static WidgetPtr getFocussed();
		//static setFocus();

		/// Is a specific element pointed by the mouse.
		/// \details Only one component can have the mouse-over property. It is the element, which
		//static bool isMouseOver(WidgetPtr& _widget);

		/// Get the render backend for elementary draw opertations
		static class IRenderBackend* getRenderBackend();
		/// Get the theme which handles high level draw operations
		static class ITheme* getTheme();

		/// Get the current GUI width in pixels.
		/// \details The GUI size is the size which was given in onResize. A GUI may also be
		///		created for a part of the real app, but usually it is expected to cover the entire
		///		frame buffer.
		static int getWidth();

		/// Get the current GUI height in pixels.
		/// \details The GUI size is the size which was given in onResize. A GUI may also be
		///		created for a part of the real app, but usually it is expected to cover the entire
		///		frame buffer.
		static int getHeight();
	};

} // namespace cag