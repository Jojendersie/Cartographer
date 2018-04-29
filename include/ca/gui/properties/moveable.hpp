#pragma once

#include <functional>
#include "ca/gui/backend/mouse.hpp"

namespace ca { namespace gui {

	/// A moveable component is moved when the left mouse button is pushed while beeing over the
	/// component and then moved.
	class Moveable: public IMouseProcessAble
	{
	public:
		/// Make a reference frame moveable.
		/// \param [in] _thisWidget Widget where this component belongs to. Uses reference frame
		///		and anchorable component of the widget. Since positioning of anchorables is
		///		done automatically it must be reset/disabled by the moveable. Non-anchorable
		///		widgets can pass a nullptr others should always provide access to their
		///		anchorable component.
		Moveable(class Widget * _thisWidget);

		/// Process mouse input for drag & drop like movement.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		virtual bool processInput(class Widget & _thisWidget, const struct MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput) override;

		bool isMoving() const { return m_moving; }

		/// Given a continous position compute some restricted position which should be
		/// used instead.
		typedef std::function<Coord2(const Coord2&)> RestrictionFunction;

		/// Snapping: set a condition which transforms a continous position into some
		/// restricted position.
		/// Automatically enables the restriction.
		/// TODO: premade snapping functions e.g. for anchorProviders.
		void restrictMovement(RestrictionFunction _restrictionFunction);
		/// (Temporarly) enable or disable the restriction (snapping) function.
		void setEnableRestriction(bool _enable) { m_useRestriction = _enable; }

		bool isMovingEnabled() const { return m_movingEnabled; }
		void setMoveable(bool _enable) { m_movingEnabled = _enable; }
	private:
		bool m_movingEnabled;
		bool m_moving;	/// Currently actively moving
		bool m_useRestriction;

		RestrictionFunction m_snapFunction;
		Coord2 m_floatingPosition;	/// Current continous position - may differ from the widged position if snapping is active.
	};

}} // namespace ca::gui
