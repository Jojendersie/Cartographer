#include <ca/pa/log.hpp>
#include "ca/gui/rendering/sharp3dtheme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/guimanager.hpp"

using namespace ei;

namespace ca { namespace gui {

	/// Helper to multiply in RGB components and keep alpha
	Vec4 scaleColor(const Vec4& _color, float _factor)
	{
		return Vec4(_color.r * _factor, _color.g * _factor, _color.b * _factor, _color.a);
	}

	/// Helper to reduce the size of a rectangle without making min larger than max
	Rect2D saveBorderShrink(const Rect2D& _rect, int _borderWidth)
	{
		Vec2 border = (Vec2)max(IVec2{0}, min(IVec2{_borderWidth}, floor(0.5f * (_rect.max - _rect.min))));
		return Rect2D(_rect.min + border, _rect.max - border);
	}
	
	Sharp3DTheme::Sharp3DTheme(const Sharp3DProperties& _desc) :
		m_properties(_desc)
	{
		pa::logInfo("[ca::gui] Created Sharp 3D Theme.");
	}

	Sharp3DTheme::~Sharp3DTheme()
	{
		pa::logInfo("[ca::gui] Destroyed Sharp 3D Theme.");
	}

	void Sharp3DTheme::drawTextArea(const ei::Rect2D& _rect)
	{
		Rect2D rect = saveBorderShrink(_rect, m_properties.borderWidth);
		drawBorderRect(_rect, rect, m_properties.basicColor, scaleColor(m_properties.basicColor, 4.0f));
		GUIManager::renderBackend().drawRect(rect, m_properties.textBackColor);
	}

	void Sharp3DTheme::drawBackgroundArea(const ei::Rect2D& _rect, float _opacity, const ei::Vec3& _individualColor)
	{
		if(_opacity > 0.0f)
		{
			Vec4 color = _individualColor >= 0.0f ? Vec4(_individualColor, 1.0f) : m_properties.basicColor;
			color.a *= _opacity;
			if(m_properties.borderWidth)
			{
				Rect2D rect = saveBorderShrink(_rect, m_properties.borderWidth);
				drawBorderRect(_rect, rect, color, scaleColor(color, 4.0f));

				GUIManager::renderBackend().drawRect(rect, scaleColor(color, 0.5f));
			} else
				GUIManager::renderBackend().drawRect(_rect, color);
		}
	}

	void Sharp3DTheme::drawButton(const ei::Rect2D& _rect, bool _mouseOver, bool _mouseDown, bool _horizontal)
	{
		Vec4 color = _mouseOver ? m_properties.basicHoverColor : m_properties.basicColor;
		Rect2D rect = saveBorderShrink(_rect, m_properties.borderWidth);
		Vec2 gfrom, gto;
		if(_horizontal)
		{
			gfrom = Vec2(0.0f, _mouseDown ? 1.0f : 0.0f);
			gto   = Vec2(0.0f, _mouseDown ? 0.0f : 1.0f);
		} else {
			gfrom = Vec2(_mouseDown ? 0.0f : 1.0f, 0.0f);
			gto   = Vec2(_mouseDown ? 1.0f : 0.0f, 0.0f);
		}
		drawBorderRect(_rect, rect, color, scaleColor(color, 4.0f));

		GUIManager::renderBackend().drawRect(rect, gfrom, gto,
			scaleColor(color, 0.5f),
			scaleColor(color, 2.0f));
	}

	void Sharp3DTheme::drawCheckbox(const Rect2D& _rect, bool _checked, bool _mouseOver)
	{
		const Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		// Draw three different sized rectangles (border, background and a smaller one for the
		// checkmark).
		GUIManager::renderBackend().drawRect(_rect, Vec2(0.0f), Vec2(0.0f, 1.0f), scaleColor(color, 0.5f), scaleColor(color, 2.0f));
		Rect2D backRect = saveBorderShrink(_rect, 1);
		GUIManager::renderBackend().drawRect(backRect, m_properties.textBackColor);
		if(_checked)
		{
			Rect2D checkRect = saveBorderShrink(_rect, 3);
			GUIManager::renderBackend().drawRect(checkRect, Vec2(0.0f), Vec2(0.0f, 1.0f), scaleColor(color, 0.5f), scaleColor(color, 2.0f));
		}
	}

