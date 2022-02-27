#pragma once

#include <functional>
#include "ca/gui/backend/mouse.hpp"

namespace ca { namespace gui {

	/// Allow interactive resizing of the component when the mouse cursor is on the border of the
	/// reference frame.
	class Resizeable: public IMouseProcessAble
	{
	public:
		/// Make a reference frame resizeable.
		/// \param [in] _thisWidget Widget where this component belongs to.
		Resizeable(class Widget * _thisWidget);

		/// Process mouse input for interactive resizing.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		virtual bool processInput(class Widget & _thisWidget, const struct MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput) override;

		bool isResizeingEnabled() const { return m_resizingEnabled; }
		void setResizeable(bool _enable) { m_resizingEnabled = _enable; }

		/// Register a callback function that is called when a resize is finished.
		/// Also see onExtentChanged for a continuous callback.
		typedef std::function<void(class Widget* _this)> OnResizeEnded;
		void setOnResizeEndedFunc(OnResizeEnded _callback) { m_onResizeEnded = _callback; }
	private:
		OnResizeEnded m_onResizeEnded;
		bool m_resizingEnabled;
		bool m_active;			/// Currently actively resizing
		bool m_resizing[4];		/// Flag for each resizing direction L,R,B,T
	};

}} // namespace ca::gui