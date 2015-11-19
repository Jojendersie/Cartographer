#pragma once

#include "cagui.hpp"
#include "widgets/frame.hpp"
#include "backend/renderbackend.hpp"

namespace cag {
	std::unique_ptr<GUIManager> g_manager;

	void GUIManager::init(std::shared_ptr<class IRenderBackend> _renderer, std::shared_ptr<class ITheme> _theme)
	{
		g_manager.reset(new GUIManager);
		if(!_renderer)
		{
			// TODO Error
			return;
		}
		if(!_theme)
		{
			// TODO Error
			return;
		}
		g_manager->m_renderer = _renderer;
		g_manager->m_theme = _theme;
		// Use an internal light-weight frame as container
		g_manager->m_topFrame = std::make_shared<Frame>(false, false, false, false);
		g_manager->m_topFrame->setBackgroundOpacity(0.0f);
		// TODO: push some initial top-level clip region
	}

	void GUIManager::exit()
	{
		g_manager.reset(nullptr);
	}

	void GUIManager::onResize(int _width, int _height)
	{
	}

	void GUIManager::add(WidgetPtr _widget)
	{
		g_manager->m_topFrame->add(_widget);
	}

	void GUIManager::remove(WidgetPtr _widget)
	{
		g_manager->m_topFrame->remove(_widget);
	}

	void GUIManager::draw()
	{
		g_manager->m_topFrame->draw();
		// TODO: Assert g_manager->m_clipRegionStack.size() == 1
	}

	bool GUIManager::pushClipRegion(Coord _l, Coord _r, Coord _b, Coord _t)
	{
		auto& top = g_manager->m_clipRegionStack.top();
		// Convert Coord to pixels and compute a minimum between the current top and the new rect.
		ei::IVec4 area;
		area.x = ei::max(ei::round(_l), top.x);
		area.y = ei::min(ei::round(_r), top.y);
		area.z = ei::max(ei::round(_b), top.z);
		area.w = ei::min(ei::round(_t), top.w);
		g_manager->m_clipRegionStack.push(area);
		return ei::max(0, area.y - area.x) * ei::max(0, area.z - area.w) > 0;
	}

	void GUIManager::popClipRegion()
	{
		g_manager->m_clipRegionStack.pop();
		// Set old clip region (there is at least our initial one)
		auto& top = g_manager->m_clipRegionStack.top();
		g_manager->m_renderer->setClippingRegion(top.x, top.y, top.z, top.w);
	}

	bool GUIManager::processInput(const MouseState& _mouseState)
	{
		return g_manager->m_topFrame->processInput( _mouseState );
	}

	bool GUIManager::hasFocus(const WidgetPtr& _widget)
	{
		return hasFocus(_widget.get());
	}

	bool GUIManager::hasFocus(const Widget* _widget)
	{
		// Only if enabled...
		while(_widget->isInputReceivable() && _widget->isEnabled())
		{
			// Go along the parent pointers to check if this one is the first in all
			// hierarchy levels.
			if(!_widget->parent()) return true; // Reached the root TODO: check list of roots in GUIManager
			if(_widget->parent()->isChildFocused(_widget))
				_widget = _widget->parent();
		}
		return false;
	}

	WidgetPtr GUIManager::getFocussed()
	{
		return WidgetPtr();
	}

	void GUIManager::setFocus(WidgetPtr _widget)
	{
	}

	IRenderBackend* GUIManager::getRenderBackend()
	{
		return nullptr;
	}

	ITheme* GUIManager::getTheme()
	{
		return nullptr;
	}

	int GUIManager::getWidth()
	{
		return 0;
	}

	int GUIManager::getHeight()
	{
		return 0;
	}

} // namespace cag