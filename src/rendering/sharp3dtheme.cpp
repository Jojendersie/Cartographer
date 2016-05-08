#pragma once

#include "rendering/sharp3dtheme.hpp"
#include "backend/renderbackend.hpp"
#include "guimanager.hpp"

using namespace ei;

namespace ca { namespace gui {

	/// Helper to multiply in RGB components and keep alpha
	Vec4 scaleColor(const Vec4& _color, float _factor)
	{
		return Vec4(_color.r * _factor, _color.g * _factor, _color.b * _factor, _color.a);
	}
	
	Sharp3DTheme::Sharp3DTheme(const Sharp3DProperties& _desc) :
		m_properties(_desc)
	{
	}

	void Sharp3DTheme::drawTextArea(const RefFrame& _rect)
	{
		RefFrame rect(_rect.left() + m_properties.borderWidth, _rect.right() - m_properties.borderWidth, _rect.bottom() + m_properties.borderWidth, _rect.top() - m_properties.borderWidth);
		drawBorderRect(_rect, rect, m_properties.basicColor, scaleColor(m_properties.basicColor, 4.0f));
		GUIManager::renderBackend().drawRect(rect, m_properties.textBackColor);
	}

	void Sharp3DTheme::drawBackgroundArea(const RefFrame& _rect, float _opacity)
	{
		if(_opacity > 0.0f)
		{
			Vec4 color = m_properties.basicColor;
			color.a *= _opacity;
			if(m_properties.borderWidth)
			{
				RefFrame rect(_rect.left() + m_properties.borderWidth, _rect.right() - m_properties.borderWidth, _rect.bottom() + m_properties.borderWidth, _rect.top() - m_properties.borderWidth);
				drawBorderRect(_rect, rect, color, scaleColor(color, 4.0f));

				GUIManager::renderBackend().drawRect(rect, scaleColor(color, 0.5f));
			} else
				GUIManager::renderBackend().drawRect(_rect, color);
		}
	}

	void Sharp3DTheme::drawButton(const RefFrame& _rect, bool _mouseOver, bool _mouseDown)
	{
		Vec4 color = _mouseOver ? m_properties.basicHoverColor : m_properties.basicColor;
		RefFrame rect(_rect.left() + m_properties.borderWidth, _rect.right() - m_properties.borderWidth, _rect.bottom() + m_properties.borderWidth, _rect.top() - m_properties.borderWidth);
		Vec2 gfrom = Vec2(0.0f, _mouseDown ? 1.0f : 0.0f);
		Vec2 gto = Vec2(0.0f, _mouseDown ? 0.0f : 1.0f);
		drawBorderRect(_rect, rect, color, scaleColor(color, 4.0f));

		GUIManager::renderBackend().drawRect(rect, gfrom, gto,
			scaleColor(color, 0.5f),
			scaleColor(color, 2.0f));
	}

	void Sharp3DTheme::drawCheckbox(const RefFrame& _rect, bool _checked, bool _mouseOver)
	{
		const Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		// Draw three different sized rectangles (border, background and a smaller one for the
		// checkmark).
		GUIManager::renderBackend().drawRect(_rect, Vec2(0.0f), Vec2(0.0f, 1.0f), scaleColor(color, 0.5f), scaleColor(color, 2.0f));
		RefFrame backRect(_rect.left() + 1, _rect.right() - 1, _rect.bottom() + 1, _rect.top() - 1);
		GUIManager::renderBackend().drawRect(backRect, m_properties.textBackColor);
		if(_checked)
		{
			RefFrame checkRect(_rect.left() + 3, _rect.right() - 3, _rect.bottom() + 3, _rect.top() - 3);
			GUIManager::renderBackend().drawRect(checkRect, Vec2(0.0f), Vec2(0.0f, 1.0f), scaleColor(color, 0.5f), scaleColor(color, 2.0f));
		}
	}

