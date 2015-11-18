#pragma once

#include "widgets/widget.hpp"

namespace cag {

	/// Manages global properties like look-and-feel and forwards input to the GUIs.
	class GUIManagar
	{
	public:
		static void init(class IRenderBackend* _renderer, class ITheme* _theme);
		static void exit();

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
		/// Set an element as the focussed one and release the last one.
		static void setFocus(WidgetPtr _widget);

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