#include "widgets/node.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

#include <ei/2dintersection.hpp>

using namespace ei;

namespace ca { namespace gui {

	NodeHandlePtr s_tmpMouseNode;		///< An additional node which is held by the mouse.

	NodeHandle::NodeHandle() :
		Widget(true, true, false, false),
		m_angle(0.0f),
		m_color(1.0f)
	{
		Widget::setAnchorModes(Anchorable::Mode::NO_RESIZE);
		m_clickComponent->setClickRegion(new EllipseRegion(&m_refFrame));
	}

	void NodeHandle::draw()
	{
		bool mouseOver = GUIManager::hasMouseFocus(this)
			&& m_clickComponent->getClickRegion()->isMouseOver(GUIManager::getMouseState().position);
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
		Widget::setAnchoring(SIDE::LEFT, _anchorProvider->findClosestAnchor(m_refFrame.horizontalCenter(), IAnchorProvider::SearchDirection::LEFT));
		Widget::setAnchoring(SIDE::RIGHT, _anchorProvider->findClosestAnchor(m_refFrame.horizontalCenter(), IAnchorProvider::SearchDirection::RIGHT));
		Widget::setAnchoring(SIDE::BOTTOM, _anchorProvider->findClosestAnchor(m_refFrame.verticalCenter(), IAnchorProvider::SearchDirection::DOWN));
		Widget::setAnchoring(SIDE::TOP, _anchorProvider->findClosestAnchor(m_refFrame.verticalCenter(), IAnchorProvider::SearchDirection::UP));
	}

	ei::Vec2 NodeHandle::getConnectorDirection() const
	{
		return ei::Vec2(cos(m_angle), sin(m_angle));
	}


	
	NodeConnector::NodeConnector() :
		Widget(false, true, false, false),
		m_isMouseOver(false),
		m_tmpHandleState(HandleState::ATTACHED)
	{
		m_clickComponent->setClickRegion(this, false);

		// Make sure that the mouse node is always existing.
		if(!s_tmpMouseNode)
			s_tmpMouseNode = std::make_shared<NodeHandle>();
	}

	const int CONNECTOR_NUM_POINTS = 32;
	void NodeConnector::draw()
	{
		// Control points of a cubic Bezier spline:
		Vec2 p0, p1, p2, p3;
		// As support vectors take the direction given by the node handles
		// with a lenght of 1/3 from the distance between the two nodes.
		if(m_tmpHandleState == HandleState::TMP_SRC)
			p0 = s_tmpMouseNode->getPosition();
		else {
			p0 = m_sourceNode->getRefFrame().center();
			p0 += m_sourceNode->getConnectorDirection() * m_sourceNode->getRefFrame().size() / 2.0f;
		}
		if(m_tmpHandleState == HandleState::TMP_DST)
			p3 = s_tmpMouseNode->getPosition();
		else {
			p3 = m_destNode->getRefFrame().center();
			p3 += m_destNode->getConnectorDirection() * m_destNode->getRefFrame().size() / 2.0f;
		}
		float distance = len(p0 - p3) / 3.0f;
		if(m_tmpHandleState != HandleState::ATTACHED)
			distance /= 2.0f;
		// Reuse the one existing control point from the non-temp node for the
		// tmp connection direction. This way the temp node does not need an own
		// direction and the curve is visually reduced to a quadratic spline.
		if(m_tmpHandleState == HandleState::TMP_SRC)
			p1 = p3 + m_destNode->getConnectorDirection() * distance;
		else
			p1 = p0 + m_sourceNode->getConnectorDirection() * distance;
		if(m_tmpHandleState == HandleState::TMP_DST)
			p2 = p0 + m_sourceNode->getConnectorDirection() * distance;
		else
			p2 = p3 + m_destNode->getConnectorDirection() * distance;

		Coord2 boundingRect[2];
		Vec3 wayPoints[CONNECTOR_NUM_POINTS];
		for(int i = 0; i < CONNECTOR_NUM_POINTS; ++i)
		{
			// TODO: adaptive step length to increase sample density in heigh
			// curvature areas
			float t = i / (CONNECTOR_NUM_POINTS - 1.0f);
			float ti = 1.0f - t;
			wayPoints[i] = Vec3( ti*ti*ti * p0 + 3.0f*ti*ti*t * p1 + 3.0f*ti*t*t * p2 + t*t*t * p3, 0.0f);
			boundingRect[0] = min(boundingRect[0], Coord2(wayPoints[i]));
			boundingRect[1] = max(boundingRect[1], Coord2(wayPoints[i]));
		}
		bool mouseOver = isMouseOver(GUIManager::getMouseState().position);
		Vec4 sourceColor = ei::Vec4(m_sourceNode->color() * (mouseOver ? 2.0f : 1.0f), 1.0f);
		Vec4 destColor = ei::Vec4(m_destNode->color() * (mouseOver ? 2.0f : 1.0f), 1.0f);
		GUIManager::theme().drawLine(wayPoints, CONNECTOR_NUM_POINTS, sourceColor, destColor);

		m_refFrame.sides[SIDE::LEFT] = boundingRect[0].x - 3.0f;
		m_refFrame.sides[SIDE::BOTTOM] = boundingRect[0].y - 3.0f;
		m_refFrame.sides[SIDE::RIGHT] = boundingRect[1].x + 3.0f;
		m_refFrame.sides[SIDE::TOP] = boundingRect[1].y + 3.0f;
	}

