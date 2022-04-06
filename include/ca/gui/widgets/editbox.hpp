#pragma once

#include <string>
#include "widget.hpp"
#include "ca/gui/properties/textproperties.hpp"

namespace ca { namespace gui {

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

		// TODO: call on movement and resize?
		void recomputeTextPlacement(bool _fullRefresh);
		virtual void onExtentChanged(const CHANGE_FLAGS::Val _changes) override;
		virtual void onKeyboardFocus(bool _gotFocus) override;
	};

	typedef pa::RefPtr<Edit> EditPtr;

}} // namespace ca::gui
