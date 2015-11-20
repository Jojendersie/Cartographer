#pragma once

#include "properties/coordinate.hpp"

namespace ca { namespace gui {

	/// Descriptor for the current mouse state.
	struct MouseState
	{
		/// Cursor position in internal coordinates.
		/// \details Coordinates outside [0,1] are possible if the mouse leaves the window while
		///		being pressed or if the window was resized and the mouse is not yet repositioned.
		Coord2 position;

		/// Change of the position since last frame.
		Coord2 deltaPos;

		/// Flags of a mouse button state.
		/// \details It is possible that multiple flags are set in a single frame. This happens
		///		if DOWN and UP event occured in the same frame. The PRESSED flag is never set
		///		when DOWN is set.
		///
		///		Metastates are generated by some components like clickables. The manager will not
		///		produce them.
		enum ButtonState
		{
			RELEASED = 0,		///< Button is not pressed
			DOWN = 1,			///< Button was clicked in the current frame
			PRESSED = 4,		///< Button is still pressed (was clicked in the past)
			UP = 2,				///< Button was released in the current frame
			CLICKED = 8,		///< Metastate (UP and DOWN where done on the same component)
			DBL_CLICKED = 16,	///< Metastate Two CLICK events occured in a short period
		};

		/// State for up to 5 buttons
		ButtonState buttons[5];
	};

}} // namespace ca::gui