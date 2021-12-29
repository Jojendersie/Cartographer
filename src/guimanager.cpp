#include "ca/gui.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>
#include <ei/2dintersection.hpp>
#include <ctime>

namespace ca { namespace gui {
	using namespace pa;
	using namespace ei;

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
		g_manager->m_topFrame->setAnchorProvider(std::make_unique<BorderAnchorProvider>());
		// Push some (infinite) initial top-level clip region
		g_manager->m_clipRegionStack.push(ei::IVec4(0, 0x7fffffff, 0, 0x7fffffff));

		g_manager->m_keyboardFocus = nullptr;
		g_manager->m_mouseFocus = nullptr;
		g_manager->m_stickyMouseFocus = false;
		g_manager->m_lastMouseMoveTime = 0.0f;
		logInfo("[ca::gui] Initialized GUIManager.");
	}

	void GUIManager::exit()
	{
		g_manager->m_popupStack.clear();
		g_manager->m_topFrame = nullptr;
		g_manager.reset(nullptr);
		logInfo("[ca::gui] Released GUIManager.");
	}

	void GUIManager::onResize(int _width, int _height)
	{
		g_manager->m_topFrame->setExtent(ei::Vec2(0.0f), ei::Vec2((float)_width, (float)_height));
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
		// Draw info popups on top of everything else.
		for(auto& it : g_manager->m_popupStack)
			it->draw();
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

		// Show and hide popups
		float now = clock() / float(CLOCKS_PER_SEC);
		if(_mouseState.position != g_manager->m_mouseState.position
			|| _mouseState.anyButtonDown || _mouseState.anyButtonPressed)
		{
			g_manager->m_lastMouseMoveTime = now;
			// Hide if this mouse move is farther away then the last.
			if(!g_manager->m_popupStack.empty())
			{
				float dist = max(0.0f, distance(_mouseState.position, g_manager->m_popupStack.back()->getRefFrame().rect));
				if(dist > g_manager->m_cursorToPopupDistance)
				{
					g_manager->m_popupStack.back()->hide();
					g_manager->m_popupStack.pop_back();
				}
				g_manager->m_cursorToPopupDistance = dist;
			}
		} else {
			// Check if the current widget in focus has a popup and if enough
			// time passed to show it.
			if( now - g_manager->m_lastMouseMoveTime > 0.33f
				&& g_manager->m_mouseFocus
				&& g_manager->m_mouseFocus->getRefFrame().isMouseOver(_mouseState.position)
				&& g_manager->m_mouseFocus->getInfoPopup()
			)
				showPopup(g_manager->m_mouseFocus->getInfoPopup(), g_manager->m_mouseFocus);
		}

		g_manager->m_mouseState = _mouseState;
		g_manager->m_cursorType = CursorType::ARROW;

		// Reset sticky-state. The component must actively regain this.
		// Otherwise some component may keep the state forever.
		bool wasStickyMouseFocus = g_manager->m_stickyMouseFocus;
		g_manager->m_stickyMouseFocus = false;
		// If the focussed element is invisible release its focus.
		if(g_manager->m_mouseFocus && !g_manager->m_mouseFocus->isVisible())
			g_manager->m_mouseFocus = nullptr;

		if(wasStickyMouseFocus && g_manager->m_mouseFocus)
		{
			bool ret = g_manager->m_mouseFocus->processInput( _mouseState );
			return ret;
		} else {
			// First process popups (they overlay the rest)
			for(auto& it : g_manager->m_popupStack)
				if(it->processInput( _mouseState )) return true;
			return g_manager->m_topFrame->processInput( _mouseState );
		}
	}

