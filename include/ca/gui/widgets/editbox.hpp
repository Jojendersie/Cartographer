#pragma once

#include <string>
#include "widget.hpp"
#include "ca/gui/properties/textproperties.hpp"

namespace ca { namespace gui {

	enum class TextFilterMode
	{
		NONE,
		INTEGER_POS,    // Only positive integers
		INTEGER,		// Integer with sign
		DECIMAL,		// A number with a . but not including exponentials.
		FLOAT		    // Floating point numbers including exponential e+-[num]
	};

	// A box in which text can be edited.
	class Edit : public Widget, public TextProperties
	{
	public:
		Edit();

		void draw() const override;

		/// Clicking on the edit field sets the cursor and the keyboard focus.
		virtual bool processInput(const struct MouseState& _mouseState) override;

		/// Write or removes characters from the text
		virtual bool processInput(const struct KeyboardState& _keyboardState) override;

		/// Set the text (utf8 w/o BOM)
		/// \param [in] _text New text for the checkbox (utf8 w/o BOM). The text is copied internally.
		void setText(const char* _text);
		const char* getText() const { return m_text.c_str(); }

		void setDescriptorText(const char* _text);
		const char* getDescriptorText() const { return m_descriptorText.c_str(); }

		/// Set on which side including the center the text should be aligned.
		/// \details The default alignment is LEFT.
		void setAlignment(SIDE::Val _position) { m_textAlignment = _position; recomputeTextPlacement(true); }
		SIDE::Val getAlignment() const { return m_textAlignment; }

		/// Set a custom look instead of the theme's background rendering.
		/// \param [in] _textureFile Name of an image file.
		/// \param [in] _focusTextureFile A second texture if the edit has the focus.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		void setBackgroundTexture(const char* _textureFile, const char* _focusTextureFile, bool _smooth = true);

		/// Set a distance between reference frame border and the text.
		void setMargin(Coord _margin) { m_margin = _margin; recomputeTextPlacement(true); }
		float getMargin() const { return m_margin; }

		/// Callback for insertion and deletion of of the text. Allows write access to the new text
		/// which can be used for custom filtering.
		typedef std::function<void(class Widget* _this, std::string &)> OnTextChange;
		void setOnTextChangeFunc(OnTextChange _func) { m_onTextChange = std::move(_func); }

		/// Set which and how characters are accepted upon input.
		/// \param [in] _mode One of the TextFilterMode values.
		/// \param [in] _precision Number of allowed significant digits, or -1 for unlimited.
		///		If filter mode is NONE this sets a limit for the general character count.
		///     In FLOAT this is the number of digits excluding potential scientific notation.
		///		This also means that 0.001 counts as precision 4 although 1.618e-3 is actually more precise.
		void setInputFilter(TextFilterMode _mode, const int _precision = -1) { m_filter = _mode; m_filterPrecision = _precision; }
	private:
		std::string m_text;				///< One line of text
		std::string m_descriptorText;	///< A text which is shown if m_text is empty
		SIDE::Val m_textAlignment;
		ei::Vec2 m_textPosition;
		ei::Vec2 m_cursorDrawPosition;
		uint64 m_backgroundTexture;
		uint64 m_focusTexture;
		Coord m_margin;
		int m_cursorPosition;			///< Index where new characters are inserted (0 is in front of all others)
		OnTextChange m_onTextChange;
		TextFilterMode m_filter;
		int m_filterPrecision;

		// TODO: call on movement and resize?
		void recomputeTextPlacement(bool _fullRefresh);
		// Checks if a character can be inserted within the current filter mode
		bool isAllowedValue(const char* _newChar) const;
		// Removes double signs and the like. Returns new cursor position
		int repairText(std::string& _newText) const;
		virtual void onExtentChanged() override;
		virtual void onKeyboardFocus(bool _gotFocus) override;
	};

	typedef pa::RefPtr<Edit> EditPtr;

}} // namespace ca::gui
