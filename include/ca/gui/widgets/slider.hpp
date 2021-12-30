#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	class Slider : public Widget
	{
	public:
		Slider();

		/// Implement the draw method
		void draw() const override;

		virtual bool processInput(const struct MouseState& _mouseState);

		void setRange(int _min, int _max, int _stepSize = 1);
		void setRange(float _min, float _max, float _stepSize = 0.1f);

		/// Set if and where the current value should be shown.
		/// \param [in] _where Use a value outside [0,1] to disable the label and
		///		a value in [0,1] to place it somewhere from left to right. 
		void setValueTextPosition(float _where);

		void setValue(int _value);
		void setValue(float _value);

		/// Type for change callbacks.
		/// \param [in] _thisSlider A pointer to the widget for which the event is triggered.
		/// \param [in] _newValue The new value in double such that both integer and float are
		///		captured without loss.
		typedef std::function<void(class Widget* _thisSlider, const double _newValue)> OnChange;
		
		/// Replace the onChange function. The function will be called whenever the value of
		/// the slider changed.
		void setOnChange(OnChange _callback);
	protected:
		void setValue(double _value);	///< Internal set method of the true type, the others are only front ends

		double m_value;		///< Take an internal double to be able to represent entire int32 range.
		double m_min, m_range;
		double m_stepSize;
		float m_labelPos;
		bool m_isMoving;	///< Mouse is currently attached
		OnChange m_onChange;
	};

	typedef pa::RefPtr<Slider> SliderPtr;

}} // namespace ca::gui
