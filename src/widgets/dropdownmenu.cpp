#include "ca/gui/widgets/dropdownmenu.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp" // Image loading
#include <ca/pa/log.hpp>

namespace ca { namespace gui {

	struct ScrollBarItem
	{
		std::string text;
		uint64 iconTexture;
		Coord2 iconSize;
		WidgetPtr infoPopup;			//< Each item may have an individual description
	};

	class DropDownListIntenral : public Widget
	{
	public:
		std::vector<ScrollBarItem> m_items;		//< The selectable items
		Coord m_itemHeight;						//< The height of each row.
		Coord2 m_maxIconSize;					//< Size of the icon texture with the largest width
		int m_hoverItem;						//< The item with the mouse cursor or negative

		DropDownListIntenral() :
			m_itemHeight { 20.0f },
			m_maxIconSize { 0.0f, 1.0f },
			m_hoverItem { -1 }
		{
			setVisible(false);
		}

		void setIcon(int _idx, const char* _textureFile, bool _smooth)
		{
			m_items[_idx].iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
			const ei::IVec2 iconSize = GUIManager::renderBackend().getTextureSize(m_items[_idx].iconTexture);
			m_items[_idx].iconSize = coord::pixel(iconSize.x, iconSize.y);
			if(iconSize.x > m_maxIconSize.x)
				m_maxIconSize = m_items[_idx].iconSize;
		}

		bool processInput(const struct MouseState & _mouseState) override
		{
			// Get distance to to the top of the widget to compute the index.
			m_hoverItem = ei::floor((m_parent->bottom() - _mouseState.position.y) / m_itemHeight);
			// Floor made sure that the index is negative if the mouse is too far up.
			// the opposide end is still open.
			if(m_hoverItem >= (int)m_items.size())
				m_hoverItem = -1;

			if(_mouseState.btnClicked(0))
			{
				static_cast<DropDownMenu*>(m_parent)->setSelected(m_hoverItem);
				GUIManager::popups().closePopup(this);
			}
			return true;
		}

		// Get maximum horizontal and vertical size dependent on the largest element
		// and the current line height.
		Coord2 getMaxIconSize() const
		{
			// Icons are scaled automatically in vertical direction.
			const Coord margin = static_cast<DropDownMenu*>(m_parent)->getItemTextMargin();
			const Coord maxIconHeight = m_itemHeight - margin * 2;
			// Compute required horizontal space for the largest item
			const Coord maxIconWidth = m_maxIconSize.y <= maxIconHeight
				? m_maxIconSize.x
				: m_maxIconSize.x * maxIconHeight / m_maxIconSize.y;
			return Coord2{maxIconWidth, maxIconHeight};
		}

		void drawItemStandalone(int _idx, float _baseLine) const
		{
			const Coord2 maxIconSize = getMaxIconSize();
			const Coord margin = static_cast<DropDownMenu*>(m_parent)->getItemTextMargin();

			if (m_items[_idx].iconTexture != 0)
			{
				const float downScale = m_items[_idx].iconSize.y <= maxIconSize.y ? 1.0f : maxIconSize.y / m_items[_idx].iconSize.y;
				const float vMargin = floor(m_items[_idx].iconSize.y < maxIconSize.y ? margin + 0.5f * (maxIconSize.y - m_items[_idx].iconSize.y) : margin);
				ei::Rect2D iconFrame;
				iconFrame.min.x = left() + margin;
				iconFrame.max.x = iconFrame.min.x + m_items[_idx].iconSize.x * downScale;
				iconFrame.min.y = _baseLine + vMargin;
				iconFrame.max.y = iconFrame.min.y + ei::min(m_items[_idx].iconSize.y, maxIconSize.y);
				GUIManager::theme().drawImage(iconFrame, m_items[_idx].iconTexture);
			}

			GUIManager::theme().drawText(
					Coord2(left() + margin * 2 + maxIconSize.x,
						_baseLine + margin),
					m_items[_idx].text.c_str(),
					1.0f,
					false
				);
		}

		void draw() const override
		{
			// The Widget spans over its parent for proper popup input handling.
			// For drawing we need the frame below only.
			ei::Rect2D refFrame = rectangle();
			refFrame.max.y = m_parent->bottom();

			GUIManager::theme().drawBackgroundArea(refFrame);

			if (m_hoverItem >= 0)
			{
				ei::Rect2D hFrame {
					{refFrame.min.x, refFrame.max.y - (m_hoverItem+1) * m_itemHeight + 1.0f},
					{refFrame.max.x, refFrame.max.y - (m_hoverItem  ) * m_itemHeight - 1.0f}
				};
				GUIManager::theme().drawTextArea(hFrame);
			}

			const Coord2 maxIconSize = getMaxIconSize();
			const Coord margin = static_cast<DropDownMenu*>(m_parent)->getItemTextMargin();
			const Coord iconLeft = refFrame.min.x + margin;
			const Coord textLeft = refFrame.min.x + margin * 2 + maxIconSize.x;
			Coord yPos = refFrame.max.y + margin;
			for(int i = 0; i < m_items.size(); ++i)
			{
				yPos -= m_itemHeight;

				if (m_items[i].iconTexture != 0)
				{
					const float downScale = m_items[i].iconSize.y <= maxIconSize.y ? 1.0f : maxIconSize.y / m_items[i].iconSize.y;
					const float vMargin = floor(m_items[i].iconSize.y < maxIconSize.y ? 0.5f * (maxIconSize.y - m_items[i].iconSize.y) : 0.0f);
					ei::Rect2D iconFrame;
					iconFrame.min.x = iconLeft;
					iconFrame.max.x = iconLeft + m_items[i].iconSize.x * downScale;
					iconFrame.min.y = yPos + vMargin;
					iconFrame.max.y = iconFrame.min.y + ei::min(m_items[i].iconSize.y, maxIconSize.y);
					GUIManager::theme().drawImage(iconFrame, m_items[i].iconTexture);
				}

				GUIManager::theme().drawText(
					Coord2(textLeft, yPos),
					m_items[i].text.c_str(),
					1.0f,
					m_hoverItem == i
				);
			}
		}
	};


