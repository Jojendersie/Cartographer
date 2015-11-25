#pragma once

#include <memory>
#include "properties/anchorable.hpp"
#include "properties/clickable.hpp"
#include "properties/moveable.hpp"
#include "properties/refframe.hpp"
#include "properties/resizeable.hpp"

namespace ca { namespace gui {

	/// Base class with mandatory attributes for all widgets.
	/// \details A widget only contains the state. State handling in general is up to the derived
	///		elements.
	class Widget
	{
	public:
		Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable, bool _inputReceivable, bool _focusable);
		virtual ~Widget();

		/// Set the button width and heigh (resets anchoring)
		void setSize(const Coord2& _size);
		Coord2 getSize() const;

		/// Set the position (resets anchoring)
		void setPosition(const Coord2& _position);
		Coord2 getPosition() const;

		/// Set both: position and size (resets anchoring)
		void setExtent(const Coord2& _position, const Coord2& _size);

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
		bool isEnabled() const { return m_enabled; }
		void enable() { m_enabled = true; }
		void disable() { m_enabled = false; }
		void setEnabled(bool _state) { m_enabled = _state; }

		/// Is the current element visible/hidden?
		bool isVisible() const { return m_visible; }
		void show() { m_visible = true; }
		void hide() { m_visible = false; }
		void setVisible(bool _state) { m_visible = _state; }

		/// Can this element get the focus?
		bool isFocusable() const { return m_focusable; }

		const Widget* parent() const { return m_parent; }
		Widget* parent() { return m_parent; }

		/// Elements which contain other elements should implement this check for the focus of a
		/// contained element.
		virtual bool isChildFocused(const Widget* _child) const { return false; }

		/// Check if this widget consumed the last mouse input
		bool hasMouseFocus() const { return m_mouseFocus == this; }
	protected:
		const bool m_inputReceivable;	///< The element can receive input.
		bool m_enabled;		///< The element can currently receive input (not disabled).
		bool m_focusable;	///< Can this object have the focus?
		bool m_visible;		///< Draw the element if visible and mask input otherwise.
		static const Widget* m_mouseFocus;	///< Shared register for the component which has the mouse focus last time

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

}} // namespace ca::gui