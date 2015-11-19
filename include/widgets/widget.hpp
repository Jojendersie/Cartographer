#pragma once

#include <memory>
#include "properties/anchorable.hpp"
#include "properties/clickable.hpp"
#include "properties/moveable.hpp"
#include "properties/refframe.hpp"
#include "properties/resizeable.hpp"

namespace cag {

	/// Base class with mandatory attributes for all widgets.
	/// \details A widget only contains the state. State handling in general is up to the derived
	///		elements.
	class Widget
	{
	public:
		Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable, bool _inputReceivable, bool _focusable);
		~Widget();

		/// Draw the element now
		virtual void draw() = 0;

		/// Process mouse input if desired.
		/// \details The Widget implementation forwards input to click-, resize- and move-
		///		components (in this order). It should be called from any inheriting class.
		/// \return True if this component absorbed the input. If any compontent returns true
		///		no other component will be checked afterwards.
		virtual bool processInput(const struct MouseState& _mouseState);

		/// Can this element receive input?
		bool isInputReceivable() const { return m_inputReceivable; }

		/// Can this element currently receive input (enabled)?
		bool isActive() const { return m_active; }
		void enable() { m_active = true; }
		void disable() { m_active = false; }
		void setActive(bool _state) { m_active = _state; }

		/// Can this element get the focus?
		bool isFocusable() const { return m_focusable; }

	protected:
		const bool m_inputReceivable;	///< The element can receive input.
		bool m_active;		///< The element can currently receive input (not disabled).
		bool m_focusable;	///< Can this object have the focus?
//		bool m_focus;		///< The element has the focus (only active elements can get the focus)
//		bool m_mouseOver;	///< The cursor hovers over the element

		RefFrame m_refFrame;
		// List of optional components (can be nullptr)
		Anchorable* m_anchorComponent;
		Clickable* m_clickComponent;
		Moveable* m_moveComponent;
		Resizeable* m_resizeComponent;

		Widget* m_parent;
		friend class Frame; // Parent must be set from someone
	};

	typedef std::shared_ptr<Widget> WidgetPtr;

} // namespace cag