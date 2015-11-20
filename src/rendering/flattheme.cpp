#pragma once

#include "rendering/flattheme.hpp"
#include "backend/renderbackend.hpp"
#include "cagui.hpp"

using namespace ei;

namespace cag {

	FlatTheme::FlatTheme(const FlatProperties& _desc) :
		m_properties(_desc)
	{
	}

	void FlatTheme::drawTextArea(const class RefFrame& _rect)
	{
		GUIManager::getRenderBackend()->drawRect(_rect, m_properties.textBackColor);
	}

	void FlatTheme::drawBackgroundArea(const class RefFrame& _rect, float _opacity)
	{
		GUIManager::getRenderBackend()->drawRect(_rect, m_properties.backgroundColor);
	}

	void FlatTheme::drawButton(const RefFrame& _rect, bool _mouseOver)
	{
		if(_mouseOver)
			GUIManager::getRenderBackend()->drawRect(_rect, m_properties.hoverBackgroundColor);
		else 
			GUIManager::getRenderBackend()->drawRect(_rect, m_properties.backgroundColor);
	}

	void FlatTheme::drawCheckbox(const class RefFrame& _rect, bool _checked, bool _mouseOver)
	{
		const Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		// Draw three different sized rectangles (border, background and a smaller one for the
		// checkmark).
		GUIManager::getRenderBackend()->drawRect(_rect, color);
		RefFrame backRect(_rect.left() - 1, _rect.right() - 1, _rect.bottom() - 1, _rect.top() - 1);
		GUIManager::getRenderBackend()->drawRect(backRect, m_properties.textBackColor);
		if(_checked)
		{
			RefFrame checkRect(_rect.left() - 3, _rect.right() - 3, _rect.bottom() - 3, _rect.top() - 3);
			GUIManager::getRenderBackend()->drawRect(checkRect, color);
		}
	}

	void FlatTheme::drawText(const Coord2& _position, const char* _text, bool _mouseOver, float _alignX, float _alignY)
	{
		GUIManager::getRenderBackend()->drawText(_position, _text,
					m_properties.textSize,
					_mouseOver ? m_properties.hoverTextColor : m_properties.textColor,
					_alignX, _alignY,
					0.0f, true // TODO: true or false?
			);
	}

	void FlatTheme::drawImage(const RefFrame& _rect, uint64 _texHandle, float _opacity)
	{
		GUIManager::getRenderBackend()->drawTextureRect(_rect, _texHandle, _opacity);
	}

	void FlatTheme::drawArrowButton(const class RefFrame& _rect, SIDE::Val _pointTo, bool _mouseOver)
	{
		GUIManager::getRenderBackend()->drawRect(_rect, _mouseOver ? m_properties.hoverBackgroundColor : m_properties.backgroundColor);
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
		GUIManager::getRenderBackend()->drawTriangle(triangle, _mouseOver ? m_properties.hoverTextColor : m_properties.textColor);
	}

} // namespace cag