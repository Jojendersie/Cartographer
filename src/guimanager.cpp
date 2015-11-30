#pragma once

#include "cagui.hpp"
#include "widgets/frame.hpp"
#include "backend/renderbackend.hpp"
#include "ca/gui/core/error.hpp"

namespace ca { namespace gui {
	std::unique_ptr<GUIManager> g_manager;

	void GUIManager::init(std::shared_ptr<class IRenderBackend> _renderer, std::shared_ptr<class ITheme> _theme, int _width, int _height)
	{
		g_manager.reset(new GUIManager);
		if(!_renderer)
			return error("No renderer given for initialization!");
		if(!_theme)
			return error("No theme given for initialization!");
		g_manager->m_renderer = _renderer;
		g_manager->m_theme = _theme;
		// Use an internal light-weight frame as container
		g_manager->m_topFrame = std::make_shared<Frame>(false, false, false, false);
		g_manager->m_topFrame->setBackgroundOpacity(0.0f);
		g_manager->m_topFrame->setExtent(ei::Vec2(0.0f), ei::Vec2((float)_width, (float)_height));
		// Push some (infinite) initial top-level clip region
		g_manager->m_clipRegionStack.push(ei::IVec4(0x80000000, 0x7fffffff, 0x80000000, 0x7fffffff));

		g_manager->m_keyboardFocus = nullptr;
		g_manager->m_mouseFocus = nullptr;
		g_manager->m_stickyKeyboardFocus = false;
		g_manager->m_stickyMouseFocus = false;
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
		if(!g_manager)
			return error("Uninitialized GUIManager! Cannot add components!");
		g_manager->m_topFrame->add(_widget);
	}

	void GUIManager::remove(WidgetPtr _widget)
	{
		if(!g_manager)
			return error("Uninitialized GUIManager! Cannot remove components!");
		g_manager->m_topFrame->remove(_widget);
	}

	void GUIManager::draw()
	{
		if(!g_manager)
			return error("Uninitialized GUIManager! Cannot draw a GUI!");
		refitAllToAnchors();
		g_manager->m_renderer->beginDraw();
		g_manager->m_topFrame->draw();
		g_manager->m_renderer->endDraw();
		// TODO: Assert g_manager->m_clipRegionStack.size() == 1
	}

	bool GUIManager::pushClipRegion(const RefFrame& _rect)
	{
		auto& top = g_manager->m_clipRegionStack.top();
		// Convert Coord to pixels and compute a minimum between the current top and the new rect.
		ei::IVec4 area;
		area.x = ei::max(ei::round(_rect.left()), top.x);
		area.y = ei::min(ei::round(_rect.right()), top.y);
		area.z = ei::max(ei::round(_rect.bottom()), top.z);
		area.w = ei::min(ei::round(_rect.top()), top.w);
		g_manager->m_clipRegionStack.push(area);
		bool nonEmpty = ei::max(0, area.y - area.x) * ei::max(0, area.w - area.z) > 0;
		if(nonEmpty)
			g_manager->m_renderer->beginLayer(area);
		return nonEmpty;
	}

	void GUIManager::popClipRegion()
	{
		g_manager->m_clipRegionStack.pop();
		// Set old clip region (there is at least our initial one)
		auto& top = g_manager->m_clipRegionStack.top();
	}

	bool GUIManager::isClipped(const RefFrame& _rect)
	{
		auto& top = g_manager->m_clipRegionStack.top();
		return ei::round(_rect.left()) >= top.y
			|| ei::round(_rect.right()) <= top.x
			|| ei::round(_rect.bottom()) >= top.w
			|| ei::round(_rect.top()) <= top.z;
	}

	bool GUIManager::processInput(const MouseState& _mouseState)
	{
		if(!g_manager) {
			error("Uninitialized GUIManager! Cannot process input!");
			return false;
		}
		refitAllToAnchors();
		g_manager->m_mouseState = _mouseState;
		if(g_manager->m_stickyMouseFocus && g_manager->m_mouseFocus)
			return g_manager->m_mouseFocus->processInput( _mouseState );
		else return g_manager->m_topFrame->processInput( _mouseState );
	}

	const MouseState& GUIManager::getMouseState()
	{
		return g_manager->m_mouseState;
	}

	bool GUIManager::hasKeyboardFocus(const WidgetPtr& _widget)
	{
		return g_manager->m_keyboardFocus == _widget.get();
	}

	bool GUIManager::hasKeyboardFocus(const Widget* _widget)
	{
		return g_manager->m_keyboardFocus == _widget;
	}

	bool GUIManager::hasMouseFocus(const WidgetPtr& _widget)
	{
		return g_manager->m_mouseFocus == _widget.get();
	}

	bool GUIManager::hasMouseFocus(const Widget* _widget)
	{
		return g_manager->m_mouseFocus == _widget;
	}

	Widget* GUIManager::getKeyboardFocussed()
	{
		return g_manager->m_keyboardFocus;
	}

	Widget * GUIManager::getStickyKeyboardFocussed()
	{
		if(g_manager->m_stickyKeyboardFocus)
			return g_manager->m_keyboardFocus;
		return nullptr;
	}

	Widget* GUIManager::getMouseFocussed()
	{
		return g_manager->m_mouseFocus;
	}

	Widget * GUIManager::getStickyMouseFocussed()
	{
		if(g_manager->m_stickyMouseFocus)
			return g_manager->m_mouseFocus;
		return nullptr;
	}

/*	bool GUIManager::hasFocus(const WidgetPtr& _widget)
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
			else return false;
		}
		return false;
	}

	WidgetPtr GUIManager::getFocussed()
	{
		return WidgetPtr();
	}

	void GUIManager::setFocus(WidgetPtr _widget)
	{
	}*/

	void GUIManager::setKeyboardFocus(Widget* _widget, bool _sticky)
	{
		g_manager->m_keyboardFocus = _widget;
		g_manager->m_stickyKeyboardFocus = _sticky;
	}

	void GUIManager::setMouseFocus(Widget* _widget, bool _sticky)
	{
		g_manager->m_mouseFocus = _widget;
		g_manager->m_stickyMouseFocus = _sticky;
	}

	IRenderBackend& GUIManager::renderBackend()
	{
		return *g_manager->m_renderer;
	}

	ITheme& GUIManager::theme()
	{
		return *g_manager->m_theme;
	}

	int GUIManager::getWidth()
	{
		return (int)g_manager->m_topFrame->getSize().x;
	}

	int GUIManager::getHeight()
	{
		return (int)g_manager->m_topFrame->getSize().y;
	}

	void GUIManager::refitAllToAnchors()
	{
		static int s_numRunningRefits = 0;
		static int s_currentRefit = 0;
		if(IAnchorProvider::someAnchorChanged())
		{
			++s_numRunningRefits;
			while(++s_currentRefit <= s_numRunningRefits)
			{
				IAnchorProvider::resetChangedStatus();
				// Recursively call for all components
				g_manager->m_topFrame->refitToAnchors();
				// Changing components might have changed anchor points again
				if(IAnchorProvider::someAnchorChanged() && s_numRunningRefits < 3)
					++s_numRunningRefits;
			}
			s_numRunningRefits = 0;
			s_currentRefit = 0;
		}
	}

}} // namespace ca::gui