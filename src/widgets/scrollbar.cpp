#include "ca/gui/widgets/scrollbar.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>

namespace ca { namespace gui {

	ScrollBar::ScrollBar() :
		m_sliderAnchor{this},
		m_presentationAnchor{this},
		m_contentAnchor{this},
		m_horizontal(false),
		m_totalSize{100.0f},
		m_availableSize{10.0f},
		m_intervalStart{0.0f},
		m_rangeOffset{0.0f},
		m_margin{0.0f},
		m_movingPos{-1.0f}
	{
		registerMouseInputComponent(this);
		linkAnchor(m_sliderAnchor.m_anchor);
	}

	ScrollBar::~ScrollBar()
	{
	}

	void ScrollBar::draw() const
	{
		GUIManager::theme().drawBackgroundArea(rectangle());
		// Compute percentage of available area to total area and create a smaller frame
		// with the same relation to m_refFrame (in the selected dimension)
		const float relSize = ei::min(1.0f, m_availableSize / m_totalSize);
		const float relStart = m_intervalStart / m_totalSize;
		ei::Rect2D subFrame;
		if(m_horizontal)
		{
			const float w = width();
			subFrame.min.x = left() + relStart * w;
			subFrame.max.x = subFrame.min.x + ei::max(3.0f, relSize * w);
			subFrame.min.y = bottom();
			subFrame.max.y = top();
		} else {
			const float h = height();
			subFrame.min.x = left();
			subFrame.max.x = right();
			subFrame.min.y = bottom() + relStart * h;
			subFrame.max.y = subFrame.min.y + ei::max(3.0f, relSize * h);
		}
		GUIManager::theme().drawButton(subFrame, (relSize < 1.0f) && (GUIManager::hasMouseFocus(this) || m_movingPos >= 0.0f), false, m_horizontal);
	}

	void ScrollBar::onExtentChanged()
	{
		// The resize of the slider is a likely point where reference widgets where
		// changed as well. To make sure we don't miss events, we also have the notification anchors.
		Widget::onExtentChanged();
		m_availableSize = getAvailableSize();
		m_totalSize = getContentSize();
		checkInterval();
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
		const float widgetBegin = m_horizontal ? left() : bottom();
		const float widgetSize = m_horizontal ? width() : height();
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
			recomputeAnchorFrame();
		return true;
	}

	void ScrollBar::setHorizontalMode(const bool _horizontal)
	{
		m_horizontal = _horizontal;
		m_intervalStart = ei::clamp(m_intervalStart, 0.0f, ei::max(0.0f, m_totalSize - m_availableSize));
		recomputeAnchorFrame();
	}

	void ScrollBar::setAvailableSize(const float _availableSize)
	{
		if(m_presentationWidget) {
			m_presentationWidget = nullptr;
			m_sliderAnchor.attach(this);
			m_presentationAnchor.detach(); // No need for further notifications
		}
		m_availableSize = _availableSize;
		checkInterval();
	}

	void ScrollBar::setContentSize(const float _contentSize, SIDE::Val _side)
	{
		if(m_horizontal && (_side != SIDE::LEFT && _side != SIDE::RIGHT))
			ca::pa::logWarning("[ca::gui::ScrollBar] Scroll bar is in horizontal mode, but change is said to be on side ", SIDE::STR_NAMES[(int)_side]);
		if(!m_horizontal && (_side != SIDE::BOTTOM && _side != SIDE::TOP))
			ca::pa::logWarning("[ca::gui::ScrollBar] Scroll bar is in vertical mode, but change is said to be on side ", SIDE::STR_NAMES[(int)_side]);
		if(_side == SIDE::LEFT || _side == SIDE::BOTTOM)
		{
			const float delta = ei::max(0.0f, m_totalSize - m_availableSize) - ei::max(0.0f, _contentSize - m_availableSize);
			m_rangeOffset -= delta;
			//m_intervalStart += delta;
		}
		m_contentWidget = nullptr;
		m_contentAnchor.detach();
		m_totalSize = _contentSize;
		checkInterval(true);
	}

	void ScrollBar::setViewArea(WidgetPtr _presentationWidget, const float _margin)
	{
		m_presentationWidget = std::move(_presentationWidget);
		m_sliderAnchor.attach(m_presentationWidget.get());
		m_presentationAnchor.attach(m_presentationWidget.get(), 0.0f, 0.0f, 0);
		m_margin = ei::max(0.0f, _margin);
		m_availableSize = getAvailableSize();
		checkInterval(true);
	}

	void ScrollBar::setContent(WidgetPtr _contentWidget)
	{
		m_contentWidget = std::move(_contentWidget);
		m_contentAnchor.attach(m_contentWidget.get(), 0.0f, 0.0f, 0);
		m_totalSize = m_horizontal ? m_contentWidget->width() : m_contentWidget->height();
		checkInterval();
	}

	void ScrollBar::setScrollOffset(const float _amount)
	{
		m_intervalStart = _amount;
		checkInterval();
	}

	void ScrollBar::setScrollOffsetTop(const float _amount)
	{
		setScrollOffset((m_totalSize - m_availableSize) - _amount);
	}


	float ScrollBar::getAvailableSize() const
	{
		if(m_presentationWidget)
		{
			const float fullSize = m_horizontal ?
				m_presentationWidget->width() : m_presentationWidget->height();
			return ei::max(0.0f, fullSize - m_margin);
		}
		return m_availableSize;
	}

	float ScrollBar::getContentSize() const
	{
		if(m_contentWidget)
		{
			return m_horizontal ? m_contentWidget->width() : m_contentWidget->height();
		}
		return m_totalSize;
	}

	void ScrollBar::checkInterval(const bool _forceAnchorReset)
	{
		const float old = m_intervalStart;
		m_intervalStart = ei::clamp(m_intervalStart, 0.0f, ei::max(0.0f, m_totalSize - m_availableSize));
		if(_forceAnchorReset || old != m_intervalStart)
			recomputeAnchorFrame();
	}


	void ScrollBar::recomputeAnchorFrame()
	{
		m_sliderAnchor.setAnchor(m_rangeOffset - m_intervalStart);
	}



	ScrollBar::SliderAnchor::SliderAnchor(ScrollBar* _parent) :
		m_parent{_parent},
		m_anchor{this}
	{
		matchGeomVersion();
	}

	void ScrollBar::SliderAnchor::attach(const IAnchorProvider* _target)
	{
		const int dim = m_parent->isHorizontal() ? 0 : 1;
		const float targetPosition = _target->getPosition(dim,0.0f);
		m_anchor.attach(
			_target,
			targetPosition, // Bottom or left
			targetPosition+m_parent->getScrollOffset(), dim);
	}

	void ScrollBar::SliderAnchor::setAnchor(float _offset)
	{
		if(m_anchor.absoluteDistance == _offset)
			return;
		// Update and set a new geom version for this primary event
		m_anchor.absoluteDistance = _offset;
		increaseGeomVersion();
		IAnchorProvider::onExtentChanged(); // Then trigger updates of others
	}

	void ScrollBar::SliderAnchor::onExtentChanged()
	{
		IAnchorProvider::onExtentChanged();
	}

	Coord ScrollBar::SliderAnchor::getPosition(int _dimension, float _relativePos) const
	{
		return m_anchor.getPosition(m_parent->isHorizontal() ? 0 : 1);
	}

	float ScrollBar::SliderAnchor::getRelativePosition(int _dimension, Coord _position) const
	{
		return 0.0f; // Only absolute position meaningful
	}

}} // namespace ca::gui