#include "ca/gui/widgets/editbox.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/pa/string/utf8.hpp"

using namespace ei;

namespace ca { namespace gui {

	Edit::Edit() :
		m_textAlignment(SIDE::LEFT),
		m_textPosition(0.0f),
		m_backgroundTexture(0),
		m_focusTexture(0),
		m_cursorPosition(0)
	{
	}

	void Edit::draw() const
	{
		bool isEditing = GUIManager::hasKeyboardFocus(this);
		// Background
		if(isEditing)
		{
			if(m_focusTexture)
				GUIManager::theme().drawImage(rectangle(), m_focusTexture);
			else GUIManager::theme().drawTextArea(rectangle());
		} else {
			if(m_backgroundTexture)
				GUIManager::theme().drawImage(rectangle(), m_backgroundTexture);
			else GUIManager::theme().drawTextArea(rectangle());
		}

		// Text
		GUIManager::pushClipRegion(Rect2D{rectangle().min + m_margin, rectangle().max - m_margin});
		if(m_text.empty())
			GUIManager::theme().drawText(m_textPosition, m_descriptorText.c_str(), m_relativeTextSize, false, Vec4(Vec3(m_textColor), 0.5f));
		else
			GUIManager::theme().drawText(m_textPosition, m_text.c_str(), m_relativeTextSize, isEditing, m_textColor);

		// Cursor
		if(isEditing)
		{
			GUIManager::theme().drawText(m_cursorDrawPosition, "|", m_relativeTextSize, isEditing, m_textColor);
		}
		GUIManager::popClipRegion();
	}

	bool Edit::processInput(const MouseState & _mouseState)
	{
		if(getRegion()->isMouseOver(_mouseState.position)
			&& (_mouseState.buttons[0] == MouseState::DOWN
				|| _mouseState.buttons[0] == MouseState::PRESSED))
		{
			// Search for the closest position between two characters.
			m_cursorPosition = GUIManager::theme().getTextCharacterPosition(_mouseState.position, m_textPosition, m_text.c_str(), m_relativeTextSize);
			recomputeTextPlacement(!GUIManager::hasKeyboardFocus(this));
			GUIManager::setKeyboardFocus(this);
		}

		return Widget::processInput(_mouseState);
	}

	// TODO: move to helper header
	const char* CONTROL_STOP_CHARACTERS = " \t\n\r,.;:+-*/\\|^&~\"'()[]{}!?<>";

	static uint findPrevControlStop(const std::string & _str, uint _off)
	{
		if(_off <= 1) return 0;
		uint pos = (uint)_str.find_last_of( CONTROL_STOP_CHARACTERS, _off - 1 );
		return pos == std::string::npos ? 0 : pos;
	}

	static uint findNextControlStop(const std::string & _str, uint _off)
	{
		if(_off+1 >= _str.length()) return (uint)_str.length();
		uint pos = (uint)_str.find_first_of( CONTROL_STOP_CHARACTERS, _off + 1 );
		return pos == std::string::npos ? (uint)_str.length() : pos;
	}


	bool Edit::processInput(const KeyboardState & _keyboardState)
	{
		// Nothing to process?
		if(!_keyboardState.anyKeyChanged && _keyboardState.characterInput.empty()) return false;

		// Add new text at the cursor position
		if(!_keyboardState.characterInput.empty() && isAllowedValue(_keyboardState.characterInput.c_str()))
		{
			std::string newText = m_text.substr(0, m_cursorPosition)
				+ _keyboardState.characterInput
				+ m_text.substr(m_cursorPosition);
			m_cursorPosition += _keyboardState.characterInput.length();
			m_cursorPosition = repairText(newText);
			if(m_onTextChange) m_onTextChange(this, newText);
			m_text = newText;
			m_cursorPosition = std::min((int)newText.length(), m_cursorPosition);
			recomputeTextPlacement(false);
			return true;
		}

		// Try to change cursor position. Move entire words if CONTROL is pressed
		int oldCursor = m_cursorPosition;
		if(_keyboardState.isKeyDown(KeyboardState::Key::ARROW_LEFT)) {
			if(_keyboardState.isControlPressed())
				m_cursorPosition = findPrevControlStop(m_text, m_cursorPosition);
			else
				m_cursorPosition = pa::utf8::prev( m_text.c_str(), m_cursorPosition );
		}
		if(_keyboardState.isKeyDown(KeyboardState::Key::ARROW_RIGHT)) {
			if(_keyboardState.isControlPressed())
				m_cursorPosition = findNextControlStop(m_text, m_cursorPosition);
			else
				m_cursorPosition = pa::utf8::next( m_text.c_str(), m_cursorPosition );
		}
		if(_keyboardState.isKeyDown(KeyboardState::Key::END)
			|| (_keyboardState.isKeyDown(KeyboardState::Key::NUMPAD_1) ))//&& _keyboardState.isNumLock()))
			m_cursorPosition = (int)m_text.length();
		if(_keyboardState.isKeyDown(KeyboardState::Key::HOME)
			|| (_keyboardState.isKeyDown(KeyboardState::Key::NUMPAD_7) ))//&& _keyboardState.isNumLock()))
			m_cursorPosition = 0;

		// Delete some character or a word?
		if(_keyboardState.isKeyDown(KeyboardState::Key::BACKSPACE)
			&& m_cursorPosition > 0)
		{
			uint num = 0;
			if(_keyboardState.isControlPressed())
				num = m_cursorPosition - findPrevControlStop(m_text, m_cursorPosition);
			else num = m_cursorPosition - pa::utf8::prev(m_text.c_str(), m_cursorPosition);
			m_cursorPosition -= num;
			m_text.erase(m_cursorPosition, num);
			if(m_onTextChange) m_onTextChange(this, m_text);
		}
		if(_keyboardState.isKeyDown(KeyboardState::Key::DELETE)
			|| _keyboardState.isKeyDown(KeyboardState::Key::NUMPAD_DELETE))
		{
			uint num = 0;
			if(_keyboardState.isControlPressed())
				num = findNextControlStop(m_text, m_cursorPosition) - m_cursorPosition;
			else
				num = pa::utf8::next(m_text.c_str(), m_cursorPosition) - m_cursorPosition;
			m_text.erase(m_cursorPosition, num);
			if(m_onTextChange) m_onTextChange(this, m_text);
			// Does not change the cursor -> not handled automatically.
			recomputeTextPlacement(false);
			return true;
		}


		// Did something change?
		if(oldCursor != m_cursorPosition)
		{
			recomputeTextPlacement(false);
			return true;
		}

		return false;
	}

