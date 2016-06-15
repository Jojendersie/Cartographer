#include "widgets/node.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace ca { namespace gui {

	NodeHandle::NodeHandle() :
		Widget(true, true, false, false),
		m_angle(0.0f),
		m_color(1.0f)
	{
		Widget::setAnchorModes(Anchorable::Mode::PREFER_MOVE);
	}

	void NodeHandle::draw()
	{
		bool mouseOver = GUIManager::hasMouseFocus(this)
			&& m_refFrame.isMouseOver(GUIManager::getMouseState().position);
		GUIManager::theme().drawNodeHandle(
			0.5f * Coord2(m_refFrame.left() + m_refFrame.right(), m_refFrame.bottom() + m_refFrame.top()),
			0.5f * m_refFrame.width(),
			mouseOver ? 2.0f * m_color : m_color);
		//GUIManager::theme().drawCheckbox(m_refFrame, true, true);
	}

	/*void NodeHandle::setAnchoring(AnchorPtr _anchor)
	{
		Widget::setAnchoring(SIDE::BOTTOM, _anchor);
		Widget::setAnchoring(SIDE::LEFT, _anchor);
	}*/

	void NodeHandle::autoAnchor(const class IAnchorProvider* _anchorProvider)
	{
		Widget::setAnchoring(SIDE::LEFT, _anchorProvider->findClosestAnchor(m_refFrame.horizontalCenter(), IAnchorProvider::SearchDirection::HORIZONTAL));
		Widget::setAnchoring(SIDE::BOTTOM, _anchorProvider->findClosestAnchor(m_refFrame.verticalCenter(), IAnchorProvider::SearchDirection::VERTICAL));
	}

}} // namespace ca::gui
