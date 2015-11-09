#pragma once

namespace cag {

	/// Allow interactive resizing of the component when the mouse cursor is on the border of the
	/// reference frame.
	class Resizeable
	{
	public:
		/// Make a reference frame resizeable.
		/// \param [in,opt] _anchorable Since positioning of anchorables is done automaically
		///		it must be reset/disabled by the resizeable. Non-anchorable widgets can pass a
		///		nullptr others should always provide access to their anchorable component.
		Resizeable(RefFrame* _selfFrame, Anchorable* _anchorable);

		/// Process mouse input for interactive resizing.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		void processInput(const struct MouseState& _mouseState);
	};

} // namespace cag