	void Edit::setText(const char * _text)
	{
		m_text = _text;
		m_cursorPosition = 0;
		repairText(m_text);
		if(m_onTextChange) m_onTextChange(this, m_text);
		recomputeTextPlacement(true);
	}

	void Edit::setDescriptorText(const char * _text)
	{
		m_descriptorText = _text;
		if(m_text.empty())
			recomputeTextPlacement(true);
	}

	void Edit::setBackgroundTexture(const char * _textureFile, const char * _focusTextureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		if(_focusTextureFile) m_focusTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		else m_focusTexture = m_backgroundTexture;
	}

	void Edit::recomputeTextPlacement(bool _fullRefresh)
	{
		Vec2 textSize = GUIManager::theme().getTextBB(Coord2(0.0f), m_text.c_str(), m_relativeTextSize).max;
		Vec2 textSizeAfterCursor = GUIManager::theme().getTextBB(Coord2(0.0f), m_text.c_str() + m_cursorPosition, m_relativeTextSize).max;
		Vec2 textSizeBeforeCursor = textSize - textSizeAfterCursor;
		Vec2 cursorSize = GUIManager::theme().getTextBB(Coord2(0.0f), "|", m_relativeTextSize).max;
		// Overwrite text size for placement of teh descriptor text.
		if(m_text.empty() && !m_descriptorText.empty())
			textSize = GUIManager::theme().getTextBB(Coord2(0.0f), m_descriptorText.c_str(), m_relativeTextSize).max;

		// Compute an initial text position (may change due to cursor).
		Coord2 textPos = m_textPosition;
		if(_fullRefresh)
		{
			Coord2 center = this->center();
			center -= textSize * 0.5f;
			switch(m_textAlignment)
			{
			case SIDE::LEFT: textPos = Coord2(left() + m_margin + 1.0f, center.y); break;
			case SIDE::RIGHT: textPos = Coord2(right() - textSize.x - m_margin - 1.0f, center.y); break;
			case SIDE::BOTTOM: textPos = Coord2(center.x, bottom() + m_margin); break;
			case SIDE::TOP: textPos = Coord2(center.x, top() - textSize.y - m_margin); break;
			case SIDE::CENTER: textPos = center; break;
			}
		}

		// Compute a local and then the global cursor position.
		Vec2 curPos = textSizeBeforeCursor;
		curPos += textPos;
		float offset = 0.0f;
		// Make sure the cursor is visible
		if(GUIManager::hasKeyboardFocus(this))
		{
			if(curPos.x < left() + m_margin)
				offset = left() + m_margin - curPos.x;
			if(curPos.x > right() - m_margin - cursorSize.x)
				offset = right() - m_margin - cursorSize.x - curPos.x;
		}
		//curPos.x -= max(1.0f, cursorSize.x * 0.5f);
		curPos.x -= cursorSize.x * 0.5f;
		m_cursorDrawPosition = curPos + Vec2(offset, 0.0f);
		m_textPosition = textPos + Vec2(offset, 0.0f);
	}

	bool Edit::isAllowedValue(const char* _newChar) const
	{
		switch (m_filter)
		{
			case TextFilterMode::NONE: return true;
			case TextFilterMode::INTEGER_POS:
				return pa::utf8::isDigit(_newChar);
			case TextFilterMode::INTEGER:
				return pa::utf8::isDigit(_newChar) || pa::utf8::isSign(_newChar);
			case TextFilterMode::DECIMAL:
				return pa::utf8::isDigit(_newChar) || pa::utf8::isSign(_newChar) || (*_newChar == '.');
			case TextFilterMode::FLOAT:
				return pa::utf8::isDigit(_newChar) || pa::utf8::isSign(_newChar) || (*_newChar == '.') || (*_newChar == 'e');
		}
		return false;
	}

