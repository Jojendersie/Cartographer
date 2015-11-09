#pragma once

#include <functional>
#include <vector>
#include "backend/mouse.hpp"

namespace cag {

	/// Interface for clickable objects defining input and callback handling.
	class IClickable
	{
	public:
		/// Process mouse input (uses the callbacks).
		virtual void processInput(const struct MouseState& _mouseState);

		/// Type for click callbacks.
		/// \param [in] _where Screenspace position as info.
		/// \param [in] _button Index of the changed mouse button.
		/// \param [in] _state New state of the changed mouse button.
		typedef std::function<void(const Vec2& _where, int _button, MouseState::ButtonState _state)> OnButtonChange;

		/// Attach another callback to the element.
		/// \details It is not possible to detach function pointers.
		/// \param [in] _callback A new function which is called on any click event of this component.
		void addOnButtonChangeFunc(OnButtonChange _callback);

		/// Enable and disable certain button state events.
		/// \param [in] _states A bitmask which is 1 for each allowed state. I.e. the callback
		///		is called for all those states.
		void maskEvents(MouseState::ButtonState _states);
	protected:
		std::vector<OnButtonChange> m_changeFunc;
		MouseState::ButtonState m_allowedStates;
	};

	/// Use the reference frame to detect click events.
	class RectClickable: public IClickable
	{
	public:
		void processInput(const struct MouseState& _mouseState) override;
	};

	/// Use a bitmap mask to detect click events.
	class MaskClickable: public IClickable
	{
	public:
		void processInput(const struct MouseState& _mouseState) override;
	};

} // namespace cag