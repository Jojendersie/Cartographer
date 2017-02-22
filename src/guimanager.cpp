#pragma once

#include "ca/gui.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>

namespace ca { namespace gui {
	using namespace pa;

	std::unique_ptr<GUIManager> g_manager;

	void GUIManager::init(std::shared_ptr<class IRenderBackend> _renderer, std::shared_ptr<class ITheme> _theme, int _width, int _height)
	{
		g_manager.reset(new GUIManager);
		if(!_renderer)
			return logError("[ca::gui] No renderer given for initialization!");
		if(!_theme)
			return logError("[ca::gui] No theme given for initialization!");
		g_manager->m_renderer = _renderer;
		g_manager->m_theme = _theme;
		// Use an internal light-weight frame as container
		g_manager->m_topFrame = std::make_shared<Group>();
		g_manager->m_topFrame->setExtent(ei::Vec2(0.0f), ei::Vec2((float)_width, (float)_height));
		// Push some (infinite) initial top-level clip region
		g_manager->m_clipRegionStack.push(ei::IVec4(0, 0x7fffffff, 0, 0x7fffffff));

		g_manager->m_keyboardFocus = nullptr;
		g_manager->m_mouseFocus = nullptr;
		g_manager->m_stickyKeyboardFocus = false;
		g_manager->m_stickyMouseFocus[0] = false;
		g_manager->m_stickyMouseFocus[1] = false;
		logInfo("[ca::gui] Initialized GUIManager.");
	}

	void GUIManager::exit()
	{
		g_manager.reset(nullptr);
		logInfo("[ca::gui] Released GUIManager.");
	}

	void GUIManager::onResize(int _width, int _height)
	{
	}

	void GUIManager::add(WidgetPtr _widget, unsigned _innerLayer)
	{
		if(!g_manager)
			return logError("Uninitialized GUIManager! Cannot add components!");
		g_manager->m_topFrame->add(_widget, _innerLayer);
	}

	void GUIManager::add(uint _name, WidgetPtr _widget, unsigned _innerLayer)
	{
		if(!g_manager)
			return logError("Uninitialized GUIManager! Cannot add components!");
		g_manager->m_topFrame->add(_name, _widget, _innerLayer);
	}

	void GUIManager::remove(WidgetPtr _widget)
	{
		if(!g_manager)
			return logError("Uninitialized GUIManager! Cannot remove components!");
		g_manager->m_topFrame->remove(_widget);
	}

	WidgetPtr GUIManager::find(uint _name)
	{
		if(!g_manager) {
			logError("Uninitialized GUIManager! Cannot find components!");
			return WidgetPtr();
		}
		return std::move(g_manager->m_topFrame->find(_name));
	}

	void GUIManager::clear()
	{
		g_manager->m_topFrame->clear();
	}

	void GUIManager::draw()
	{
		if(!g_manager)
			return logError("Uninitialized GUIManager! Cannot draw a GUI!");
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
		ei::IVec4& top = g_manager->m_clipRegionStack.top();
		g_manager->m_renderer->beginLayer(top);
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
			logError("Uninitialized GUIManager! Cannot process input!");
			return false;
		}

		refitAllToAnchors();
		g_manager->m_mouseState = _mouseState;
		g_manager->m_cursorType = CursorType::ARROW;

		// Reset sticky-state. The component must actively regain this.
		// Otherwise some component may keep the state forever.
		g_manager->m_stickyMouseFocus[1] = false;

		if(g_manager->m_stickyMouseFocus[0] && g_manager->m_mouseFocus)
		{
			bool ret = g_manager->m_mouseFocus->processInput( _mouseState );
			g_manager->m_stickyMouseFocus[0] = g_manager->m_stickyMouseFocus[1];
			return ret;
		} else return g_manager->m_topFrame->processInput( _mouseState );
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
		if(g_manager->m_stickyMouseFocus[0])
			return g_manager->m_mouseFocus;
		return nullptr;
	}

	void GUIManager::setKeyboardFocus(Widget* _widget, bool _sticky)
	{
		g_manager->m_keyboardFocus = _widget;
		g_manager->m_stickyKeyboardFocus = _sticky;
	}

	void GUIManager::setMouseFocus(Widget* _widget, bool _sticky)
	{
		g_manager->m_mouseFocus = _widget;
		g_manager->m_stickyMouseFocus[0] = g_manager->m_stickyMouseFocus[1] = _sticky;
	}

	CursorType GUIManager::getCursorType()
	{
		return g_manager->m_cursorType;
	}

	void GUIManager::setCursorType(CursorType _type)
	{
		g_manager->m_cursorType = _type;
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