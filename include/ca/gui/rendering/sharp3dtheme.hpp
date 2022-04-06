#pragma once

#include "theme.hpp"

namespace ca { namespace gui {

	struct Sharp3DProperties
	{
		int borderWidth;				///< Width of border around buttons... 0 or 1 look best.
		ei::Vec4 basicColor;			///< Used for the background of frames and buttons
		ei::Vec4 basicHoverColor;		///< Used for the background of active elements
		ei::Vec4 textColor;				///< Color for text
		ei::Vec4 textBackColor;			///< Color for text areas, checkboxes, ...
		ei::Vec4 hoverTextColor;		///< Color for texts when mouse is over
		float textSize;					///< Height of text in pixels
	};

	/// A look with sharp shapes and moderate gradients
	class Sharp3DTheme : public ITheme
	{
	public:
		Sharp3DTheme(const Sharp3DProperties& _desc);
		~Sharp3DTheme();

		virtual void drawTextArea(const ei::Rect2D& _rect) override;
		virtual void drawBackgroundArea(const ei::Rect2D& _rect, float _opacity = 1.0f, const ei::Vec3& _individualColor = ei::Vec3(-1.0f)) override;
		virtual void drawButton(const ei::Rect2D& _rect, bool _mouseOver, bool _mouseDown, bool _horizontal) override;
		virtual void drawCheckbox(const ei::Rect2D& _rect, bool _checked, bool _mouseOver) override;
		virtual void drawSliderBar(const ei::Rect2D& _rect, float _relativeValue) override;
		virtual void drawText(const Coord2& _position, const char* _text, float _relativeScale, bool _mouseOver, ei::Vec4 _color = ei::Vec4(-1.0f), float _alignX = 0.0f, float _alignY = 0.0f) override;
		virtual ei::Rect2D getTextBB(const Coord2& _position, const char* _text, float _relativeScale, float _alignX = 0.0f, float _alignY = 0.0f) override;
		virtual uint getTextCharacterPosition(const ei::Vec2& _findPosition, const ei::Vec2& _textPosition, const char* _text, float _relativeScale, float _alignX = 0.0f, float _alignY = 0.0f) override;
		virtual void drawImage(const ei::Rect2D& _rect, uint64 _texHandle, float _opacity = 1.0f, bool _tiling = false) override;
		virtual void drawArrowButton(const ei::Rect2D& _rect, SIDE::Val _pointTo, bool _mouseOver) override;
		virtual float getTextSize() const override { return m_properties.textSize; }
		virtual void drawNodeHandle(const Coord2& _position, float _radius, const ei::Vec3& _color) override;
		virtual void drawLine(const ei::Vec2* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB) override;
	protected:
		Sharp3DProperties m_properties;

		/// Draw a rectangle with borders. This rectangle might be completely filled.
		void drawBorderRect(const ei::Rect2D& _outer, const ei::Rect2D& _inner, const ei::Vec4& _colorA, const ei::Vec4& _colorB);
	};

}} // namespace ca::gui
