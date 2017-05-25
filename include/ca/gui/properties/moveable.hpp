#pragma once

#include <functional>

namespace ca { namespace gui {

	/// A moveable component is moved when the left mouse button is pushed while beeing over the
	/// component and then moved.
	class Moveable
	{
	public:
		/// Make a reference frame moveable.
		/// \param [in,opt] _anchorable Since positioning of anchorables is done automaically
		///		it must be reset/disabled by the moveable. Non-anchorable widgets can pass a
		///		nullptr others should always provide access to their anchorable component.
		Moveable(class RefFrame* _selfFrame, class Anchorable* _anchorable);

		/// Process mouse input for drag & drop like movement.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		bool processInput(const struct MouseState& _mouseState);

		bool isMoving() const { return m_moving; }

		/// Should the anchor component change update this component.
		/// \param [in] _anchorable The new anchor component or new if there is none afterwards.
		void registerAnchorCompoentent(class Anchorable* _anchorable) { m_anchorable = _anchorable; }

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
	private:
		class RefFrame* m_refFrame;
		class Anchorable* m_anchorable;
		bool m_moving;	/// Currently actively moving
		bool m_useRestriction;

		RestrictionFunction m_snapFunction;
		Coord2 m_floatingPosition;	/// Current continous position - may differ from the widged position if snapping is active.
	};

}} // namespace ca::gui
