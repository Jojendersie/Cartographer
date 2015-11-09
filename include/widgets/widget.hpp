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
		Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable);
		~Widget();

		/// Draw the element now
		virtual void draw() = 0;

		/// Process mouse input if desired.
		/// \return True if this component absorbed the input. If any compontent returns true
		///		no other component will be checked afterwards.
		virtual bool processInput(const struct MouseState& _mouseState) { return false; }

	protected:
		bool m_active;		///< The element can receive input.
//		bool m_focus;		///< The element has the focus (only active elements can get the focus)
//		bool m_mouseOver;	///< The cursor hovers over the element

		RefFrame m_refFrame;
		// List of optional components (can be nullptr)
		Anchorable* m_anchorComponent;
		Clickable* m_clickComponent;
		Moveable* m_moveComponent;
		Resizeable* m_resizeComponent;

		std::shared_ptr<Widget> m_parent;
	};

	typedef std::shared_ptr<Widget> WidgetPtr;

} // namespace cag