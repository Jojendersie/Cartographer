#define _CRT_SECURE_NO_WARNINGS

#include "ca/gui/widgets/slider.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Slider::Slider() :
		m_value(0.5),
		m_min(0.0),
		m_range(1.0),
		m_stepSize(0.01),
		m_labelPos(0.5f),
		m_isMoving(false)
	{
		Widget::setKeyboardFocusable(true);
		enable();
	}

	void Slider::draw() const
	{
		// Background
		//GUIManager::theme().drawBackgroundArea(m_refFrame);
		GUIManager::theme().drawTextArea(m_refFrame);
		GUIManager::theme().drawSliderBar(m_refFrame, float((m_value - m_min) / m_range));

		// Draw value string
		if(m_labelPos >= 0.0f && m_labelPos <= 1.0f)
		{
			char valueString[32];
			sprintf(valueString, "%g", m_value);
			Coord2 textPos;
			textPos.x = m_refFrame.left() + (m_refFrame.width() - 6.0f) * m_labelPos + 4.0f;
			textPos.y = m_refFrame.bottom() + m_refFrame.height() * 0.5f;
			GUIManager::theme().drawText(textPos, valueString, 1.0f, false, ei::Vec4(-1.0f), m_labelPos, 0.5f);
		}
	}

	bool Slider::processInput(const MouseState& _mouseState)
	{
		// While the left mouse button is down move the mark
		if(_mouseState.buttons[0] == MouseState::DOWN)
		{
			if(m_refFrame.isMouseOver(_mouseState.position))
			{
				m_isMoving = true;
				GUIManager::setMouseFocus(this, true);
				return true;
			}
		} else if(_mouseState.buttons[0] == MouseState::RELEASED || _mouseState.buttons[0] == MouseState::UP)
		{
			m_isMoving = false;
		} else if(m_isMoving) {
			double relativeVal = (GUIManager::getMouseState().position.x - m_refFrame.left()) / (m_refFrame.width()-2.0f);
			relativeVal = ei::clamp(relativeVal, 0.0, 1.0);
			double newVal = m_min + relativeVal * m_range;
			// Round to step size
			newVal = round(newVal / m_stepSize) * m_stepSize;
			setValue(newVal);

			// Keep sticky focus until mouse is released (above)
			GUIManager::setMouseFocus(this, true);
		}

		return Widget::processInput(_mouseState);
	}

	void Slider::setRange(int _min, int _max, int _stepSize)
	{
		m_min = _min;
		m_range = _max - _min;
		m_stepSize = _stepSize;
	}

	void Slider::setRange(float _min, float _max, float _stepSize)
	{
		m_min = _min;
		m_range = _max - _min;
		m_stepSize = _stepSize;
		m_value = ei::clamp(m_value, (double)_min, (double)_max);
	}

	void Slider::setValueTextPosition(float _where)
	{
		m_labelPos = _where;
	}

	void Slider::setValue(int _value)
	{
		setValue(static_cast<double>(_value));
	}

	void Slider::setValue(float _value)
	{
		setValue(static_cast<double>(_value));
	}

	void Slider::setOnChange(OnChange _callback)
	{
		m_onChange = _callback;
	}

	void Slider::setValue(double _value)
	{
		_value = ei::clamp(_value, m_min, m_min+m_range);
		if(_value != m_value) {
			m_value = _value;
			if(m_onChange) m_onChange(this, _value);
		}
	}

}} // namespace ca::gui
