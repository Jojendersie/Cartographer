#include "ca/gui/widgets/node.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/properties/regionshapes.hpp"

#include <ei/2dintersection.hpp>

using namespace ei;

namespace ca { namespace gui {

	// A uniform grid search structure to find the closest node fast.
	// The cell size is always 16x16 pixel. Using snapRadius x snapRadius
	// pixels would be more efficient most time, but then changing the radius
	// requires a rebuild.
	//
	// Approach disambonded: Rebuild if resolution changes, Rebuild if node moves.
	// -> too many rebuilds make it more difficult and not more efficient.
	/*class NodeGrid
	{
	public:
		static float s_snapRadius;

		static void addNode(NodeHandle* _node)
		{
			IVec2 cellPos = IVec2(_node->getPosition() / 16.0f);
			int cellIndex = cellPos.x + cellPos.y;

			s_cells[cellIndex].nodes.push_back(_node);
		}

		static void removeNode(NodeHandle* _node);
	private:
		struct Cell
		{
			std::vector<NodeHandle*> nodes;
		};

		static std::vector<Cell> s_cells;
	};
	float NodeGrid::s_snapRadius = 10.0f;*/

	/// A simple list of all existing nodes to find the closest ones.
	class NodeList
	{
	public:
		static std::vector<NodeHandle*> s_nodes;
		static float s_snapRadiusSq;

		static void addNode(NodeHandle* _node)
		{
			s_nodes.push_back(_node);
		}

		static void removeNode(NodeHandle* _node)
		{
			// Linear search for the address of the node
			for(size_t i = 0; i < s_nodes.size(); ++i)
			{
				if(s_nodes[i] == _node)
				{
					// Replace with last node. Order is not important.
					s_nodes[i] = s_nodes.back();
					s_nodes.pop_back();
					return;
				}
			}
			//eiAssert(false, "Node to remove was not found.");
		}

		static NodeHandle* findClosest(Coord2 _position)
		{
			float minDistSq = s_snapRadiusSq;
			NodeHandle* closestNode = nullptr;
			// Linear search for the closest node.
			// If nothing is as close as the search radius the method will return
			// the default nullptr.
			for(size_t i = 0; i < s_nodes.size(); ++i)
			{
				float distSq = lensq(s_nodes[i]->getPosition() - _position);
				if(distSq <= minDistSq)
				{
					minDistSq = distSq;
					closestNode = s_nodes[i];
				}
			}
			return closestNode;
		}
	};
	float NodeList::s_snapRadiusSq = 100.0f;
	std::vector<NodeHandle*> NodeList::s_nodes;



	NodeHandlePtr s_tmpMouseNode;		///< An additional node which is held by the mouse.

	NodeHandle::NodeHandle(bool _register) :
		Clickable(this),
		m_color(1.0f),
		m_angle(0.0f)
	{
		Widget::setAnchorModes(Anchorable::Mode::NO_RESIZE);
		Widget::setRegion(std::make_unique<EllipseRegion>(&m_refFrame));

		if(_register)
			NodeList::addNode(this);
	}

	NodeHandle::~NodeHandle()
	{
		NodeList::removeNode(this);
	}

	void NodeHandle::draw() const
	{
		bool mouseOver = GUIManager::hasMouseFocus(this)
			&& getRegion()->isMouseOver(GUIManager::getMouseState().position);
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
		Widget::setAnchor(SIDE::LEFT, _anchorProvider->findClosestAnchor(m_refFrame.horizontalCenter(), IAnchorProvider::SearchDirection::LEFT));
		Widget::setAnchor(SIDE::RIGHT, _anchorProvider->findClosestAnchor(m_refFrame.horizontalCenter(), IAnchorProvider::SearchDirection::RIGHT));
		Widget::setAnchor(SIDE::BOTTOM, _anchorProvider->findClosestAnchor(m_refFrame.verticalCenter(), IAnchorProvider::SearchDirection::DOWN));
		Widget::setAnchor(SIDE::TOP, _anchorProvider->findClosestAnchor(m_refFrame.verticalCenter(), IAnchorProvider::SearchDirection::UP));
	}

	ei::Vec2 NodeHandle::getConnectorDirection() const
	{
		return ei::Vec2(cos(m_angle), sin(m_angle));
	}

	void NodeHandle::setConnectorSnapRadius(Coord _radius)
	{
		NodeList::s_snapRadiusSq = _radius * _radius;
	}



