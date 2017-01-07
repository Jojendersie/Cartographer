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
		Clickable(RefFrame* _selfFrame);
		~Clickable();

		/// Process mouse input (uses the callbacks).
		bool processInput(const struct MouseState& _mouseState);

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

		const IRegion* getClickRegion() const { return m_clickRegion; }

		bool isAnyButtonDown() const { for(int i = 0; i < 8; ++i) if(m_buttonDownReceived[i]) return true; return false; }
	protected:
		IRegion* m_clickRegion;
		bool m_deleteRegion;
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
		EllipseRegion(RefFrame* _selfFrame);

		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	private:
		RefFrame* m_selfFrame;
	};

	/// Use a bitmap mask to detect click events.
	class MaskRegion : public IRegion
	{
	public:
		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	};

}} // namespace ca::gui
