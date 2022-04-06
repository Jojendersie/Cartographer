#include "ca/gui/widgets/editbox.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

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

	//static bool utf8IsStartByte(char _c)
	//{
	//	//return ((_c & 0x80) == 0) || ((_c & 0xc0) == 0xc0);
	//	return (_c & 0xc0) != 0x80;
	//}
	static bool utf8IsIntermediateBye(char _c)
	{
		return (_c & 0xc0) == 0x80;
	}

	static uint utf8Next(const char* _text, uint _pos)
	{
		if(_text[_pos] == 0) return _pos;
		++_pos;
		while(utf8IsIntermediateBye(_text[_pos])) ++_pos;
		return _pos;
	}

	static uint utf8Prev(const char* _text, uint _pos)
	{
		if(_pos == 0) return 0;
		--_pos;
		while(utf8IsIntermediateBye(_text[_pos]) && _pos > 0) --_pos;
		return _pos;
	}


	bool Edit::processInput(const KeyboardState & _keyboardState)
	{
		// Nothing to process?
		if(!_keyboardState.anyKeyChanged && _keyboardState.characterInput.empty()) return false;

		// Add new text at the cursor position
		if(!_keyboardState.characterInput.empty())
		{
			int oldLen = (int)m_text.length();
			m_text = m_text.substr(0, m_cursorPosition)
				+ _keyboardState.characterInput
				+ m_text.substr(m_cursorPosition);
			if(m_onTextChange) m_onTextChange(this, m_text);
			m_cursorPosition += (int)m_text.length() - oldLen;
			recomputeTextPlacement(false);
			return true;
		}

		// Try to change cursor position. Move entire words if CONTROL is pressed
		int oldCursor = m_cursorPosition;
		if(_keyboardState.isKeyDown(KeyboardState::Key::ARROW_LEFT)) {
			if(_keyboardState.isControlPressed())
				m_cursorPosition = findPrevControlStop(m_text, m_cursorPosition);
			else
				m_cursorPosition = utf8Prev( m_text.c_str(), m_cursorPosition );
		}
		if(_keyboardState.isKeyDown(KeyboardState::Key::ARROW_RIGHT)) {
			if(_keyboardState.isControlPressed())
				m_cursorPosition = findNextControlStop(m_text, m_cursorPosition);
			else
				m_cursorPosition = utf8Next( m_text.c_str(), m_cursorPosition );
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
			else num = m_cursorPosition - utf8Prev(m_text.c_str(), m_cursorPosition);
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
				num = utf8Next(m_text.c_str(), m_cursorPosition) - m_cursorPosition;
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

	void Edit::onExtentChanged(const CHANGE_FLAGS::Val _changes)
	{
		Widget::onExtentChanged(_changes);
		recomputeTextPlacement(true);
	}

	void Edit::onKeyboardFocus(bool _gotFocus)
	{
		Widget::onKeyboardFocus(_gotFocus);
		if(!_gotFocus)
			recomputeTextPlacement(true);
	}

}} // namespace ca::gui
