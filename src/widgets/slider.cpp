#define _CRT_SECURE_NO_WARNINGS

#include "widgets/slider.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace ca { namespace gui {

	Slider::Slider() :
		Widget(true, false, false, false, true, true),
		m_value(0.5),
		m_min(0.0),
		m_range(1.0),
		m_stepSize(0.01),
		m_isMoving(false)
	{
	}

	void Slider::draw()
	{
		// Background
		GUIManager::theme().drawBackgroundArea(m_refFrame);

		// Fill left side
		RefFrame leftFrame;
		leftFrame.sides[SIDE::LEFT] = m_refFrame.left() + 1.0f;
		leftFrame.sides[SIDE::RIGHT] = roundf(leftFrame.left() + 1.0f + (m_refFrame.width() - 3.0f) * (float)m_value);
		leftFrame.sides[SIDE::TOP] = m_refFrame.top() - 1.0f;
		leftFrame.sides[SIDE::BOTTOM] = m_refFrame.bottom() + 1.0f;
		GUIManager::theme().drawButton(leftFrame, false, true);

		// Draw value
		char valueString[32];
		sprintf(valueString, "%g", m_value);
		Coord2 textPos;
		textPos.x = m_refFrame.left() + m_refFrame.width() * 0.5f + 1.0f;
		textPos.y = m_refFrame.bottom() + m_refFrame.height() * 0.5f;
		GUIManager::theme().drawText(textPos, valueString, 1.0f, false, 0.5f, 0.5f);
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
			relativeVal *= m_range;
			// Round to step size
			relativeVal = round(relativeVal / m_stepSize) * m_stepSize;
			m_value = m_min + relativeVal;

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
	}

}} // namespace ca::gui
