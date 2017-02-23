#pragma once

namespace ca { namespace gui {

	/// Allow interactive resizing of the component when the mouse cursor is on the border of the
	/// reference frame.
	class Resizeable
	{
	public:
		/// Make a reference frame resizeable.
		/// \param [in,opt] _anchorable Since positioning of anchorables is done automaically
		///		it must be reset/disabled by the resizeable. Non-anchorable widgets can pass a
		///		nullptr others should always provide access to their anchorable component.
		Resizeable(class RefFrame* _selfFrame, class Anchorable* _anchorable);

		/// Process mouse input for interactive resizing.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		bool processInput(const struct MouseState& _mouseState);

		/// Should the anchor component change update this component.
		/// \param [in] _anchorable The new anchor component or new if there is none afterwards.
		void registerAnchorCompoentent(class Anchorable* _anchorable) { m_anchorable = _anchorable; }
	private:
		class RefFrame* m_refFrame;
		class Anchorable* m_anchorable;
		bool m_active;			/// Currently actively resizing
		bool m_resizing[4];		/// Flag for each resizing direction L,R,B,T
	};

}} // namespace ca::gui