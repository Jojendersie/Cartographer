#pragma once

#include "properties/coordinate.hpp"
#include "properties/refframe.hpp"
#include <ei/2dtypes.hpp>

namespace ca { namespace gui {

	/// Interface for different rendering implementations (look).
	/// \details Dependent on the generality a new theme does not ever need a new implementation.
	///		Changing colors should be possible over theme settings. A theme is more a general
	///		style like flat buttos versus bloby buttons...
	class ITheme
	{
	public:
		/// Draw a background for text
		virtual void drawTextArea(const class RefFrame& _rect) = 0;
		/// Draw a background for frames...
		virtual void drawBackgroundArea(const class RefFrame& _rect, float _opacity = 1.0f) = 0;
		/// Draw the background for buttons
		virtual void drawButton(const class RefFrame& _rect, bool _mouseOver) = 0;
		/// Draw the box with the check mark
		virtual void drawCheckbox(const class RefFrame& _rect, bool _checked, bool _mouseOver) = 0;
		/// Draw a string
		virtual void drawText(const Coord2& _position, const char* _text, bool _mouseOver, float _alignX = 0.0f, float _alignY = 0.0f) = 0;
		/// Get the width and the height of a string
		virtual ei::Rect2D getTextBB(const Coord2& _position, const char* _text, float _alignX = 0.0f, float _alignY = 0.0f) = 0;
		/// Draw an image
		virtual void drawImage(const class RefFrame& _rect, uint64 _texHandle, float _opacity = 1.0f) = 0;
		/// Draw an arrow for sliders, scrollbars or drop down menus
		virtual void drawArrowButton(const class RefFrame& _rect, SIDE::Val _pointTo, bool _mouseOver) = 0;
		
		// more are following...
	};

}} // namespace ca::gui