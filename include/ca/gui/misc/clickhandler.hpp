#pragma once

namespace ca { namespace gui {

	/// Helper class to detect click and double click events.
	class ClickHandler
	{
	public:
		ClickHandler();

		/// Checks for click and dbl-click events and sets the according button states
		/// within the given mouseState.
		void handleUpEvents(struct MouseState& _mouseState);

		/// Reset which widget (if any) currently received the down event for later
		/// detection of up events.
		void handleDownEvents(const struct MouseState& _mouseState, const class Widget* _widget);
	private:
		const class Widget* m_mouseDown;	///< The widget that received the last button down event (if there was any)
		bool m_buttonDownReceived[8];		///< Detect for each button, if it was pressed on the m_mouseDown element (for CLICK events)
		float m_lastClick[8];				///< Time stamp for the last click to detect DBL_CLICK events
	};
}} // namespace ca::gui