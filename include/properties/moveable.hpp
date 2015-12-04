#pragma once

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
	private:
		class RefFrame* m_refFrame;
		class Anchorable* m_anchorable;
		bool m_moving;	/// Currently actively moving
	};

}} // namespace ca::gui
