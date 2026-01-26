#include "ca/gui/widgets/scrollbar.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>

namespace ca { namespace gui {

	ScrollBar::ScrollBar() :
		m_sliderAnchor{this},
		m_availableStart{this},
		m_availableEnd{this},
		m_horizontal(false),
		m_movingPos{-1.0f},
		m_availableInterval{0.0f, 1.0f},
		m_contentInterval{0.0f, 0.0f}
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
		Coord2 unionInterval {
			ei::min(m_contentInterval.x, 0.0f),
			ei::max(m_contentInterval.y, m_availableInterval.y - m_availableInterval.x)
		};
		const float unionLen = unionInterval.y - unionInterval.x;
		const float relSize = ei::min(1.0f, (m_availableInterval.y - m_availableInterval.x) / unionLen);
		const float relStart = -unionInterval.x / unionLen;

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

		const float newStart = m_availableStart.getPosition(m_horizontal?0:1);
		const float newEnd = m_availableEnd.getPosition(m_horizontal?0:1);
		const bool isResize = (newEnd - newStart) != (m_availableInterval.y - m_availableInterval.x);
		if (m_availableInterval.x != newStart && isResize && m_contentInterval.x != m_contentInterval.y)
			m_contentInterval += m_availableInterval.x - newStart;
		m_availableInterval.x = newStart;
		m_availableInterval.y = newEnd;
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
		_ensureNextInput = true;

		// Pick the right values of some reused quantities
		const float widgetBegin = m_horizontal ? left() : bottom();
		const float widgetSize = m_horizontal ? width() : height();
		const float mousePos = m_horizontal ? _mouseState.position.x : _mouseState.position.y;

		// Compute world space parameters of the moveable interval
		const float availableSize = m_availableInterval.y - m_availableInterval.x;
		Coord2 unionInterval {
			ei::min(m_contentInterval.x, 0.0f),
			ei::max(m_contentInterval.y, availableSize)
		};
		const float unionLen = unionInterval.y - unionInterval.x;
		const float relSize = ei::min(1.0f, availableSize / unionLen);
		const float relStart = -unionInterval.x / unionLen;
		//const float relSize = m_relativeSize;
		//const float relStart = m_relativeStart;
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
		// Inverse of relSize computation:
		float newOffset = -widgetSpacePos * unionLen - unionInterval.x;
		// Compute the valid range for m_offset and restrict it if necessary
		const float offsetRangeMin = ei::min(0.0f, availableSize - m_contentInterval.y); // Move towards the right by shifting components left
		const float offsetRangeMax = ei::max(0.0f, - m_contentInterval.x);
		newOffset = ei::clamp(roundf(newOffset), offsetRangeMin, offsetRangeMax);
		if (newOffset != 0.0f)
		{
			m_contentInterval += newOffset;
			if (m_onChange) m_onChange(this, -newOffset);
			m_sliderAnchor.moveAnchor(newOffset);
		}
		return true;
	}

	void ScrollBar::setHorizontalMode(const bool _horizontal)
	{
		m_horizontal = _horizontal;
	}


	void ScrollBar::setAvailableArea(IAnchorProvider* _area, Coord _minPos, Coord _maxPos)
	{
		// If the content interval is non-empty, move it such that it stays where it is
		if (m_contentInterval.x != m_contentInterval.y)
			m_contentInterval += _minPos - m_availableInterval.x;
		const int dim = m_horizontal?0:1;
		m_availableStart.attach(_area, _area->getPosition(dim, 0.0f), _minPos, dim);
		m_availableEnd.attach(_area, _area->getPosition(dim, 1.0f), _maxPos, dim);
		m_availableInterval = Coord2{_minPos, _maxPos};
	}


	void ScrollBar::setContentInterval(const Coord _min, const Coord _max)
	{
		m_contentInterval = Coord2{_min, _max} - m_availableInterval.x;
	}


	/*void ScrollBar::setScrollOffset(const float _amount)
	{
		const float old = m_intervalStart;
		m_intervalStart = _amount;
		checkInterval();
		if (old != m_intervalStart && m_onChange)
			m_onChange(this, m_intervalStart - old);
	}

	void ScrollBar::setScrollOffsetTop(const float _amount)
	{
		setScrollOffset((m_totalSize - m_availableSize) - _amount);
	}*/


	float ScrollBar::getAvailableSize() const
	{
		return m_availableInterval.y - m_availableInterval.x;
	}


	Coord2 ScrollBar::getContentInterval() const
	{
		return m_contentInterval + m_availableInterval.x;
	}


	ScrollBar::SliderAnchor::SliderAnchor(ScrollBar* _parent) :
		m_parent{_parent},
		m_anchor{this}
	{
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
		IAnchorProvider::onExtentChanged(); // Then trigger updates of others
	}


	void ScrollBar::SliderAnchor::moveAnchor(float _offset)
	{
		if (_offset == 0.0f)
			return;
		m_anchor.absoluteDistance += _offset;
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

	void ScrollBar::SliderAnchor::refitToAnchors()
	{
		// Check for update cycles and for general anchoring ability.
		if(!IAnchorable::startRefit()) return;
		IAnchorProvider::onExtentChanged();
		IAnchorable::endRefit();
	}

}} // namespace ca::gui