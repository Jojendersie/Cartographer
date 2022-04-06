#pragma once

#include "ca/gui/properties/coordinate.hpp"
#include "ca/gui/properties/refframe.hpp" // for SIDE
#include <ei/2dtypes.hpp>
#include <memory>

namespace ca { namespace gui {

	/// Interface for different rendering implementations (look).
	/// \details Dependent on the generality a new theme does not ever need a new implementation.
	///		Changing colors should be possible over theme settings. A theme is more a general
	///		style like flat buttons versus bloby buttons...
	class ITheme
	{
	public:
		/// Draw a background for text
		virtual void drawTextArea(const ei::Rect2D& _rect) = 0;
		/// Draw a background for frames, labels, ...
		/// \param [in] _individualColor Use an individual color. If negative (default) the
		///		theme colors are used.
		virtual void drawBackgroundArea(const ei::Rect2D& _rect, float _opacity = 1.0f, const ei::Vec3& _individualColor = ei::Vec3(-1.0f)) = 0;
		/// Draw the background for buttons
		/// \param [in] _horizontal Draw in normal (horizontal mode). If false rotates the
		///		shading (if there is any) by 90�.
		virtual void drawButton(const ei::Rect2D& _rect, bool _mouseOver, bool _mouseDown, bool _horizontal) = 0;
		/// Draw the box with the check mark
		virtual void drawCheckbox(const ei::Rect2D& _rect, bool _checked, bool _mouseOver) = 0;
		/// Draw the bar and the value mark of a slider
		/// \param [in] _relativeValue Position of the mark in [0,1] coordinates relative to the _rect.
		virtual void drawSliderBar(const ei::Rect2D& _rect, float _relativeValue) = 0;
		/// Draw a string
		/// \param [in] _relativeScale A number to scale the text relative to the global font size.
		/// \param [in] _color A custom color. If values are negative the theme colors are used instead.
		///		It is possible to define color channels partially. E.e. it is possible to set alpha
		///		while using the theme RGB color.
		virtual void drawText(const Coord2& _position, const char* _text, float _relativeScale, bool _mouseOver, ei::Vec4 _color = ei::Vec4(-1.0f), float _alignX = 0.0f, float _alignY = 0.0f) = 0;
		/// Get the width and the height of a string
		/// \param [in] _relativeScale A number to scale the text relative to the global font size.
		virtual ei::Rect2D getTextBB(const Coord2& _position, const char* _text, float _relativeScale, float _alignX = 0.0f, float _alignY = 0.0f) = 0;
		/// Get the character closest to the find position.
		virtual uint getTextCharacterPosition(const ei::Vec2& _findPosition, const ei::Vec2& _textPosition, const char* _text, float _relativeScale, float _alignX = 0.0f, float _alignY = 0.0f) = 0;
		/// Draw an image
		virtual void drawImage(const ei::Rect2D& _rect, uint64 _texHandle, float _opacity = 1.0f, bool _tiling = false) = 0;
		/// Draw an arrow for sliders, scrollbars or drop down menus
		virtual void drawArrowButton(const ei::Rect2D& _rect, SIDE::Val _pointTo, bool _mouseOver) = 0;
		/// Query the global basis font size. Headers or similar can differ from that (element dependent).
		/// \return Height in pixels.
		virtual float getTextSize() const = 0;
		/// Draw a small handle for node based editors
		virtual void drawNodeHandle(const Coord2& _position, float _radius, const ei::Vec3& _color) = 0;
		/// General purpose line drawing used for splines and borders
		virtual void drawLine(const ei::Vec2* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB) = 0;
		// more are following...

	protected:
	};

	typedef std::shared_ptr<ITheme> ThemePtr;

}} // namespace ca::gui