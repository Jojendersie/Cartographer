#pragma once

namespace cag {

	/// Interface for different rendering implementations (look).
	/// \details Dependent on the generality a new theme does not ever need a new implementation.
	///		Changing colors should be possible over theme settings. A theme is more a general
	///		style like flat buttos versus bloby buttons...
	class ITheme
	{
	public:
		/// Draw a background for text
		virtual void drawTextArea();
		/// Draw the background for buttons
		virtual void drawButton(const char* _text);
		/// Draw the box with the check mark
		virtual void drawCheckbox(bool _checked);
		/// Draw a string
		virtual void drawText();
		/// Draw an image
		virtual void drawImage();
		/// Draw an arrow for sliders, scrollbars or drop down menus
		virtual void drawArrowButton();
		
		// more are following...
	};

} // namespace cag