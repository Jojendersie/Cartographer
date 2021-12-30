#pragma once

#include "widgets/widget.hpp"
#include "backend/keyboard.hpp"
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
		/// The named add allows to assign a unique value to each element to find it later.
		static void add(uint _name, WidgetPtr _widget, unsigned _innerLayer = 0);

		/// Find and remove a widget (O(n) with n = number of elements)
		static void remove(WidgetPtr _widget);

		/// Find a named element.
		WidgetPtr find(uint _name);

		/// Remove all current widgets.
		/// \details This can be usefull to switch between different GUIs. As long as you
		///		keep one WidgetPtr the component is not deleted and thus can be added back
		///		later.
		static void clear();

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
		/// Process keyboard input.
		static bool processInput(const struct KeyboardState& _keyboardState);

		static const struct MouseState& getMouseState();
		static const struct KeyboardState& getKeyboardState();

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
		static void setKeyboardFocus(Widget* _widget);
		static void setMouseFocus(Widget* _widget, bool _sticky = false);
		/// Overwrite the component with the one that has the mouse on it.
		/// \details While processing the input this gets constantly overwritten.
		///		The last compontent to write this must be the one deepest in the
		///		hierarchy.
		static void setMouseOver(Widget* _widget);

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

		/// Unhides and positionates a widget and starts tracking its visibility state.
		/// \details The popup is hidden if the mouse distance to the widget increases.
		///		All popups are tracked on a stack. Only the topmost one is hidden in one
		///		step. This allows chains of info popups.
		static void showPopup(WidgetPtr& _popup, const Widget* _originator);

		/// Set the time in seconds before a popup is shown
		static void setPopupTime(float _showAfter);

		/// Get the anchor component of the window sized top level component
		static const IAnchorProvider* getAnchorProvider();
	private:
		std::shared_ptr<class IRenderBackend> m_renderer;
		std::shared_ptr<class ITheme> m_theme;
		std::shared_ptr<class Group> m_topFrame;
		std::vector<WidgetPtr> m_popupStack;
		float m_cursorToPopupDistance;
		float m_popupTime;				///< Seconds when to show a popup

		Widget* m_keyboardFocus;		///< The element with the keyboard-focus or nullptr
		Widget* m_mouseFocus;			///< The element with the mouse-focus or nullptr
		Widget* m_mouseOver;			///< The element over which the mouse currently sits (can be a disabled element)
		//bool m_stickyKeyboardFocus;		///< The focus guarantees exclusive input handling by the element. The element must release this active.
		bool m_stickyMouseFocus;		///< The focus guarantees exclusive input handling by the element.
		float m_lastMouseMoveTime;		///< Point in time where the mouse was moved the last.

		std::stack<ei::IVec4, std::vector<ei::IVec4>> m_clipRegionStack;
		MouseState m_mouseState;		///< Buffer the last seen mouse state
		KeyboardState m_keyboardState;
		CursorType m_cursorType;		///< Current desired cursor shape

		/// Globally check all elements for changes and cause a refitting of sizes and positions.
		/// \details This method blocks recursive calls. I.e. if a resize causes another refit
		///		this is logged and executed later until convergence or at most three times.
		static void refitAllToAnchors();
	};

}} // namespace ca::gui