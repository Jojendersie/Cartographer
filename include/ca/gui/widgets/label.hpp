#pragma once

#include <string>
#include "widget.hpp"
#include "ca/gui/properties/textproperties.hpp"

namespace ca { namespace gui {

	/// Simple text element
	class Label : public Widget, public TextProperties
	{
	public:
		Label();

		/// Shortcut constructor for new Label + set_text + setBackgroundOpacity
		Label(const char* _text, float _fitSize, float _opacity);

		/// Implement the draw method
		void draw() const override;

		/// Set the description text (utf8 w/o BOM)
		/// \param [in] _text New text for the checkbox (utf8 w/o BOM). The text is copied internally.
		/// \param [in] _fitSize Change the label size to the size of the text plus boundary. Negtive
		///		values disable fitting (previous size is kept).
		void setText(const char* _text, float _fitSize = -1.0f);
		const char* getText() { return m_text.c_str(); }

		/// Set on which side including the center the text should be aligned.
		/// \details The default alignment is LEFT.
		void setAlignment(SIDE::Val _position) { m_textPosition = _position; }
		SIDE::Val getAlignment() const { return m_textPosition; }

		/// \param [in] _color Set an individual RGB color (0-1) including alpha value.
		///		If the label color should depend on the theme consider setBackgroundOpacity.
		void setBackgroundColor(const ei::Vec4& _color) { m_backgroundColor = _color; }
		const ei::Vec4& getBackgroundColor() const { return m_backgroundColor; }

		/// Overwrite background opacity only (keep color/use theme color).
		void setBackgroundOpacity(float _opacity) { m_backgroundColor.a = _opacity; }
		float getBackgroundOpacity() const { return m_backgroundColor.a; }

		/// Set a distance between reference frame border and the text (as long as the text is not
		/// down scaled).
		/// On down scaling the margin is scaled too.
		/// The margin is set automatically if setText with  a fitSize is used.
		void setMargin(Coord _margin) { m_margin = ei::Vec2(_margin); }
		void setMargin(Coord2 _margin) { m_margin = _margin; }
		ei::Vec2 getMargin() const { return m_margin; }
	private:
		std::string m_text;				///< Descriptive text
		ei::Vec2 m_textSize;
		ei::Vec2 m_margin;				///< Horizontal and vertical distance of the text to the sides
		SIDE::Val m_textPosition;
		ei::Vec4 m_backgroundColor;
	};

	typedef pa::RefPtr<Label> LabelPtr;

}} // namespace ca::gui
