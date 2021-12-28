#pragma once

#include <string>
#include "widget.hpp"

namespace ca { namespace gui {

	/// Element to make a binary choice (true/false)
	class CheckBox : public Widget, protected Clickable
	{
	public:
		CheckBox();

		/// Implement the draw method
		void draw() const override;

		/// Set the description text (utf8 w/o BOM)
		/// \param [in] _text New text for the checkbox (utf8 w/o BOM). The text is copied internally.
		void setText(const char* _text);
		const char* getText() { return m_text.c_str(); }

		/// Set a custom look instead of the theme's background rendering.
		/// \param [in] _textureFile Name of an image file.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		void setBackgroundTexture(const char* _textureFile, bool _smooth = true);
	private:
		std::string m_text;				///< Descriptive text beside the checkmark
		float m_textWidth;
		bool m_checked;					///< Toggle state
		uint64 m_backgroundTexture;		///< A custom texture for the background
		uint64 m_checkmarkTexture[2];	///< Custom textures for the checkmark
	};

	typedef pa::RefPtr<CheckBox> CheckBoxPtr;

}} // namespace ca::gui