	void Sharp3DTheme::drawSliderBar(const Rect2D& _rect, float _relativeValue)
	{
		// Fill left side with a button like structure
		Rect2D leftFrame;
		leftFrame.min = _rect.min + 1.0f;
		leftFrame.max.x = roundf(_rect.min.x + 1.0f + (_rect.max.x - _rect.min.x - 2.0f) * _relativeValue);
		leftFrame.max.y = _rect.max.y - 1.0f;
		if(leftFrame.min.x != leftFrame.max.x)
		{
			Vec2 gfrom = Vec2(0.0f, 1.0f);
			Vec2 gto = Vec2(0.0f, 0.0f);

			GUIManager::renderBackend().drawRect(leftFrame, gfrom, gto,
				scaleColor(m_properties.basicColor, 0.5f),
				scaleColor(m_properties.basicColor, 2.0f));
		//	drawButton(leftFrame, false, true);
		}
	}

	void Sharp3DTheme::drawText(const Coord2& _position, const char * _text, float _relativeScale, bool _mouseOver, ei::Vec4 _color, float _alignX, float _alignY)
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

	ei::Rect2D Sharp3DTheme::getTextBB(const Coord2& _position, const char * _text, float _relativeScale, float _alignX, float _alignY)
	{
		return GUIManager::renderBackend().getTextBB(_position, _text,
			m_properties.textSize * _relativeScale,
			_alignX, _alignY,
			0.0f, true );
	}

	uint Sharp3DTheme::getTextCharacterPosition(const ei::Vec2 & _findPosition, const ei::Vec2 & _textPosition, const char * _text, float _relativeScale, float _alignX, float _alignY)
	{
		return GUIManager::renderBackend().getTextCharacterPosition(_findPosition, _textPosition, _text,
			m_properties.textSize * _relativeScale, _alignX, _alignY, 0.0f, true);
	}

	void Sharp3DTheme::drawImage(const Rect2D& _rect, uint64 _texHandle, float _opacity, bool _tiling)
	{
		if(_opacity > 0.0f)
			GUIManager::renderBackend().drawTextureRect(_rect, _texHandle, _opacity, _tiling);
	}

	void Sharp3DTheme::drawArrowButton(const Rect2D& _rect, SIDE::Val _pointTo, bool _mouseOver)
	{
		Triangle2D tri;
		switch(_pointTo) {
		default:
		case SIDE::LEFT:
			tri.v0 = Vec2 {_rect.max.x, _rect.min.y};
			tri.v1 = Vec2 {_rect.max.x, _rect.max.y};
			tri.v2 = Vec2 {_rect.min.x, (_rect.min.y + _rect.max.y) * 0.5f};
			break;
		case SIDE::BOTTOM:
			tri.v0 = Vec2 {_rect.max.x, _rect.max.y};
			tri.v1 = Vec2 {_rect.min.x, _rect.max.y};
			tri.v2 = Vec2 {(_rect.min.x + _rect.max.x) * 0.5f, _rect.min.y};
			break;
		case SIDE::RIGHT:
			tri.v0 = Vec2 {_rect.min.x, _rect.max.y};
			tri.v1 = Vec2 {_rect.min.x, _rect.min.y};
			tri.v2 = Vec2 {_rect.max.x, (_rect.min.y + _rect.max.y) * 0.5f};
			break;
		case SIDE::TOP:
			tri.v0 = Vec2 {_rect.min.x, _rect.min.y};
			tri.v1 = Vec2 {_rect.max.x, _rect.min.y};
			tri.v2 = Vec2 {(_rect.min.x + _rect.max.x) * 0.5f, _rect.max.y};
			break;
		}
		const Vec4& color = _mouseOver ? m_properties.hoverTextColor : m_properties.textColor;
		GUIManager::renderBackend().drawTriangle(tri, color, color, color);
	}

	void Sharp3DTheme::drawBorderRect(const Rect2D& _outer, const Rect2D& _inner, const ei::Vec4& _colorA, const ei::Vec4& _colorB)
	{
		// TODO: Find out what is faster 4 small rects or one large and much overdraw?
		Rect2D borderRect;
		borderRect = Rect2D(_outer.min, {_inner.min.x, _outer.max.y});
		GUIManager::renderBackend().drawRect(borderRect, Vec2(0.0), Vec2(0.0, 1.0), _colorA, _colorB);
		borderRect = Rect2D({_inner.max.x,_outer.min.y}, _outer.max);
		GUIManager::renderBackend().drawRect(borderRect, Vec2(0.0), Vec2(0.0, 1.0), _colorA, _colorB);
		borderRect = Rect2D({_inner.min.x, _inner.max.y}, {_inner.max.x, _outer.max.y});
		GUIManager::renderBackend().drawRect(borderRect, _colorB);
		borderRect = Rect2D({_inner.min.x, _outer.min.y}, {_inner.max.x, _inner.min.y});
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

	void Sharp3DTheme::drawLine(const ei::Vec2* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB)
	{
		GUIManager::renderBackend().drawLine(_positions, _numPositions, _colorA, _colorB);
	}

}} // namespace ca::gui
