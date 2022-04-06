#include "ca/gui/widgets/checkbox.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	CheckBox::CheckBox() :
		Clickable(this),
		m_textWidth(0.0f),
		m_checked(false),
		m_backgroundTexture(0)
	{
		Widget::setKeyboardFocusable(true);
		m_checkmarkTexture[0] = m_checkmarkTexture[1] = 0;
		addOnButtonChangeFunc(
			[](Widget* _thisBtn, const Coord2&, int, MouseState::ButtonState){
				static_cast<CheckBox*>(_thisBtn)->setChecked(!static_cast<CheckBox*>(_thisBtn)->m_checked);
			},
			MouseState::ButtonState(MouseState::CLICKED | MouseState::DBL_CLICKED));
	}

	void CheckBox::draw() const
	{
		// Background
		if(m_backgroundTexture)
			GUIManager::theme().drawImage(rectangle(), m_backgroundTexture);
		else
			GUIManager::theme().drawButton(rectangle(), false, false, true);

		// Determine vertical center for text and box
		float padding = GUIManager::theme().getTextSize() * 0.25f;
		float downScale = ei::min(1.0f, ei::min(height() / (GUIManager::theme().getTextSize() + 2.0f),
			width() / (m_textWidth + 6.0f + GUIManager::theme().getTextSize())));
		float size = (GUIManager::theme().getTextSize() - 2.0f) * downScale;
		float vcenter = (bottom() + top() - size) * 0.5f;

		// "Icon"
		bool mouseOver = GUIManager::hasMouseFocus(this);
		// Create a rectangle with the same size as the global font size
		ei::Rect2D checkRect;
		checkRect.min.x = floorf(padding + left());
		checkRect.max.x = floorf(checkRect.min.x + size);
		checkRect.min.y = floorf(vcenter);
		checkRect.max.y = floorf(checkRect.min.y + size);
		GUIManager::theme().drawCheckbox(checkRect, m_checked, mouseOver);

		// Text
		GUIManager::theme().drawText(Coord2(checkRect.max.x + padding, vcenter - 1.0f), m_text.c_str(), downScale, mouseOver);
	}

	void CheckBox::setText(const char* _text)
	{
		m_text = _text;
		m_textWidth = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f).max.x;
	}

	void CheckBox::setBackgroundTexture(const char * _textureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
	}

	void CheckBox::setChecked(bool _value)
	{
		if(m_checked != _value)
		{
			m_checked = _value;
			if(m_onChange) m_onChange(this, _value);
		}
	}

	void CheckBox::setOnChange(OnChange _callback)
	{
		m_onChange = _callback;
	}

}} // namespace ca::gui
