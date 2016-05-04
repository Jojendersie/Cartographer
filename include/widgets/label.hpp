#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Simple text element
	class Label : public Widget
	{
	public:
		Label();

		/// Implement the draw method
		void draw() override;

		/// Set the description text (utf8 w/o BOM)
		/// \param [in] _text New text for the checkbox (utf8 w/o BOM). The text is copied internally.
		void setText(const char* _text);
		const char* getText() { return m_text.c_str(); }

		/// Set on which side including the center the text should be aligned.
		/// \details The default alignment is LEFT.
		void setAlignment(SIDE::Val _position) { m_textPosition = _position; }
		SIDE::Val getAlignment() const { return m_textPosition; }
	private:
		std::string m_text;				///< Descriptive text
		ei::Vec2 m_textSize;
		SIDE::Val m_textPosition;
	};

	typedef std::shared_ptr<Label> LabelPtr;

}} // namespace ca::gui
