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

		/// Change checked/unchecked state of the element, may trigger the onChange callback
		void setChecked(bool _value);

		/// Query current status
		bool isChecked() const { return m_checked; }

		/// Type for change callbacks.
		/// \param [in] _thisBox A pointer to the widget for which the event is triggered.
		/// \param [in] _newValue The new value. The old one is consequently !_newValue.
		typedef std::function<void(class Widget* _thisBox, const bool _newValue)> OnChange;
		
		/// Replace the onChange function. The function will be called whenever the value of
		/// the check box changed.
		void setOnChange(OnChange _callback);
	private:
		std::string m_text;				///< Descriptive text beside the checkmark
		float m_textWidth;
		bool m_checked;					///< Toggle state
		uint64 m_backgroundTexture;		///< A custom texture for the background
		uint64 m_checkmarkTexture[2];	///< Custom textures for the checkmark
		OnChange m_onChange;
	};

	typedef pa::RefPtr<CheckBox> CheckBoxPtr;

}} // namespace ca::gui
