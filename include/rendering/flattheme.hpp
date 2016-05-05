#pragma once

#include <ei/vector.hpp>
#include "theme.hpp"

namespace ca { namespace gui {

	struct FlatProperties
	{
		ei::Vec4 backgroundColor;		///< Color for frames...
		ei::Vec4 buttonColor;			///< Color for buttons
		ei::Vec4 foregroundColor;		///< Color for arrows...
		ei::Vec4 textColor;				///< Color for text
		ei::Vec4 textBackColor;			///< Color for text areas, checkboxes, ...
		ei::Vec4 hoverButtonColor;		///< Color for buttons... when mouse is over
		ei::Vec4 hoverTextColor;		///< Color for texts when mouse is over
		float textSize;					///< Height of text in pixels
	};

	/// Super simple look without borders or gradients.
	class FlatTheme : public ITheme
	{
	public:
		FlatTheme(const FlatProperties& _desc);

		virtual void drawTextArea(const class RefFrame& _rect) override;
		virtual void drawBackgroundArea(const class RefFrame& _rect, float _opacity = 1.0f) override;
		virtual void drawButton(const class RefFrame& _rect, bool _mouseOver, bool _mouseDown) override;
		virtual void drawCheckbox(const class RefFrame& _rect, bool _checked, bool _mouseOver) override;
		virtual void drawSliderBar(const class RefFrame& _rect, float _relativeValue) override;
		virtual void drawText(const Coord2& _position, const char* _text, float _relativeScale, bool _mouseOver, float _alignX = 0.0f, float _alignY = 0.0f) override;
		virtual ei::Rect2D getTextBB(const Coord2& _position, const char* _text, float _relativeScale, float _alignX = 0.0f, float _alignY = 0.0f) override;
		virtual void drawImage(const class RefFrame& _rect, uint64 _texHandle, float _opacity = 1.0f) override;
		virtual void drawArrowButton(const class RefFrame& _rect, SIDE::Val _pointTo, bool _mouseOver) override;
		virtual float getTextSize() const override { return m_properties.textSize; }
		virtual void drawNodeHandle(const Coord2& _position, float _radius, const ei::Vec3& _color) override;
		virtual void drawLine(const Coord2& _position, const ei::Vec3& _color) override;
	protected:
		FlatProperties m_properties;
	};

}} // namespace ca::gui