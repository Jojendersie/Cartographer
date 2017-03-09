#pragma once

#include <functional>
#include <vector>
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	/// Component for clickable objects defining input and callback handling.
	/// \details Per default a clickable uses the objects reference frame as input region.
	class Clickable
	{
	public:
		Clickable(class Widget* _thisWidget);
		~Clickable();

		/// Process mouse input (uses the callbacks).
		bool processInput(const struct MouseState& _mouseState);

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

		bool isAnyButtonDown() const { for(int i = 0; i < 8; ++i) if(m_buttonDownReceived[i]) return true; return false; }
	protected:
		class Widget* m_widget;
		std::vector<OnButtonChange> m_changeFuncs;
		std::vector<MouseState::ButtonState> m_statesMasks; // Call callbacks only for desired events
		bool m_buttonDownReceived[8];	// Detect for each button, if it was pressed on this element (for CLICK events)
		float m_lastClick[8];			// Time stamp for the last click to detect DBL_CLICK events
	};

	/// Use the reference frame fitted ellipse to detect click events.
	class EllipseRegion : public IRegion
	{
	public:
		/// \param [in] _selfFrame The reference frame for the clickable area
		EllipseRegion(const RefFrame* _selfFrame);

		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	private:
		const RefFrame* m_selfFrame;
	};

	/// Use a bitmap mask to detect click events.
	class MaskRegion : public IRegion
	{
	public:
		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	};

}} // namespace ca::gui