	void Sharp3DTheme::drawSliderBar(const class RefFrame& _rect, float _relativeValue)
	{
		// Fill left side with a button like structure
		RefFrame leftFrame;
		leftFrame.sides[SIDE::LEFT] = _rect.left() + 1.0f;
		leftFrame.sides[SIDE::RIGHT] = roundf(_rect.left() + 1.0f + (_rect.width() - 2.0f) * _relativeValue);
		leftFrame.sides[SIDE::TOP] = _rect.top() - 1.0f;
		leftFrame.sides[SIDE::BOTTOM] = _rect.bottom() + 1.0f;
		if(leftFrame.left() != leftFrame.right())
		{
			Vec2 gfrom = Vec2(0.0f, 1.0f);
			Vec2 gto = Vec2(0.0f, 0.0f);

			GUIManager::renderBackend().drawRect(leftFrame, gfrom, gto,
				scaleColor(m_properties.basicColor, 0.5f),
				scaleColor(m_properties.basicColor, 2.0f));
		//	drawButton(leftFrame, false, true);
		}
	}

	void Sharp3DTheme::drawText(const Coord2& _position, const char * _text, float _relativeScale, bool _mouseOver, float _alignX, float _alignY)
	{
		GUIManager::renderBackend().drawText(_position, _text,
					m_properties.textSize * _relativeScale,
					_mouseOver ? m_properties.hoverTextColor : m_properties.textColor,
					_alignX, _alignY,
					0.0f, true // TODO: dynamic rounding mode for moving components
			);
	}

	ei::Rect2D Sharp3DTheme::getTextBB(const Coord2& _position, const char * _text, float _relativeScale, float _alignX, float _alignY)
	{
		return GUIManager::renderBackend().getTextBB(_position, _text,
			m_properties.textSize * _relativeScale,
			_alignX, _alignY,
			0.0f );
	}

	void Sharp3DTheme::drawImage(const RefFrame& _rect, uint64 _texHandle, float _opacity)
	{
		if(_opacity > 0.0f)
			GUIManager::renderBackend().drawTextureRect(_rect, _texHandle, _opacity);
	}

	void Sharp3DTheme::drawArrowButton(const RefFrame& _rect, SIDE::Val _pointTo, bool _mouseOver)
	{
	}

	void Sharp3DTheme::drawBorderRect(const RefFrame& _outer, const RefFrame& _inner, const ei::Vec4& _colorA, const ei::Vec4& _colorB)
	{
		// TODO: Find out what is faster 4 small rects or one large and much overdraw?
		RefFrame borderRect;
		borderRect = RefFrame(_outer.left(), _inner.left(), _outer.bottom(), _outer.top());
		GUIManager::renderBackend().drawRect(borderRect, Vec2(0.0), Vec2(0.0, 1.0), _colorA, _colorB);
		borderRect = RefFrame(_inner.right(), _outer.right(), _outer.bottom(), _outer.top());
		GUIManager::renderBackend().drawRect(borderRect, Vec2(0.0), Vec2(0.0, 1.0), _colorA, _colorB);
		borderRect = RefFrame(_inner.left(), _inner.right(), _inner.top(), _outer.top());
		GUIManager::renderBackend().drawRect(borderRect, _colorB);
		borderRect = RefFrame(_inner.left(), _inner.right(), _outer.bottom(), _inner.bottom());
		GUIManager::renderBackend().drawRect(borderRect, _colorA);
		/*GUIManager::renderBackend().drawRect(_rect,// <- large one
		Vec2(0.0f), Vec2(0.0f, 1.0f), scaleColor(m_properties.basicColor, 1.0f),
		scaleColor(m_properties.basicColor, 4.0f));*/
	}

	void Sharp3DTheme::drawNodeHandle(const Coord2& _position, float _radius, const ei::Vec3& _color)
	{
		Vec4 centerColor(_color * 2.0f, 1.0f);
		Vec4 color(_color / 2.0f, 1.0f);
		// Draw a triangle fan to create a small circle with a single other-colored vertex in the center
		Triangle2D triangle;
		triangle.v0 = _position + _radius / 3.0f * Coord2(cos(2*PI*3/12.0f), sin(2*PI*3/12.0f));
		triangle.v2 = _position + _radius * Coord2(cos(2*PI/12.0f), sin(2*PI/12.0f));
		for(int i = 0; i < 12; ++i)
		{
			triangle.v1 = triangle.v2;
			triangle.v2 = _position + _radius * Coord2(cos(2*PI * (i+2)/12.0f), sin(2*PI * (i+2)/12.0f));
			GUIManager::renderBackend().drawTriangle(triangle, centerColor, color, color);
		}
		// TODO: draw a border
	}

	void Sharp3DTheme::drawLine(const Coord2& _position, const ei::Vec3& _color)
	{
	}

}} // namespace ca::gui
