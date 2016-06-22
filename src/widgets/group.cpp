#pragma once

#include "widgets/group.hpp"
#include "backend/mouse.hpp"
#include "backend/renderbackend.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"

namespace ca { namespace gui {

	Group::Group() :
		Widget(false, false, false, false)
	{
		Widget::enable();
		// Just use extreme large coordinates to make the group always visible
		setExtent(Coord2(0.0f), Coord2(5000.0f));
	}

	void Group::draw()
	{
		if(m_visible)
		{
			for(const auto& i : m_children)
			{
				if( i.widget->isVisible() && !GUIManager::isClipped(i.widget->getRefFrame()) )
					i.widget->draw();
			}
		}
	}

	void Group::add(WidgetPtr _widget, unsigned _innerLayer)
	{
		_widget->m_parent = this;
		m_children.push_back({move(_widget), _innerLayer});
		// Move to the front of the vector as long as there are elements
		// with a larger layer (-> insertion sort).
		auto currentIt = m_children.rbegin();
		auto nextIt = currentIt + 1;
		while(nextIt != m_children.rend() && (nextIt->innerLayer > currentIt->innerLayer))
		{
			std::iter_swap(currentIt, nextIt);
			currentIt = nextIt++;
		}
	}

	void Group::remove(WidgetPtr _widget)
	{
		// Search linearly in one of the lists.
		for(auto it = m_children.begin(); it != m_children.end(); ++it)
			if(it->widget == _widget)
			{
				m_children.erase(it);
				return;
			}
	}

	bool Group::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		// Exclusive input? If so don't check children.
		if(GUIManager::getStickyMouseFocussed() != this) // TODO why is this processInput ever called, iff only the sticking element receives anything? Is it necessary for the Widget::processInput below?
		{
			// Forward to sub-elements. Use inverse order such that the visibly top
			// most component receives the input first.
			for(int i = (int)m_children.size()-1; i >= 0; --i)
			{
				WidgetPtr& e = m_children[i].widget;
				if(e->isEnabled() && e->processInput(_mouseState))
				{
					// If a group took the input change the order
					if(dynamic_cast<Group*>(e.get()) && _mouseState.anyButtonDown)
						moveToFront(i);
					return true;
				}
			}
		}
		return false;
	}

	void Group::refitToAnchors()
	{
		Widget::refitToAnchors();
		for(auto it = m_children.begin(); it != m_children.end(); ++it)
			(*it).widget->refitToAnchors();
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

}} // namespace ca::gui