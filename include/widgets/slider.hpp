#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	class Slider : public Widget
	{
	public:
		Slider();

		/// Implement the draw method
		void draw() override;

		virtual bool processInput(const struct MouseState& _mouseState);

		void setRange(int _min, int _max, int _stepSize = 1);
		void setRange(float _min, float _max, float _stepSize = 0.1f);

		// TODO onChanged, observer callback
	protected:
		double m_value;		///< Take an internal double to be able to represent entire int32 range.
		double m_min, m_range;
		double m_stepSize;
		bool m_isMoving;	///< Mouse is currently attached
	};

	typedef std::shared_ptr<Slider> SliderPtr;

}} // namespace ca::gui