	// ************************************************************************ //
	DropDownMenu::DropDownMenu() :
		m_maxNumShown { 100 },
		m_selected { -1 },
		m_itemTextMargin { 2.0f }
	{
		m_list = new DropDownListIntenral;
		m_list->setParent(this);
	}

	
	int DropDownMenu::addItem(const char* _text, const char* _textureFile, bool _smooth)
	{
		const int idx = (int)m_list->m_items.size();
		m_list->m_items.push_back({_text, 0, Coord2{0.0f}, nullptr});
		if(_textureFile)
			m_list->setIcon(idx, _textureFile, _smooth);
		if (m_selected == -1)
			m_selected = 0;

		updateSubWidget();
		return idx;
	}


	void DropDownMenu::removeItem(int _idx)
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during removeItem().");
			return;
		}

		m_list->m_items.erase(m_list->m_items.begin() + _idx);
		if(_idx == m_selected) // Deleted the selected item?
		{
			m_selected = m_list->m_items.empty() ? -1
						 : ei::max(0, _idx - 1);
			m_onSelect(this, m_selected);
		} else if (_idx < m_selected) // Index of the selected item changed, because it comes after the deleted one.
			--m_selected;

		updateSubWidget();
	}


	int DropDownMenu::getNumItems() const
	{
		return (int)m_list->m_items.size();
	}


	const char* DropDownMenu::getItemText(int _idx) const
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during getItemText().");
			return nullptr;
		}

		return m_list->m_items[_idx].text.c_str();
	}

	void DropDownMenu::setItemText(int _idx, const char* _text)
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during setItemText().");
			return;
		}
		m_list->m_items[_idx].text = _text;
	}


	const uint64 DropDownMenu::getItemIcon(int _idx) const
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during getItemIcon().");
			return 0;
		}

		return m_list->m_items[_idx].iconTexture;
	}

	void DropDownMenu::setItemIcon(int _idx, const char* _textureFile, bool _smooth)
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during setItemIcon().");
			return;
		}

		//if(m_list->m_items[_idx].iconTexture != 0)
			// Free texture?

		m_list->setIcon(_idx, _textureFile, _smooth);
	}


	Coord DropDownMenu::getItemTextMargin() const
	{
		return m_itemTextMargin;
	}


	void DropDownMenu::setItemTextMargin(Coord _margin)
	{
		m_itemTextMargin = _margin;
	}


	const Widget* DropDownMenu::getItemInfoPopup(int _idx) const
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during getItemInfoPopup().");
			return nullptr;
		}

		return m_list->m_items[_idx].infoPopup.get();
	}
	
	Widget* DropDownMenu::getItemInfoPopup(int _idx)
	{
		return const_cast<Widget*>(const_cast<const DropDownMenu*>(this)->getItemInfoPopup(_idx));
	}

	void DropDownMenu::setItemInfoPopup(int _idx, WidgetPtr _popup)
	{
		if (_idx < 0 || _idx >= getNumItems())
		{
			pa::logWarning("[ca::gui::DropDownMenu] Item index out of bounds during setItemInfoPopup().");
			return;
		}

		m_list->m_items[_idx].infoPopup = std::move(_popup);
	}


	void DropDownMenu::setSelected(int _idx)
	{
		if(_idx != m_selected && _idx >= 0)
		{
			m_selected = _idx;
			if(m_onSelect)
				m_onSelect(this, m_selected);
		}
	}


	void DropDownMenu::draw() const
	{
		GUIManager::theme().drawBackgroundArea(rectangle());

		// Draw the selected item at the top (always)
		if (!m_list->m_items.empty())
		{
			// Draw item in place.
			m_list->drawItemStandalone(m_selected, bottom());
		}

		// Draw a drop down button.
		const float btnSize = this->size().y;
		ei::Rect2D btnFrame {{right() - btnSize, bottom()}, {right(), top()}};
		const bool mouseOverBtn = isMouseOver(GUIManager::getMouseState().position);
		GUIManager::theme().drawButton(btnFrame, mouseOverBtn, false, true);
		const float arrowMargin = btnSize * 0.25f;
		btnFrame.min += arrowMargin;
		btnFrame.max -= arrowMargin;
		GUIManager::theme().drawArrowButton(btnFrame,
			m_list->isVisible() ? SIDE::BOTTOM : SIDE::LEFT,
			mouseOverBtn);
	}


	bool DropDownMenu::processInput(const struct MouseState & _mouseState)
	{
		if(_mouseState.btnClicked(0))
		{
			if(m_list->isVisible())
				GUIManager::popups().closePopup(m_list.get());
			else
				GUIManager::popups().showPopup((WidgetPtr&)m_list, this, PopupClosingCondition::OTHER_INTERACTION, true);
		}
		return true;

		return Widget::processInput(_mouseState);
	}


	void DropDownMenu::setOnSelect(OnSelect _callback)
	{
		m_onSelect = _callback;
	}


	void DropDownMenu::updateSubWidget()
	{
		RefFrame listFrame {
			left(),
			right(),
			bottom() - m_list->m_itemHeight * m_list->m_items.size(),
			top()
		};
		m_list->setExtent(listFrame);
	}


	void DropDownMenu::onExtentChanged()
	{
		Widget::onExtentChanged();
		updateSubWidget();
	}

}} // namespace ca::gui