	int Edit::repairText(std::string& _newText) const
	{
		int newCursor = 0;
		switch (m_filter)
		{
			case TextFilterMode::NONE: break;
			case TextFilterMode::INTEGER_POS: {
				std::string filtered;
				for(size_t i = 0; i < _newText.length() && ((int)filtered.length() < m_filterPrecision); ++i)
				{
					if(pa::utf8::isDigit(&_newText[i]))
					{
						// Lookahead 1 to check if the current character is a leading zero
						if(filtered.empty() && (_newText[i] == '0') && (_newText[i+1] != '\0'))
							continue;
						filtered += _newText[i];
						if((int)i < m_cursorPosition)
							++newCursor;
					}
				}
				_newText = filtered;
			} break;
			case TextFilterMode::INTEGER: {
				std::string filtered;
				int precision = 0;
				for(size_t i = 0; i < _newText.length() && (precision < m_filterPrecision); ++i)
				{
					bool add = false;
					if(pa::utf8::isDigit(&_newText[i]))
					{
						// Lookahead 1 to check if the current character is a leading zero
						if(filtered.empty() && (_newText[i] == '0') && (_newText[i+1] != '\0'))
							continue;
						add = true;
						++precision;
					}
					else if(pa::utf8::isSign(&_newText[i]) && (filtered.length() == 0))
					{
						add = true;
					}

					if(add)
					{
						filtered += _newText[i];
						if((int)i < m_cursorPosition)
							++newCursor;
					}
				}
				_newText = filtered;
			} break;
			case TextFilterMode::DECIMAL: {
				std::string filtered;
				bool hasPoint = m_cursorPosition > 0 ? _newText[m_cursorPosition-1] == '.' : false; // If there are multiple points keep the one that just got inserted
				int precision = 0;
				for(size_t i = 0; i < _newText.length(); ++i)
				{
					bool add = false;
					if(pa::utf8::isSign(&_newText[i]) && (filtered.length() == 0))
					{
						add = true;
					}
					else if(pa::utf8::isDigit(&_newText[i]) && (precision < m_filterPrecision))
					{
						// Lookahead 1 to check if the current character is a leading zero
						if(filtered.empty() && (_newText[i] == '0') && (_newText[i+1] != '.') && (_newText[i+1] != '\0'))
							continue;
						add = true;
						++precision;
					}
					else if(_newText[i] == '.' && (!hasPoint || (int)i == m_cursorPosition-1))
					{
						add = hasPoint = true;
					}
					// Fallthrough: Anything that wasn't added above will vanish
					if(add)
					{
						filtered += _newText[i];
						if((int)i < m_cursorPosition)
							++newCursor;
					}
				}
				_newText = filtered;
			} break;
			case TextFilterMode::FLOAT: {
				std::string filtered;
				int ePos = -1;
				bool hasPoint = m_cursorPosition > 0 ? _newText[m_cursorPosition-1] == '.' : false; // If there are multiple points keep the one that just got inserted
				int precision = 0;
				for(size_t i = 0; i < _newText.length(); ++i)
				{
					bool add = false;
					//if(pa::utf8::isSign(&_newText[i]) && ((filtered.length() == 0) || (ePos+1 == (int)filtered.length())))
					if(pa::utf8::isSign(&_newText[i]) && (ePos+1 == (int)filtered.length())) // Equiv. to above due to initialization of ePos
					{
						add = true;
					}
					else if(_newText[i] == 'e' && (ePos == -1))
					{
						add = true;
						ePos = (int)i;
					}
					else if(pa::utf8::isDigit(&_newText[i]) && (ePos != -1 || precision < m_filterPrecision))
					{
						// Lookahead 1 to check if the current character is a leading zero
						if(filtered.empty() && (_newText[i] == '0') && (_newText[i+1] != '.') && (_newText[i+1] != '\0'))
							continue;
						add = true;
						++precision;
					}
					else if(_newText[i] == '.' && (!hasPoint || (int)i == m_cursorPosition-1))
					{
						add = hasPoint = true;
					}
					// Fallthrough: Anything that wasn't added above will vanish
					if(add)
					{
						filtered += _newText[i];
						if((int)i < m_cursorPosition)
							++newCursor;
					}
				}
				_newText = filtered;
			} break;
		}
		return newCursor;
	}

	void Edit::onExtentChanged()
	{
		Widget::onExtentChanged();
		recomputeTextPlacement(true);
	}

	void Edit::onKeyboardFocus(bool _gotFocus)
	{
		Widget::onKeyboardFocus(_gotFocus);
		if(!_gotFocus)
			recomputeTextPlacement(true);
	}

}} // namespace ca::gui
