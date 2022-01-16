#include "ca/gui/widgets/scrollbar.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	ScrollBar::ScrollBar() :
		m_horizontal(false),
		m_movingPos{-1.0f},
		m_totalSize{100.0f},
		m_availableSize{10.0f},
		m_intervalStart{0.0f},
		m_margin{0.0f}
	{
		setAnchorProvider(std::make_unique<SingleAnchorProvider>(this));
		setHorizontalAnchorMode(Anchorable::NO_RESIZE);
		setVerticalAnchorMode(Anchorable::PREFER_RESIZE);
		registerMouseInputComponent(this);
	}

	ScrollBar::~ScrollBar()
	{
	}

	void ScrollBar::draw() const
	{
		GUIManager::theme().drawBackgroundArea(m_refFrame);
		// Compute percentage of available area to total area and create a smaller frame
		// with the same relation to m_refFrame (in the selected dimension)
		const float relSize = ei::min(1.0f, m_availableSize / m_totalSize);
		const float relStart = m_intervalStart / m_totalSize;
		RefFrame subFrame;
		if(m_horizontal)
		{
			const float w = m_refFrame.width();
			subFrame.rect.min.x = m_refFrame.left() + relStart * w;
			subFrame.rect.max.x = subFrame.rect.min.x + ei::max(3.0f, relSize * w);
			subFrame.rect.min.y = m_refFrame.bottom();
			subFrame.rect.max.y = m_refFrame.top();
		} else {
			const float h = m_refFrame.height();
			subFrame.rect.min.x = m_refFrame.left();
			subFrame.rect.max.x = m_refFrame.right();
			subFrame.rect.min.y = m_refFrame.bottom() + relStart * h;
			subFrame.rect.max.y = subFrame.rect.min.y + ei::max(3.0f, relSize * h);
		}
		GUIManager::theme().drawButton(subFrame, (relSize < 1.0f) && isMouseOver(), false);
	}

	bool ScrollBar::processInput(Widget & _thisWidget, const MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput)
	{
		// Not moving at all, or stopped moving.
		if(_mouseState.buttons[0] != MouseState::PRESSED
			&& _mouseState.buttons[0] != MouseState::DOWN)
		{
			m_movingPos = -1.0f;
			return false;
		}

		// Starting to move, or already moving
		GUIManager::setCursorType(m_horizontal ? CursorType::RESIZE_H : CursorType::RESIZE_V);
		GUIManager::setMouseFocus(&_thisWidget, true);
		_ensureNextInput = true;

		// Pick the right values of some reused quantities
		const float widgetBegin = m_horizontal ? m_refFrame.left() : m_refFrame.bottom();
		const float widgetSize = m_horizontal ? getRefFrame().width() : getRefFrame().height();
		const float mousePos = m_horizontal ? _mouseState.position.x : _mouseState.position.y;

		// Compute world space parameters of the moveable interval
		const float relSize = ei::min(1.0f, m_availableSize / m_totalSize);	// TODO: precompute?
		const float relStart = m_intervalStart / m_totalSize;	// TODO: precompute?
		const float intervalBegin = widgetBegin + relStart * widgetSize;	// TODO: precompute?
		const float intervalSize = ei::max(3.0f, relSize * widgetSize);	// TODO: precompute?
		if(_mouseState.buttons[0] == MouseState::DOWN && _cursorOnWidget)
		{
			// Start moving.
			// Find out where the mouse is relative to the moveable interval
			m_movingPos = ei::clamp((mousePos - intervalBegin) / intervalSize, 0.0f, 1.0f);
			// If the mouse was outside the interval we want to skip the respective boundary
			// towards it. We achieve that automatically by the repositioning code below.
		}

		// Recompute a new interval start position by matching m_movingPos to mousePos.
		const float offsetPos = mousePos - intervalSize * m_movingPos;			// Move to left boundary
		const float widgetSpacePos = (offsetPos - widgetBegin) / widgetSize;	// Pos of left boundary within widget
		const float scrollSpacePos = widgetSpacePos * m_totalSize;
		const float old = m_intervalStart;
		m_intervalStart = ei::clamp(scrollSpacePos, 0.0f, ei::max(0.0f, m_totalSize - m_availableSize));
		if(old != m_intervalStart)
			m_anchorProvider->recomputeAnchors(m_refFrame);
		return true;
	}

	void ScrollBar::setHorizontalMode(const bool _horizontal)
	{
		m_horizontal = _horizontal;
		setHorizontalAnchorMode(_horizontal ? Anchorable::PREFER_RESIZE : Anchorable::NO_RESIZE);
		setVerticalAnchorMode(_horizontal ? Anchorable::NO_RESIZE : Anchorable::PREFER_RESIZE);
		m_intervalStart = ei::clamp(m_intervalStart, 0.0f, ei::max(0.0f, m_totalSize - m_availableSize));
		m_anchorProvider->recomputeAnchors(m_refFrame);
	}

	void ScrollBar::setAvailableSize(const float _availableSize)
	{
		m_presentationWidget = nullptr;
		m_availableSize = _availableSize;
		checkInterval();
	}

	void ScrollBar::setTotalSize(const float _totalSize)
	{
		m_contentWidget = nullptr;
		m_totalSize = _totalSize;
		checkInterval();
	}

	void ScrollBar::setViewArea(WidgetPtr _presentationWidget, const float _margin)
	{
		m_presentationWidget = std::move(_presentationWidget);
		m_margin = ei::max(0.0f, _margin);
		m_availableSize = getAvailableSize();
		checkInterval();
	}

	void ScrollBar::setContent(WidgetPtr _contentWidget)
	{
		m_contentWidget = std::move(_contentWidget);
		m_totalSize = m_horizontal ? m_contentWidget->getSize().x : m_contentWidget->getSize().y;
		checkInterval();
	}

	void ScrollBar::setScrollOffset(const float _amount)
	{
		m_intervalStart = _amount;
		checkInterval();
	}

	AnchorPtr ScrollBar::getAnchor() const
	{
		return getAnchorProvider()->findClosestAnchor(0.0f, m_horizontal
			? IAnchorProvider::SearchDirection::HORIZONTAL
			: IAnchorProvider::SearchDirection::VERTICAL);
	}

	float ScrollBar::getAvailableSize() const
	{
		if(m_presentationWidget)
		{
			const float fullSize = m_horizontal ?
				m_presentationWidget->getSize().x : m_presentationWidget->getSize().y;
			return ei::max(0.0f, fullSize - m_margin);
		}
		return m_availableSize;
	}

	float ScrollBar::getTotalSize() const
	{
		if(m_contentWidget)
		{
			return m_horizontal ? m_contentWidget->getSize().x : m_contentWidget->getSize().y;
		}
		return m_totalSize;
	}

	void ScrollBar::checkInterval()
	{
		const float old = m_intervalStart;
		m_intervalStart = ei::clamp(m_intervalStart, 0.0f, ei::max(0.0f, m_totalSize - m_availableSize));
		if(old != m_intervalStart)
			m_anchorProvider->recomputeAnchors(m_refFrame);
	}

	void ScrollBar::refitToAnchors()
	{
		m_availableSize = getAvailableSize();
		m_totalSize = getTotalSize();
		checkInterval();
		Widget::refitToAnchors();
	}



	ScrollBar::SingleAnchorProvider::SingleAnchorProvider(ScrollBar* _parent) :
		m_parent(_parent)
	{
		m_anchor = std::make_shared<AnchorPoint>(this);
		m_anchor->position = 0.0f;
	}

	ScrollBar::SingleAnchorProvider::~SingleAnchorProvider()
	{
		// Cause a future deletion of the anchor
		m_anchor->host = nullptr;
	}

	void ScrollBar::SingleAnchorProvider::recomputeAnchors(const RefFrame& _selfFrame)
	{
		const float old = m_anchor->position;
		if(m_parent->isHorizontal())
			m_anchor->position = _selfFrame.left() - m_parent->m_intervalStart;
		else
			m_anchor->position = _selfFrame.bottom() - m_parent->m_intervalStart;
		if(old != m_anchor->position)
			m_someChanged = true;
	}

	AnchorPtr ScrollBar::SingleAnchorProvider::findClosestAnchor(Coord _position, SearchDirection _direction) const
	{
		return m_anchor;
	}

}} // namespace ca::gui