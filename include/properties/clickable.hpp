#pragma once

#include <functional>
#include <vector>
#include "backend/mouse.hpp"
#include "refframe.hpp"

namespace ca { namespace gui {

	/// Interface for clickable objects defining input and callback handling.
	/// \details Per default a clickable uses the objects reference frame as input region.
	class Clickable
	{
	public:
		Clickable(RefFrame* _selfFrame);
		~Clickable();

		/// Process mouse input (uses the callbacks).
		virtual bool processInput(const struct MouseState& _mouseState);

		/// Type for click callbacks.
		/// \param [in] _where Internal position as info.
		/// \param [in] _button Index of the changed mouse button.
		/// \param [in] _state New state of the changed mouse button.
		typedef std::function<void(const Coord2& _where, int _button, MouseState::ButtonState _state)> OnButtonChange;

		/// Attach another callback to the element.
		/// \details It is not possible to detach function pointers.
		/// \param [in] _callback A new function which is called on any click event of this component.
		/// \param [in] _stateMask A bitmask which is 1 for each allowed state. I.e. the callback
		///		is called for all those states.
		void addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask);

		/// Set a handler to detect if the mouse is in the region of this object.
		/// \param [in] _region Valid click-regions are CircleRegion and MaskRegion.
		///
		///		The clickable takes the ownership of the given object if _delete is true. Make
		///		sure that delete will not fail.
		///
		///		If nothing is set/the nullptr is given, the own reference frame defines the region.
		/// \param [in] _delete The region object must be deleted.
		void setClickRegion(IRegion* _region, bool _delete = true);
	protected:
		IRegion* m_clickRegion;
		bool m_deleteRegion;
		std::vector<OnButtonChange> m_changeFuncs;
		std::vector<MouseState::ButtonState> m_statesMasks;
		bool m_buttonDownReceived[5];	// Detect for each button, if it was pressed on this element (for CLICK events)
		float m_lastClick[5];			// Time stamp for the last click to detect DBL_CLICK events
	};

	/// Use the reference frame fitted ellipse to detect click events.
	class EllipseRegion: public IRegion
	{
	public:
		/// \param [in] _selfFrame The reference frame for the clickable area
		EllipseRegion(RefFrame* _selfFrame);

		virtual bool isMouseOver(const Coord2& _mousePos) override;
	private:
		RefFrame* m_selfFrame;
	};

	/// Use a bitmap mask to detect click events.
	class MaskRegion: public IRegion
	{
	public:
		virtual bool isMouseOver(const Coord2& _mousePos) override;
	};

}} // namespace ca::gui
