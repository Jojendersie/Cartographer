#pragma once

#include "widgets/widget.hpp"
#include <stack>

namespace ca { namespace gui { // namespace ca::gui { will maybe possible in C++17

	/// Manages global properties like look-and-feel and forwards input to the GUIs.
	class GUIManager
	{
	public:
		/// Init the manager with existing backends.
		/// \param [in] _width Window width in pixels. This should be the same number as used in
		///		onResize().
		/// \param [in] _height Window height in pixels. This should be the same number as used in
		///		onResize().
		static void init(std::shared_ptr<class IRenderBackend> _renderer, std::shared_ptr<class ITheme> _theme, int _width, int _height);
		static void exit();

		/// Resize/Repositioninng of all GUI components.
		static void onResize(int _width, int _height);

		/// Add a new child element. It is recommended to add single top level frames as entire
		/// GUIs. Those program state switches can be done by simple hide/show top level widgets.
		static void add(WidgetPtr _widget);

		/// Find and remove a widget (O(n) with n = number of elements)
		static void remove(WidgetPtr _widget);

		/// Render all content
		static void draw();

		/// Restrict rendering to a subarea.
		/// \details Clipping areas are handled on a stack. The current real region is the minimum
		///		over all elements.
		/// \return true if the new set region is non-empty (due to recursive restrictions).
		static bool pushClipRegion(const RefFrame& _rect);
		static void popClipRegion();
		/// Check if a rectangle is entirely clipped by the current region
		static bool isClipped(const RefFrame& _rect);

		/// Process mouse input.
		/// \return true if the input was consumed by any element in the GUI.
		static bool processInput(const struct MouseState& _mouseState);

		static const struct MouseState& getMouseState();

		/// Does a specific element has the focus?
		/// \details Only one element can have the focus at a time. The focus can be changed by
		///		by mouse clicks or by different key events. For keys the GUI must define handlers
		///		on its own (Tab, Arrowkeys...).
		static bool hasFocus(const WidgetPtr& _widget);
		static bool hasFocus(const Widget* _widget);
		/// Get the element with the focus. Can return a nullptr.
		static WidgetPtr getFocussed();
		/// Set an element as the focussed one and release the last one.
		static void setFocus(WidgetPtr _widget);

		/// Access the render backend for elementary draw opertations
		static class IRenderBackend& renderBackend();
		/// Access the theme which handles high level draw operations
		static class ITheme& theme();

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

		/// Globally check all elements for changes and cause a refiting of sizes and positions.
		/// \details This method blocks recursive calls. I.e. if a resize causes another refit
		///		this is logged and executed later until convergence or at most three times.
		static void refitAllToAnchors();
	private:
		std::shared_ptr<class IRenderBackend> m_renderer;
		std::shared_ptr<class ITheme> m_theme;
		std::shared_ptr<class Frame> m_topFrame;

		std::stack<ei::IVec4, std::vector<ei::IVec4>> m_clipRegionStack;
		MouseState m_mouseState;		///< Buffer the last seen mouse state
	};

}} // namespace ca::gui