#include "widgets/node.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

using namespace ei;

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

	ei::Vec2 NodeHandle::getConnectorDirection() const
	{
		return ei::Vec2(cos(m_angle), sin(m_angle));
	}


	
	NodeConnector::NodeConnector() :
		Widget(false, true, false, false)
	{
	}

	const int CONNECTOR_NUM_POINTS = 32;
	void NodeConnector::draw()
	{
		// Create a cubic spline.
		// As support vectors take the direction given by the node handles
		// with a lenght of 1/3 from the distance between the two nodes.
		Vec2 p0 = m_sourceNode->getRefFrame().center();
		Vec2 p3 = m_destNode->getRefFrame().center();
		float distance = len(p0 - p3) / 3.0f;
		Vec2 p1 = p0 + m_sourceNode->getConnectorDirection() * distance;
		Vec2 p2 = p3 + m_destNode->getConnectorDirection() * distance;
		Vec3 wayPoints[CONNECTOR_NUM_POINTS];
		for(int i = 0; i < CONNECTOR_NUM_POINTS; ++i)
		{
			float t = i / (CONNECTOR_NUM_POINTS - 1.0f);
			float ti = 1.0f - t;
			wayPoints[i] = Vec3( ti*ti*ti * p0 + 3.0f*ti*ti*t * p1 + 3.0f*ti*t*t * p2 + t*t*t * p3, 0.0f);
		}
		GUIManager::theme().drawLine(wayPoints, CONNECTOR_NUM_POINTS, Vec4(1.0f), Vec4(1.0f));
	}

	void NodeConnector::refitToAnchors()
	{
		m_refFrame.sides[SIDE::LEFT] = min(m_sourceNode->getPosition().x, m_destNode->getPosition().x);
		m_refFrame.sides[SIDE::BOTTOM] = min(m_sourceNode->getPosition().y, m_destNode->getPosition().y);
		m_refFrame.sides[SIDE::RIGHT] = max(m_sourceNode->getPosition().x, m_destNode->getPosition().x);
		m_refFrame.sides[SIDE::TOP] = max(m_sourceNode->getPosition().y, m_destNode->getPosition().y);
	}

}} // namespace ca::gui