	bool NodeConnector::isMouseOver(const Coord2& _mousePos) const
	{
		if(!m_refFrame.isMouseOver(_mousePos))
			return false;

		// Move along the spline and check if any segment is closer than 3.0 pixels
		// to the mouse position.
		Vec2 p0 = m_sourceNode->getRefFrame().center();
		p0 += m_sourceNode->getConnectorDirection() * m_sourceNode->getRefFrame().size() / 2.0f;
		Vec2 p3 = m_destNode->getRefFrame().center();
		p3 += m_destNode->getConnectorDirection() * m_destNode->getRefFrame().size() / 2.0f;
		float nodeDistance = len(p0 - p3) / 3.0f;
		Vec2 p1 = p0 + m_sourceNode->getConnectorDirection() * nodeDistance;
		Vec2 p2 = p3 + m_destNode->getConnectorDirection() * nodeDistance;

		Segment2D line;
		line.a = p0;
		for(int i = 1; i < CONNECTOR_NUM_POINTS; ++i)
		{
			float t = i / (CONNECTOR_NUM_POINTS - 1.0f);
			float ti = 1.0f - t;
			line.b = Vec2( ti*ti*ti * p0 + 3.0f*ti*ti*t * p1 + 3.0f*ti*t*t * p2 + t*t*t * p3);
			// Test if the _mousePos is close to the line segment l0-l1
			if(distanceSq(line, _mousePos) <= 9.0f) {
				m_isMouseOver = true;
				m_mouseT = t;
				return true;
			}
			line.a = line.b;
		}
		m_isMouseOver = false;
		return false;
	}

	void NodeConnector::refitToAnchors()
	{
		m_refFrame.sides[SIDE::LEFT] = min(m_sourceNode->getPosition().x, m_destNode->getPosition().x);
		m_refFrame.sides[SIDE::BOTTOM] = min(m_sourceNode->getPosition().y, m_destNode->getPosition().y);
		m_refFrame.sides[SIDE::RIGHT] = max(m_sourceNode->getPosition().x, m_destNode->getPosition().x);
		m_refFrame.sides[SIDE::TOP] = max(m_sourceNode->getPosition().y, m_destNode->getPosition().y);
	}

	bool NodeConnector::processInput(const MouseState& _mouseState)
	{
		if(m_isMouseOver && _mouseState.buttons[0] == MouseState::ButtonState::DOWN)
		{
			// Tear off now
			if(m_mouseT < 0.5)
				m_tmpHandleState = HandleState::TMP_SRC;
			else
				m_tmpHandleState = HandleState::TMP_DST;
		}

		if(_mouseState.buttons[0] == MouseState::ButtonState::RELEASED)
			m_tmpHandleState = HandleState::ATTACHED;

		if(m_tmpHandleState != HandleState::ATTACHED)
		{
			s_tmpMouseNode->setPosition(_mouseState.position);
			// Get exclusive focus
			GUIManager::setMouseFocus(this, true);
		}

		return m_tmpHandleState != HandleState::ATTACHED;
	}

}} // namespace ca::gui
