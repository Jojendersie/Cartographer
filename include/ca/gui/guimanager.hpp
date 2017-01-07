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
		/// \param [in] _innerLayer Stir rendering order within the component.
		///		The larger the layer the later is the element drawn (more in foreground).
		static void add(WidgetPtr _widget, unsigned _innerLayer = 0);

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
		static bool hasKeyboardFocus(const WidgetPtr& _widget);
		static bool hasKeyboardFocus(const Widget* _widget);
		static bool hasMouseFocus(const WidgetPtr& _widget);
		static bool hasMouseFocus(const Widget* _widget);
		/// Get the element with the focus. Can return a nullptr.
		static Widget* getKeyboardFocussed();
		static Widget* getStickyKeyboardFocussed();
		static Widget* getMouseFocussed();
		static Widget* getStickyMouseFocussed();

		/// Set an element as the focussed one and release the last one.
		static void setKeyboardFocus(Widget* _widget, bool _sticky = false);
		static void setMouseFocus(Widget* _widget, bool _sticky = false);

		/// What cursor should actually be used?
		static CursorType getCursorType();

		/// Set the desired cursor shape, it is reset every frame.
		static void setCursorType(CursorType _type);

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
	private:
		std::shared_ptr<class IRenderBackend> m_renderer;
		std::shared_ptr<class ITheme> m_theme;
		std::shared_ptr<class Group> m_topFrame;

		Widget* m_keyboardFocus;		///< The element with the keyboard-focus or nullptr
		Widget* m_mouseFocus;			///< The element with the mouse-focus or nullptr
		bool m_stickyKeyboardFocus;		///< The focus guarantees exclusive input handling by the element. The element must release this active.
		bool m_stickyMouseFocus[2];		///< The focus guarantees exclusive input handling by the element.

		std::stack<ei::IVec4, std::vector<ei::IVec4>> m_clipRegionStack;
		MouseState m_mouseState;		///< Buffer the last seen mouse state
		CursorType m_cursorType;		///< Current desired cursor shape

		/// Globally check all elements for changes and cause a refitting of sizes and positions.
		/// \details This method blocks recursive calls. I.e. if a resize causes another refit
		///		this is logged and executed later until convergence or at most three times.
		static void refitAllToAnchors();
	};

}} // namespace ca::gui