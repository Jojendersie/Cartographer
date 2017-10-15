#pragma once

#include "ca/gui/rendering/flattheme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/guimanager.hpp"

using namespace ei;

namespace ca { namespace gui {

	FlatTheme::FlatTheme(const FlatProperties& _desc) :
		m_properties(_desc)
	{
	}

	void FlatTheme::drawTextArea(const class RefFrame& _rect)
	{
		GUIManager::renderBackend().drawRect(_rect, m_properties.textBackColor);
	}

	void FlatTheme::drawBackgroundArea(const class RefFrame& _rect, float _opacity, const ei::Vec3& _individualColor)
	{
		if(_opacity > 0.0f)
		{
			Vec4 color = _individualColor >= 0.0f ? Vec4(_individualColor, 1.0f) : m_properties.backgroundColor;
			color.a *= _opacity;
			GUIManager::renderBackend().drawRect(_rect, color);
		}
	}

	void FlatTheme::drawButton(const RefFrame& _rect, bool _mouseOver, bool _mouseDown)
	{
		if(_mouseOver)
			GUIManager::renderBackend().drawRect(_rect, m_properties.hoverButtonColor);
		else 
			GUIManager::renderBackend().drawRect(_rect, m_properties.buttonColor);
	}

	void FlatTheme::drawCheckbox(const class RefFrame& _rect, bool _checked, bool _mouseOver)
	{
		const Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		// Draw three different sized rectangles (border, background and a smaller one for the
		// checkmark).
		GUIManager::renderBackend().drawRect(_rect, color);
		RefFrame backRect(_rect.left() + 1, _rect.right() - 1, _rect.bottom() + 1, _rect.top() - 1);
		GUIManager::renderBackend().drawRect(backRect, m_properties.textBackColor);
		if(_checked)
		{
			RefFrame checkRect(_rect.left() + 3, _rect.right() - 3, _rect.bottom() + 3, _rect.top() - 3);
			GUIManager::renderBackend().drawRect(checkRect, color);
		}
	}

	void FlatTheme::drawSliderBar(const class RefFrame& _rect, float _relativeValue)
	{
		RefFrame leftFrame;
		leftFrame.sides[SIDE::LEFT] = _rect.left() + 1.0f;
		leftFrame.sides[SIDE::RIGHT] = roundf(_rect.left() + 1.0f + (_rect.width() - 2.0f) * _relativeValue);
		leftFrame.sides[SIDE::TOP] = _rect.top() - 1.0f;
		leftFrame.sides[SIDE::BOTTOM] = _rect.bottom() + 1.0f;
		if(leftFrame.left() != leftFrame.right())
			GUIManager::renderBackend().drawRect(leftFrame, m_properties.hoverButtonColor);
	}

	void FlatTheme::drawText(const Coord2& _position, const char* _text, float _relativeScale, bool _mouseOver, ei::Vec4 _color, float _alignX, float _alignY)
	{
		for(int i = 0; i < 4; ++i) if(_color[i] < 0.0f)
			_color[i] = _mouseOver ? m_properties.hoverTextColor[i] : m_properties.textColor[i];
		GUIManager::renderBackend().drawText(_position, _text,
					m_properties.textSize * _relativeScale,
					_color,
					_alignX, _alignY,
					0.0f, true // TODO: dynamic rounding mode for moving components
			);
	}

	ei::Rect2D FlatTheme::getTextBB(const Coord2& _position, const char* _text, float _relativeScale, float _alignX, float _alignY)
	{
		return GUIManager::renderBackend().getTextBB(_position, _text,
			m_properties.textSize * _relativeScale,
			_alignX, _alignY,
			0.0f );
	}

	uint FlatTheme::getTextCharacterPosition(const ei::Vec2 & _findPosition, const ei::Vec2 & _textPosition, const char * _text, float _relativeScale, float _alignX, float _alignY)
	{
		return GUIManager::renderBackend().getTextCharacterPosition(_findPosition,
			_textPosition, _text, m_properties.textSize * _relativeScale,
			_alignX, _alignY, 0.0f);
	}

	void FlatTheme::drawImage(const RefFrame& _rect, uint64 _texHandle, float _opacity, bool _tiling)
	{
		if(_opacity > 0.0f)
			GUIManager::renderBackend().drawTextureRect(_rect, _texHandle, _opacity, _tiling);
	}

	void FlatTheme::drawArrowButton(const class RefFrame& _rect, SIDE::Val _pointTo, bool _mouseOver)
	{
		GUIManager::renderBackend().drawRect(_rect, _mouseOver ? m_properties.hoverButtonColor : m_properties.buttonColor);
		// Use the minimum possible size to create a triangle without stretch
		float size = min(_rect.width(), _rect.height()) - 2.0f;
		float sizeh = size/2.0f;
		Triangle2D triangle;
		Vec2 center((_rect.left() + _rect.right()) * 0.5f, (_rect.bottom() + _rect.top()) * 0.5f);
		switch(_pointTo)
		{
		case SIDE::LEFT:
			triangle.v0 = Vec2(center.x - sizeh, center.y);
			triangle.v1 = Vec2(center.x + sizeh, center.y - sizeh);
			triangle.v2 = Vec2(center.x + sizeh, center.y + sizeh);
			break;
		case SIDE::RIGHT:
			triangle.v0 = Vec2(center.x + sizeh, center.y);
			triangle.v1 = Vec2(center.x - sizeh, center.y + sizeh);
			triangle.v2 = Vec2(center.x - sizeh, center.y - sizeh);
			break;
		case SIDE::BOTTOM:
			triangle.v0 = Vec2(center.x,         center.y - sizeh);
			triangle.v1 = Vec2(center.x + sizeh, center.y + sizeh);
			triangle.v2 = Vec2(center.x - sizeh, center.y + sizeh);
			break;
		case SIDE::TOP:
			triangle.v0 = Vec2(center.x,         center.y + sizeh);
			triangle.v1 = Vec2(center.x - sizeh, center.y - sizeh);
			triangle.v2 = Vec2(center.x + sizeh, center.y - sizeh);
			break;
		}
		Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		GUIManager::renderBackend().drawTriangle(triangle, color, color, color);
	}

	void FlatTheme::drawNodeHandle(const Coord2& _position, float _radius, const ei::Vec3& _color)
	{
		Vec4 color(_color, 1.0f);
		// Draw a triangle fan to create a small circle
		Triangle2D triangle;
		triangle.v0 = _position + Coord2(_radius, 0.0f);
		triangle.v2 = _position + _radius * Coord2(cos(2*PI/12.0f), sin(2*PI/12.0f));
		for(int i = 0; i < 10; ++i)
		{
			triangle.v1 = triangle.v2;
			triangle.v2 = _position + _radius * Coord2(cos(2*PI * (i+2)/12.0f), sin(2*PI * (i+2)/12.0f));
			GUIManager::renderBackend().drawTriangle(triangle, color, color, color);
		}
	}

	void FlatTheme::drawLine(const ei::Vec3* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB)
	{
		GUIManager::renderBackend().drawLine(_positions, _numPositions, _colorA, _colorB);
	}


}} // namespace ca::gui