	static void createBezierSpline(const Coord2& p0, const Coord2& p1, const Coord2& p2, const Coord2& p3, const const int num_points, Coord2* points, Coord2& bbmin, Coord2& bbmax)
	{
		bbmin = min(p0, p3);
		bbmax = max(p0, p3);
		for(int i = 0; i < num_points; ++i)
		{
			// TODO: adaptive step length to increase sample density in heigh
			// curvature areas
			float t = i / (num_points - 1.0f);
			float ti = 1.0f - t;
			points[i] = ti*ti*ti * p0 + 3.0f*ti*ti*t * p1 + 3.0f*ti*t*t * p2 + t*t*t * p3;
			bbmin = min(bbmin, points[i]);
			bbmax = max(bbmax, points[i]);
		}
	}

	NodeConnector::NodeConnector() :
		Clickable(this),
		m_stiffness(1.0f/3.0f),
		m_tmpHandleState(HandleState::ATTACHED),
		m_isMouseOver(false)
	{
		m_region = this;
		setAnchorable(false);

		// Make sure that the mouse node is always existing.
		if(!s_tmpMouseNode)
			s_tmpMouseNode = new NodeHandle(false);
	}

	const int CONNECTOR_NUM_POINTS = 32;
	void NodeConnector::draw() const
	{
		// Control points of a cubic Bezier spline:
		Vec2 p0, p1, p2, p3;
		// As support vectors take the direction given by the node handles
		// with a lenght of 1/3 from the distance between the two nodes.
		bool mouseFarAwayFromSrc = lensq(m_sourceNode->getRefFrame().center() - s_tmpMouseNode->getPosition()) > NodeList::s_snapRadiusSq;
		bool mouseFarAwayFromDst = lensq(m_destNode->getRefFrame().center() - s_tmpMouseNode->getPosition()) > NodeList::s_snapRadiusSq;
		if(m_tmpHandleState == HandleState::TMP_SRC && mouseFarAwayFromSrc)
			p0 = s_tmpMouseNode->getPosition();
		else {
			p0 = m_sourceNode->getRefFrame().center();
			p0 += m_sourceNode->getConnectorDirection() * m_sourceNode->getRefFrame().size() / 2.0f;
		}
		if(m_tmpHandleState == HandleState::TMP_DST && mouseFarAwayFromDst)
			p3 = s_tmpMouseNode->getPosition();
		else {
			p3 = m_destNode->getRefFrame().center();
			p3 += m_destNode->getConnectorDirection() * m_destNode->getRefFrame().size() / 2.0f;
		}
		float distance = len(p0 - p3) * m_stiffness;
		if((m_tmpHandleState == HandleState::TMP_SRC && mouseFarAwayFromSrc)
			|| (m_tmpHandleState == HandleState::TMP_DST && mouseFarAwayFromDst))
			distance /= 2.0f;
		// Reuse the one existing control point from the non-temp node for the
		// tmp connection direction. This way the temp node does not need an own
		// direction and the curve is visually reduced to a quadratic spline.
		if(m_tmpHandleState == HandleState::TMP_SRC && mouseFarAwayFromSrc)
			p1 = p3 + m_destNode->getConnectorDirection() * distance;
		else
			p1 = p0 + m_sourceNode->getConnectorDirection() * distance;
		if(m_tmpHandleState == HandleState::TMP_DST && mouseFarAwayFromDst)
			p2 = p0 + m_sourceNode->getConnectorDirection() * distance;
		else
			p2 = p3 + m_destNode->getConnectorDirection() * distance;

		Coord2 wayPoints[CONNECTOR_NUM_POINTS];
		Coord2 bbmin, bbmax;
		createBezierSpline(p0, p1, p2, p3, CONNECTOR_NUM_POINTS, wayPoints, bbmin, bbmax);
		bool mouseOver = isMouseOver(GUIManager::getMouseState().position);
		Vec4 sourceColor = ei::Vec4(m_sourceNode->color() * (mouseOver ? 2.0f : 1.0f), 1.0f);
		Vec4 destColor = ei::Vec4(m_destNode->color() * (mouseOver ? 2.0f : 1.0f), 1.0f);
		GUIManager::theme().drawLine(wayPoints, CONNECTOR_NUM_POINTS, sourceColor, destColor);

		NodeConnector* t = const_cast<NodeConnector*>(this);
		t->m_refFrame.sides[SIDE::LEFT] = bbmin.x - 3.0f;
		t->m_refFrame.sides[SIDE::BOTTOM] = bbmin.y - 3.0f;
		t->m_refFrame.sides[SIDE::RIGHT] = bbmax.x + 3.0f;
		t->m_refFrame.sides[SIDE::TOP] = bbmax.y + 3.0f;
	}

