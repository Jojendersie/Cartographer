#include "ca/gui/widgets/group.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"

namespace ca { namespace gui {

	Group::Group() :
		m_autoResize{false}
	{
		setSize(Coord2{10000.0f});
	}

	Group::Group(bool _autoResize) :
		m_autoResize{_autoResize}
	{
	}

	Group::~Group()
	{
		if(m_autoResize)
		{
			// Somewhat wasted, if the children go away with the group,
			// so check the reference counter first.
			for(auto& i : m_children) if (i.widget->getRefCount() > 1)
				i.widget->removeOnExtentChangeFunc(this);
		}
	}

	void Group::draw() const
	{
		if(m_visible)
		{
			for(const auto& i : m_children)
			{
				if( i.widget->isVisible() && !GUIManager::isClipped(*i.widget) )
					i.widget->draw();
			}
		}
	}

	void Group::add(WidgetPtr _widget, unsigned _innerLayer)
	{
		_widget->setParent(this);
		m_children.push_back({_widget, _innerLayer});
		// Move to the front of the vector as long as there are elements
		// with a larger layer (-> insertion sort).
		auto currentIt = m_children.rbegin();
		auto nextIt = currentIt + 1;
		while(nextIt != m_children.rend() && (nextIt->innerLayer > currentIt->innerLayer))
		{
			std::iter_swap(currentIt, nextIt);
			currentIt = nextIt++;
		}

		if(m_autoResize)
		{
			receiveExtentChange(_widget.get());
			_widget->addOnExtentChangeFunc(this);
		}
	}

	void Group::add(uint _name, WidgetPtr _widget, unsigned _innerLayer)
	{
		add(_widget, _innerLayer);
		m_nameMap.add(_name, std::move(_widget));
	}

	void Group::remove(WidgetPtr _widget)
	{
		if(m_autoResize)
			_widget->removeOnExtentChangeFunc(this);

		for(auto it : m_nameMap)
			if(it.data() == _widget)
		{
			m_nameMap.remove(it.key());
			break;
		}
		// Search linearly in one of the lists.
		for(auto it = m_children.begin(); it != m_children.end(); ++it)
			if(it->widget == _widget)
			{
				m_children.erase(it);
				return;
			}
	}

	WidgetPtr Group::find(uint _name)
	{
		auto it = m_nameMap.find(_name);
		if(it)
			return it.data();
		return WidgetPtr();
	}

	void Group::clear()
	{
		m_children.clear();
	}

	bool Group::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		// Exclusive input? If so don't check children.
		// Can happen for frames (derived from Group) which are moved or resized
		// for example.
		if(GUIManager::getStickyMouseFocussed() != this)
		{
			// Forward to sub-elements. Use inverse order such that the visibly top
			// most component receives the input first.
			const int num_children = (int)m_children.size();
			for(int i = num_children-1; i >= 0; --i)
			{
				Widget* e = m_children[i].widget.get();
				if(e->isVisible())
				{
					const bool mouseOnWidget = e->isMouseOver(GUIManager::getMouseState().position);
					if(mouseOnWidget)
						GUIManager::setMouseOver(e);
					if(e->isEnabled()
						&& mouseOnWidget
						&& e->processInput(_mouseState))
					{
						// If something got deleted/added stop further processing
						if(num_children != (int)m_children.size()) return true;
						// If a group took the input change the order
						const bool isGroup = dynamic_cast<Group*>(e) != nullptr;
						if(isGroup && _mouseState.anyButtonDown && i != (num_children-1))
							moveToFront(i);
						return true;
					}
					// If something got deleted/added stop further processing
					if(num_children != (int)m_children.size()) return true;
				}
			}
		}
		return false;
	}


	void Group::onExtentChanged()
	{
		const bool ar = m_autoResize;
		m_autoResize = false;
		Widget::onExtentChanged();
		if(ar)
		{
			m_autoResize = ar;
			//receiveExtentChange(m_children[0].widget.get());
		}
	}


	void Group::moveToFront(size_t _index)
	{
		// Move towards back (front in rendering) and keep relative order of
		// everything else. Do not rotate further if the level changes.
		auto currentIt = m_children.begin() + _index;
		auto nextIt = currentIt + 1;
		while(nextIt != m_children.end() && (nextIt->innerLayer <= currentIt->innerLayer))
		{
			std::iter_swap(currentIt, nextIt);
			currentIt = nextIt++;
		}
	}


	void Group::receiveExtentChange(const Widget* _origin)
	{
		if(!m_autoResize) return;
		if(m_children.size() == 1)
			silentSetFrame(_origin->left(), _origin->bottom(), _origin->right(), _origin->top());
		else
		// TODO: proper reaction to downsize
			silentSetFrame(ei::min(left(), _origin->left()),
				ei::min(bottom(), _origin->bottom()),
				ei::max(right(), _origin->right()),
				ei::max(top(), _origin->top()));
		// Here comes the expensive part -- make sure that no anchors move around.
		// IDEA to get rid of this: have to reference frames -- one for the group size, the other to anchor the internal stuff.
		resetAnchors();
		for(auto& c : m_children)
			c.widget->resetAnchors();
	}

}} // namespace ca::gui