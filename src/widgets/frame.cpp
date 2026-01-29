#include "ca/gui/widgets/frame.hpp"
#include "ca/gui/widgets/scrollbar.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"

namespace ca { namespace gui {

	void childChangedCallback(Widget* _this, Widget* _child, bool _add)
	{
		Frame* f = (Frame*)_this;
		if (f->isScrollingEnabled())
			f->m_scrollbarContentSizeDirty = true;
	}


	Frame::Frame() :
		Resizeable(this),
		Moveable(this),
		m_opacity(1.0f),
		m_tiling(false),
		m_passive(false),
		m_texture(0),
		m_color{-1.0f},
		m_horizontalScrollbar{nullptr},
		m_verticalScrollbar{nullptr},
		m_scrollMargin{0.0f}
	{
		m_autoResize = false; // Overwrite group policy
		setMoveable(false);
		setResizeable(false);
		setOnChildrenChangedFunc(&childChangedCallback);
	}

	Frame::~Frame()
	{
	}

	void Frame::draw() const
	{
		if(m_visible)
		{
			if (m_scrollbarContentSizeDirty)
				const_cast<Frame*>(this)->resetScrollbarContentSize();

			// Draw the frame background
			if(m_texture)
				GUIManager::theme().drawImage(rectangle(), m_texture, m_opacity, m_tiling);
			else
				GUIManager::theme().drawBackgroundArea(rectangle(), m_opacity, m_color);

			// Set clipping region for this and all subelements
			ei::Rect2D clipRegion = rectangle();
			if (m_horizontalScrollbar) clipRegion.min.y += m_horizontalScrollbar->height();
			if (m_verticalScrollbar) clipRegion.max.x -= m_verticalScrollbar->width();
			bool vis = GUIManager::pushClipRegion(clipRegion);
			if(vis)
			{
				// Draw all contained children of the group
				Group::draw();
			}

			GUIManager::popClipRegion();
			if (vis)
			{
				// Always draw the scrollbars on top
				if (m_horizontalScrollbar)
					m_horizontalScrollbar->draw();
				if (m_verticalScrollbar)
					m_verticalScrollbar->draw();
			}
		}
	}

	void Frame::clear()
	{
		ScrollBarPtr pinH { m_horizontalScrollbar };
		ScrollBarPtr pinV { m_verticalScrollbar };
		Group::clear();
		if (pinH) this->add(pinH, 99999);
		if (pinV) this->add(pinV, 99999);
	}

	bool Frame::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		if (m_scrollbarContentSizeDirty)
			resetScrollbarContentSize();
		if(Group::processInput(_mouseState)) return true;
		// Input was not consumed by an element.
		// Reclaim the focus (a child might have got it, but didn't use it).
		GUIManager::setMouseFocus(this);
		// If there are properties try them.
		if(m_passive) return false;
		return Widget::processInput(_mouseState);
	}


	void Frame::onExtentChanged()
	{
		Widget::onExtentChanged();
		// Anchoring has resized the scroll bars appropriately
		if (isScrollingEnabled())
			m_scrollbarContentSizeDirty = true;
	}


	void Frame::setBackground(const char* _imageFile, bool _smooth, float _opacity, bool _tiling)
	{
		m_texture = GUIManager::renderBackend().getTexture(_imageFile, _smooth);
		m_opacity = _opacity;
		m_tiling = _tiling;
	}

	void Frame::setBackgroundOpacity(float _opacity)
	{
		m_opacity = _opacity;
	}


	void onScrollChangedCallback(Widget* _this, const float _amount)
	{
		// We want to move all children of the parents frame. Whether the change
		// is horizontal or vertical can be found out via pointer comparison.
		Frame* frame = (Frame*)_this->parent();
		const Coord2 delta = (_this == frame->m_horizontalScrollbar) ? Coord2{-_amount, 0.0f} : Coord2{0.0f, -_amount};

		for(auto& child :  frame->m_children) if (child.widget != frame->m_horizontalScrollbar && child.widget != frame->m_verticalScrollbar)
			child.widget->move(delta);
	}


	void Frame::setScrolling(const bool _horizontal, const bool _vertical, const float _margin, const Coord _scrollbarWidth)
	{
		m_scrollMargin = _margin;
		if (_horizontal && !m_horizontalScrollbar)
		{
			ScrollBarPtr sb(new ScrollBar);
			sb->setExtent(position(), Coord2(width()-(_vertical?_scrollbarWidth:0.0f), _scrollbarWidth));
			sb->setHorizontalMode(true);
			sb->setAvailableArea(this, sb->left() + m_scrollMargin, sb->top() - m_scrollMargin);
			sb->setAnchors(this, SIDE_FLAGS::HORIZONTAL | SIDE_FLAGS::BOTTOM);
			sb->setOnChangeFunc(&onScrollChangedCallback);
			m_horizontalScrollbar = sb.get(); // Set before add() to filter out.
			this->add(sb, 99999);
		}
		else if (!_horizontal && m_horizontalScrollbar)
		{
			m_horizontalScrollbar = nullptr;
		}

		if (_vertical && !m_verticalScrollbar)
		{
			ScrollBarPtr sb(new ScrollBar);
			sb->setExtent(position()+Coord2(width() - _scrollbarWidth, _horizontal?_scrollbarWidth:0.0f),
				Coord2(_scrollbarWidth, height()-(_horizontal?_scrollbarWidth:0.0f)));
			sb->setHorizontalMode(false);
			sb->setAvailableArea(this, sb->bottom() + m_scrollMargin, sb->top() - m_scrollMargin);
			sb->setAnchors(this, SIDE_FLAGS::VERTICAL | SIDE_FLAGS::RIGHT);
			sb->setOnChangeFunc(&onScrollChangedCallback);
			m_verticalScrollbar = sb.get(); // Set before add() to filter out.
			this->add(sb, 99999);
		}
		else if (!_vertical && m_verticalScrollbar)
		{
			m_verticalScrollbar = nullptr;
		}

		m_scrollbarContentSizeDirty = true;
	}

	
	bool Frame::isScrollingEnabled() const
	{
		return m_horizontalScrollbar || m_verticalScrollbar;
	}


	void Frame::resetScrollbarContentSize()
	{
		Coord2 contentBbMin {1e38f};
		Coord2 contentBbMax {-1e38f};
		for (auto& child : m_children) if (child.widget != m_horizontalScrollbar && child.widget != m_verticalScrollbar)
		{
			const Coord2 bbmin = child.widget->position();
			const Coord2 bbmax = bbmin + child.widget->size();
			contentBbMin = min(contentBbMin, bbmin);
			contentBbMax = max(contentBbMax, bbmax);
		}
		if (m_horizontalScrollbar)
			m_horizontalScrollbar->setContentInterval(contentBbMin.x, contentBbMax.x);
		if (m_verticalScrollbar)
			m_verticalScrollbar->setContentInterval(contentBbMin.y, contentBbMax.y);
		m_scrollbarContentSizeDirty = false;
	}

}} // namespace ca::gui