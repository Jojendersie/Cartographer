#pragma once

namespace cag {

	/// A moveable component is moved when the mouse is pushed while beeing over the component
	/// and then moved.
	class Moveable
	{
	public:
		/// Make a reference frame moveable.
		/// \param [in,opt] _anchorable Since positioning of anchorables is done automaically
		///		it must be reset/disabled by the moveable. Non-anchorable widgets can pass a
		///		nullptr others should always provide access to their anchorable component.
		Moveable(RefFrame* _selfFrame, Anchorable* _anchorable);

		/// Process mouse input for drag & drop like movement.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		void processInput(const struct MouseState& _mouseState);
	};

} // namespace cag