	bool GUIManager::processInput(const KeyboardState & _keyboardState)
	{
		if(!g_manager) {
			logError("Uninitialized GUIManager! Cannot process input!");
			return false;
		}

		g_manager->m_keyboardState = _keyboardState;

		// If the focussed element is invisible release its focus.
		if(g_manager->m_keyboardFocus && !g_manager->m_keyboardFocus->isVisible())
			g_manager->m_keyboardFocus = nullptr;
		if(g_manager->m_keyboardFocus)
		{
			return g_manager->m_keyboardFocus->processInput(_keyboardState);
		}
		return false;
	}

	const MouseState& GUIManager::getMouseState()
	{
		return g_manager->m_mouseState;
	}

	const KeyboardState & GUIManager::getKeyboardState()
	{
		return g_manager->m_keyboardState;
	}

	bool GUIManager::hasKeyboardFocus(const WidgetPtr& _widget)
	{
		return g_manager && g_manager->m_keyboardFocus == _widget.get();
	}

	bool GUIManager::hasKeyboardFocus(const Widget* _widget)
	{
		return g_manager && g_manager->m_keyboardFocus == _widget;
	}

	bool GUIManager::hasMouseFocus(const WidgetPtr& _widget)
	{
		return g_manager && g_manager->m_mouseFocus == _widget.get();
	}

	bool GUIManager::hasMouseFocus(const Widget* _widget)
	{
		return g_manager && g_manager->m_mouseFocus == _widget;
	}

	Widget* GUIManager::getKeyboardFocussed()
	{
		return g_manager->m_keyboardFocus;
	}

	Widget * GUIManager::getStickyKeyboardFocussed()
	{
		return g_manager->m_keyboardFocus;
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

	void GUIManager::setKeyboardFocus(Widget* _widget)
	{
		if(_widget == g_manager->m_keyboardFocus) return;

		if(g_manager->m_keyboardFocus)
			g_manager->m_keyboardFocus->onKeyboardFocus(false);
		g_manager->m_keyboardFocus = _widget;
		if(_widget)
			_widget->onKeyboardFocus(true);
	}

	void GUIManager::setMouseFocus(Widget* _widget, bool _sticky)
	{
		g_manager->m_mouseFocus = _widget;
		g_manager->m_stickyMouseFocus = _sticky;
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

	void GUIManager::showPopup(WidgetPtr & _popup, const Widget* _originator)
	{
		if(_popup->isVisible()) return;
		if(!_originator->isVisible()) return;

		// Try to place the popup at the bottom right of the cursor.
		Coord2 pos = g_manager->m_mouseState.position + Coord2(12.0f, -12.0f);
		pos.y -= _popup->getSize().y;
		// If there is not enough place to the right side move to the left.
		float maxW = float(getWidth());
		if(_popup->parent())
			maxW = _popup->parent()->getRefFrame().right();
		pos.x += min(0.0f, maxW - (pos.x + _popup->getSize().x));
		// If it does not fit verticaly toggle it upward.
		float minH = 0.0f;
		if(_popup->parent())
			minH = _popup->parent()->getRefFrame().bottom();
		if(pos.y - _popup->getSize().y < minH)
			pos.y += _popup->getSize().y + 14.0f;

		// Show and track the popup
		_popup->setPosition(pos);
		_popup->showAsPopup(_originator);
		g_manager->m_popupStack.push_back(_popup);
		g_manager->m_cursorToPopupDistance = max(0.0f, distance(
			g_manager->m_mouseState.position,
			_popup->getRefFrame().rect));
	}

	const IAnchorProvider* GUIManager::getAnchorProvider()
	{
		return g_manager->m_topFrame->getAnchorProvider();
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
				// Also refit popups (if visible). They are not necessarily part of the hierarchy.
				for(auto& it : g_manager->m_popupStack)
					it->refitToAnchors();
				// Changing components might have changed anchor points again
				if(IAnchorProvider::someAnchorChanged() && s_numRunningRefits < 3)
					++s_numRunningRefits;
			}
			s_numRunningRefits = 0;
			s_currentRefit = 0;
		}
	}

}} // namespace ca::gui