	bool NodeConnector::isMouseOver(const Coord2& _mousePos) const
	{
		m_isMouseOver = false;
		if(!m_refFrame.isMouseOver(_mousePos))
			return false;

		// Move along the spline and check if any segment is closer than 3.0 pixels
		// to the mouse position.
		Vec2 p0 = m_sourceNode->getRefFrame().center();
		p0 += m_sourceNode->getConnectorDirection() * m_sourceNode->getRefFrame().size() / 2.0f;
		Vec2 p3 = m_destNode->getRefFrame().center();
		p3 += m_destNode->getConnectorDirection() * m_destNode->getRefFrame().size() / 2.0f;
		float nodeDistance = len(p0 - p3) * m_stiffness;
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
			// Look if there is a close node.
			NodeHandle* handle = NodeList::findClosest(_mouseState.position);
			if(handle)
			{
				if(m_tmpHandleState == HandleState::TMP_SRC)
					m_sourceNode = handle;
				else if(m_tmpHandleState == HandleState::TMP_DST)
					m_destNode = handle;
			}
		}

		return m_tmpHandleState != HandleState::ATTACHED;
	}


	WidgetConnector::WidgetConnector() :
		Clickable(this),
		m_sourceColor(1.0f),
		m_destColor(1.0f),
		m_stiffness(1.0f/3.0f),
		m_isMouseOver(false)
	{
		m_region = this;
		setAnchorable(false);
	}

	void WidgetConnector::draw() const
	{
		GUIManager::theme().drawLine(m_curve.data(), m_curve.size(), Vec4(m_sourceColor, 1.0f), Vec4(m_destColor, 1.0f));
	}

	static void findWidgetBorder(const IRegion* _region, Vec2& _pos, const Vec2& _dir)
	{
		Vec2 tmp = _pos + _dir;
		bool isInside1 = _region->isMouseOver(tmp);
		for(int i = 0; i < 16; ++i)
		{
			if(isInside1)
			{
				// Linear ray march until point 1 is outside
				_pos = tmp;
				tmp += _dir;
				isInside1 = _region->isMouseOver(tmp);
			} else {
				// Binary search
				Vec2 center = (tmp + _pos) * 0.5f;
				bool isInsideC = _region->isMouseOver(center);
				if(isInsideC)
					_pos = center;
				else tmp = center;
			}
		}
	}

	void WidgetConnector::refitToAnchors()
	{
		if(!m_sourceNode || !m_destNode) return;

		Vec2 p0 = m_sourceNode->getRefFrame().center();
		Vec2 p3 = m_destNode->getRefFrame().center();
		Vec2 adir(cos(m_sourceAngle), sin(m_sourceAngle)); // TODO: scale with widget size
		Vec2 bdir(cos(m_destAngle), sin(m_destAngle));

		// Find the borders of the source/dest widget
		const IRegion* region = m_sourceNode->getRegion();
		findWidgetBorder(region, p0, adir * (sum(m_sourceNode->getSize()) / 4.0f));
		region = m_destNode->getRegion();
		findWidgetBorder(region, p3, bdir * (sum(m_destNode->getSize()) / 4.0f));

		// Create the Bezier spline
		float nodeDistance = len(p0 - p3) * m_stiffness;
		Vec2 p1 = p0 + adir * nodeDistance;
		Vec2 p2 = p3 + bdir * nodeDistance;
		m_curve.resize(CONNECTOR_NUM_POINTS);
		Coord2 bbmin, bbmax;
		createBezierSpline(p0, p1, p2, p3, CONNECTOR_NUM_POINTS, m_curve.data(), bbmin, bbmax);

		m_refFrame.sides[SIDE::LEFT] = bbmin.x - 3.0f;
		m_refFrame.sides[SIDE::BOTTOM] = bbmin.y - 3.0f;
		m_refFrame.sides[SIDE::RIGHT] = bbmax.x + 3.0f;
		m_refFrame.sides[SIDE::TOP] = bbmax.y + 3.0f;
	}

	bool WidgetConnector::isMouseOver(const Coord2 & _mousePos) const
	{
		// Early exit if not on the frame
		m_isMouseOver = false;
		if(!m_refFrame.isMouseOver(_mousePos))
			return false;

		for(uint i = 0; i < m_curve.size()-1; ++i)
		{
			Segment2D line;
			line.a = Vec2(m_curve[i]);
			line.b = Vec2(m_curve[i+1]);
			// Test if the _mousePos is close to the line segment l0-l1
			if(distanceSq(line, _mousePos) <= 9.0f) {
				m_isMouseOver = true;
				return true;
			}
		}
		return false;
	}

}} // namespace ca::gui
