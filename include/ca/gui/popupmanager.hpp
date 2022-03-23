#pragma once

#include "widgets/widget.hpp"
#include <stack>

namespace ca { namespace gui {

	enum class PopupClosingCondition
	{
		OTHER_INTERACTION,	///< Close if the mouse/keyboard is pressed with focus on something else
		MOUSE_DISTANCE,		///< Close if the mouse distance is larger than some threshold
	};

	/// A stack-based manager for elements that are shown on top of everything else.
	/// 
	class PopupManager
	{
	public:
		PopupManager();

		/// Set the time in seconds before a popup is shown
		void setPopupTime(float _showAfter);
		float getPopupTime() const { return m_popupTime; }

		/// Add a widget to the popup stack. It is rendered above everything else.
		/// \details This methods will siliently ignore calls for elements that are already on
		///		the stack.
		/// \param [in] _closeOn Condition under which the element will be removed from the stack.
		///		Popups will be closed in a cascade as long as the top most one can be closed.
		/// \param [in] _receiveInputs Apply inputs to the popup widget. Most popups are only of
		///		visual nature and do not need inputs. In this case input handling is optimized away.
		void showPopup(
			WidgetPtr & _popup,
			const Widget * _originator,
			PopupClosingCondition _closeOn,
			bool _receiveInputs = false,
			float _closeThreshold = 20.0f);

		/// Special variant that tries to show the popup (top left) at a specified position.
		/// If the position is too close to a boundary, the popup will be moved.
		void showPopupAt(
			WidgetPtr & _popup,
			const Widget * _originator,
			Coord2 _position,
			PopupClosingCondition _closeOn = PopupClosingCondition::MOUSE_DISTANCE,
			bool _receiveInputs = false,
			float _closeThreshold = 20.0f);

		/// Manually close a popup.
		void closePopup(Widget * _popup);

		/// Input handling (handled by GUIManager)
		bool processInput(const struct MouseState& _mouseState);
		//bool processInput(const struct KeyboardState& _keyboardState);

		void draw() const;

		/// Remove all the open popups.
		void clear();

	private:
		struct PopupInfo
		{
			WidgetPtr widget;
			PopupClosingCondition cond;
			float condThreshold;		///< Semantics depends on 'cond'. Can be unused.
			bool receiveInput;			///< If false, only render the widget and do not send any inputs to it
		};

		std::vector<PopupInfo> m_popupStack;
		float m_popupTime;				///< Seconds when to show a popup
	};

}} // namespace ca::gui