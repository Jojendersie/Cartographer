#pragma once

#include "ca/gui/properties/coordinate.hpp"

namespace ca { namespace gui {

	/// Descriptor for the current mouse state.
	struct MouseState
	{
		/// Cursor position in internal (pixel) coordinates.
		/// \details Coordinates outside [0,width/heigh] are possible if the mouse leaves the window
		///		while being pressed or if the window was resized and the mouse is not yet repositioned.
		Coord2 position;

		/// Change of the position since last frame.
		Coord2 deltaPos() const { return position - lastPosition; }

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
			// The following meta events are handled by the gui itself.
			// There is no need for a backend to set these values.
			CLICKED = 8,		///< Metastate (UP and DOWN where done on the same component)
			DBL_CLICKED = 16,	///< Metastate Two CLICK events occured in a short period
		};

		enum class Button
		{
			LEFT = 0,
			RIGHT = 1,
			MIDDLE = 2,
			MB3 = 3,
			MB4 = 4,
			MB5 = 5,
			MB6 = 6,
			MB7 = 7
		};

		/// State for up to 8 buttons
		ButtonState buttons[8];
		/// Is there any DOWN or UP event for any of the 8 buttons?
		bool anyButtonUp, anyButtonDown, anyButtonPressed;

		ei::Vec2 deltaScroll;

		/// Reset/change states
		void clear();

		// *** Optional Helper functions ***
		void updateFromGLFWInput(int _glfwButton, int _glfwAction);

		// Default initilization
		MouseState()
		{
			for(int i = 0; i < 8; ++i) buttons[i] = RELEASED;
			anyButtonUp = anyButtonDown = anyButtonPressed = false;
			deltaScroll = ei::Vec2{0.0f};
			lastPosition = position = Coord2{0.0f};
		}

		bool btnReleased(int _btn) const { return buttons[_btn] == 0; }
		bool btnDown(int _btn) const { return (buttons[_btn] & MouseState::DOWN) != 0; }
		bool btnPressed(int _btn) const { return (buttons[_btn] & MouseState::PRESSED) != 0; }
		bool btnUp(int _btn) const { return (buttons[_btn] & MouseState::UP) != 0; }
		bool btnClicked(int _btn) const { return (buttons[_btn] & MouseState::CLICKED) != 0; }
		bool btnDblClicked(int _btn) const { return (buttons[_btn] & MouseState::DBL_CLICKED) != 0; }

		bool operator != (const MouseState& _other) const
		{
			return position != _other.position
				|| deltaScroll != _other.deltaScroll
				|| buttons[0] != _other.buttons[0]
				|| buttons[1] != _other.buttons[1]
				|| buttons[2] != _other.buttons[2]
				|| buttons[3] != _other.buttons[3]
				|| buttons[4] != _other.buttons[4]
				|| buttons[5] != _other.buttons[5]
				|| buttons[6] != _other.buttons[6]
				|| buttons[7] != _other.buttons[7];
		}
	private:
		Coord2 lastPosition;
	};

	/// Desired shape of the cursor.
	/// \detais The GUI will set thes in the manager and expects that the application will render
	///		according shapes.
	enum class CursorType
	{
		ARROW,					///< Standard arrow/pointer
		IBEAM,					///< Text input cursor
		CROSSHAIR,				///< The cross for precise picking
		HAND,					///< The Hand
		RESIZE_H,				///< Arrow for horizontal resizing
		RESIZE_V,				///< Arrow for vertical resizing
		RESIZE_DUP,				///< Arrow for resizing with Lower-Left to Upper-Right orientation
		RESIZE_DDOWN,			///< Arrow for resizing with Upper-Right to Lower-Left orientation
		MOVE,					///< The cross for movements
		WAIT,					///< Hour-glass or similar
	};

	class IMouseProcessAble
	{
	public:
		/// \param [in] _thisWidget Reference to the widged which is calling this behavior. This avoids
		///		that subcomponents need references to their parent.
		/// \param [in] _cursorOnWidget Precomputed value of getRegion()->isMouseOver();
		/// \param [out] _ensureNextInput Make sure the next input to the widget is passed to this
		///		component again.
		virtual bool processInput(
			class Widget& _thisWidget,
			const struct MouseState& _mouseState,
			bool _cursorOnWidget,
			bool& _ensureNextInput
		) = 0;
	};

}} // namespace ca::gui