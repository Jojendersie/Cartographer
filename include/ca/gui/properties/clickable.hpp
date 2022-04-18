#pragma once

#include <functional>
#include <vector>
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	/// Component for clickable objects defining input and callback handling.
	/// \details Per default a clickable uses the objects reference frame as input region.
	class Clickable: public IMouseProcessAble
	{
	public:
		Clickable(class Widget* _thisWidget);
		~Clickable();

		/// Process mouse input (uses the callbacks).
		virtual bool processInput(class Widget& _thisWidget, const struct MouseState& _mouseState, bool _cursorOnWidget, bool& _ensureNextInput) override;

		/// Type for click callbacks.
		/// \param [in] _thisBtn A pointer to the widget for which the event is triggered.
		/// \param [in] _where Internal position as info.
		/// \param [in] _button Index of the changed mouse button.
		/// \param [in] _state New state of the changed mouse button.
		typedef std::function<void(class Widget* _thisBtn, const Coord2& _where, int _button, MouseState::ButtonState _state)> OnButtonChange;

		/// Attach another callback to the element.
		/// \details It is not possible to detach function pointers.
		/// \param [in] _callback A new function which is called on any click event of this component.
		/// \param [in] _stateMask A bitmask which is 1 for each allowed state. I.e. the callback
		///		is called for all those states.
		void addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask);

		/// Enable or disable clicking behavior only. Consider disabling the entire widget instead (performance).
		void setClickable(bool _enable) { m_clickingEnabled = _enable; }
		bool isClickingEnabled() const { return m_clickingEnabled; }
	protected:
		std::vector<OnButtonChange> m_changeFuncs;
		std::vector<MouseState::ButtonState> m_statesMasks; // Call callbacks only for desired events
		bool m_clickingEnabled;
		bool m_hasDoubleClickFunc;		// As long as there is no double click function count each click to trigger all the callbacks.
	};

}} // namespace ca::gui
