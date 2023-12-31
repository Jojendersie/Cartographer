#include "ca/gui/widgets/node.hpp"
#include "ca/gui/widgets/group.hpp"
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
				float distSq = lensq(s_nodes[i]->position() - _position);
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
		Widget::setRegion(std::make_unique<EllipseRegion>(this));

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
			center(),
			0.5f * width(),
			mouseOver ? 2.0f * m_color : m_color);
		//GUIManager::theme().drawCheckbox(m_refFrame, true, true);
	}

	bool NodeHandle::processInput(const struct MouseState& _mouseState)
	{
		if(_mouseState.btnDown(0))
		{
			NodeConnectorPtr con ( new NodeConnector );
			con->setSource(NodeHandlePtr{this});
			con->setMouseAttached(false);
			// The parent must be a group -> add new connector as sibling.
			Group* p = dynamic_cast<Group*>(parent());
			if(p)
				p->add(con);
		}
		return true;
	}

	void NodeHandle::onExtentChanged()
	{
		for(auto& e : m_edges)
			e->onExtentChanged();
	}

	ei::Vec2 NodeHandle::getConnectorDirection() const
	{
		return ei::Vec2(cos(m_angle), sin(m_angle));
	}

	void NodeHandle::setConnectorSnapRadius(Coord _radius)
	{
		NodeList::s_snapRadiusSq = _radius * _radius;
	}

	void NodeHandle::setConnectorController(ConnectorControllerPtr _controller)
	{
		m_controller = _controller;
	}

	/// Detach all connections
	void NodeHandle::disconnectAll()
	{
		while(!m_edges.empty())
			m_edges.back()->removeThis();
	}

	void NodeHandle::removeEdge(NodeConnector* e)
	{
		for(auto it = m_edges.begin(); it != m_edges.end(); ++it)
		if(it->get() == e)
		{
			NodeHandle* other = e->getOther(this);
			if(other && m_controller)
				m_controller->onDisconnect(this, other);
			m_edges.erase(it);
			return;
		}
	}

	bool NodeHandle::addEdge(NodeConnectorPtr e, NodeHandle* other)
	{
		bool valid = true;
		if(other)
		{
			if(m_controller)
				valid &= m_controller->onConnect(this, other);
			if(other->m_controller)
				valid &= other->m_controller->onConnect(other, this);
		}
		if(valid)
			m_edges.push_back(e);
		return valid;
	}



	static void createBezierSpline(const Coord2& p0, const Coord2& p1, const Coord2& p2, const Coord2& p3, const int num_points, Coord2* points, Coord2& bbmin, Coord2& bbmax)
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

	const int CONNECTOR_NUM_POINTS = 48; // TODO: replace with distance (src <-> dst) dependent term
	NodeConnector::NodeConnector() :
		Clickable(this),
		m_stiffness(1.0f/3.0f),
		m_tmpHandleState(HandleState::ATTACHED)
	{
		m_region = this;
		setAnchorable(false);

		// Make sure that the mouse node is always existing.
		if(!s_tmpMouseNode)
			s_tmpMouseNode = new NodeHandle(false);
		m_curve.resize(CONNECTOR_NUM_POINTS);
	}

	NodeConnector::~NodeConnector()
	{
	}

	void NodeConnector::draw() const
	{
		bool mouseOver = GUIManager::hasMouseFocus(this)
			&& isMouseOver(GUIManager::getMouseState().position);
		Vec3 sourceColor = m_sourceNode ? m_sourceNode->color() : Vec3{0.5f};
		Vec3 destColor = m_destNode ? m_destNode->color() : Vec3{0.5f};
		if(mouseOver)
		{
			sourceColor *= 2.0f;
			destColor *= 2.0f;
		}
		GUIManager::theme().drawLine(m_curve.data(), CONNECTOR_NUM_POINTS, Vec4{sourceColor,1.0f}, Vec4{destColor,1.0f});
	}

	bool NodeConnector::isMouseOver(const Coord2& _mousePos) const
	{
		if(!RefFrame::isMouseOver(_mousePos))
			return false;

		// Move along the spline and check if any segment is closer than 3.0 pixels
		// to the mouse position.
		Segment2D line;
		line.a = m_curve[0];
		for(int i = 1; i < CONNECTOR_NUM_POINTS; ++i)
		{
			line.b = m_curve[i];
			// Test if the _mousePos is close to the line segment l0-l1
			if(distanceSq(line, _mousePos) <= 9.0f) {
				m_mouseT = i / (CONNECTOR_NUM_POINTS - 1.0f);
				return true;
			}
			line.a = line.b;
		}
		return false;
	}

	void NodeConnector::removeThis()
	{
		// Removing from all others can cause a deletion half way through this method.
		WidgetPtr thisPinned{this};
		Group* p = dynamic_cast<Group*>(parent());
		if(p)
			p->remove(thisPinned);
		if(m_sourceNode)
			m_sourceNode->removeEdge(this);
		if(m_destNode)
			m_destNode->removeEdge(this);
		m_sourceNode = nullptr;
		m_destNode = nullptr;
	}

	bool NodeConnector::processInput(const MouseState& _mouseState)
	{
		const HandleState oldState = m_tmpHandleState;

		if(_mouseState.btnDown(0))
		{
			// Tear off now
			if(m_mouseT < 0.5)
				m_tmpHandleState = HandleState::TMP_SRC;
			else
				m_tmpHandleState = HandleState::TMP_DST;
		}

		if(_mouseState.btnReleased(0))
		{
			m_tmpHandleState = HandleState::ATTACHED;
			if(!m_sourceNode || !m_destNode)
			{
				removeThis();
				return true;
			}
		}

		if(m_tmpHandleState != HandleState::ATTACHED)
		{
			s_tmpMouseNode->setPosition(_mouseState.position);
			// Look if there is a close node.
			NodeHandle* handle = NodeList::findClosest(_mouseState.position);
			if(m_tmpHandleState == HandleState::TMP_SRC)
				setSource(NodeHandlePtr{handle});
			else if(m_tmpHandleState == HandleState::TMP_DST)
				setDest(NodeHandlePtr{handle});
		}

		if((oldState != m_tmpHandleState) || ((m_tmpHandleState != HandleState::ATTACHED) && (_mouseState.deltaPos() != Vec2{0.0f})))
			onExtentChanged();

		return true;
	}


	void NodeConnector::onExtentChanged()
	{
		const Vec2 srcPos = m_sourceNode ? m_sourceNode->center() : s_tmpMouseNode->position();
		const Vec2 dstPos = m_destNode ? m_destNode->center() : s_tmpMouseNode->position();
		const Vec2 srcDir = m_sourceNode ? m_sourceNode->getConnectorDirection() : Vec2{0.0f};
		const Vec2 dstDir = m_destNode ? m_destNode->getConnectorDirection() : Vec2{0.0f};
		// Control points of a cubic Bezier spline:
		Vec2 p0, p1, p2, p3;
		// As support vectors take the direction given by the node handles
		// with a lenght of 1/3 from the distance between the two nodes.
		bool mouseFarAwayFromSrc = m_sourceNode
			? (lensq(srcPos - s_tmpMouseNode->position()) > NodeList::s_snapRadiusSq)
			: true;
		bool mouseFarAwayFromDst = m_destNode
			? (lensq(dstPos - s_tmpMouseNode->position()) > NodeList::s_snapRadiusSq)
			: true;
		const bool srcUseMouse = !m_sourceNode || (m_tmpHandleState == HandleState::TMP_SRC && mouseFarAwayFromSrc);
		const bool dstUseMouse = !m_destNode || (m_tmpHandleState == HandleState::TMP_DST && mouseFarAwayFromDst);
		if(srcUseMouse)
			p0 = s_tmpMouseNode->position();
		else {
			p0 = srcPos;
			p0 += srcDir * m_sourceNode->size() / 2.0f;
		}
		if(dstUseMouse)
			p3 = s_tmpMouseNode->position();
		else {
			p3 = dstPos;
			p3 += dstDir * m_destNode->size() / 2.0f;
		}
		float distance = len(p0 - p3) * m_stiffness;
		if(srcUseMouse || dstUseMouse)
			distance /= 2.0f;
		// Reuse the one existing control point from the non-temp node for the
		// tmp connection direction. This way the temp node does not need an own
		// direction and the curve is visually reduced to a quadratic spline.
		if(srcUseMouse)
			p1 = p3 + dstDir * distance;
		else
			p1 = p0 + srcDir * distance;
		if(dstUseMouse)
			p2 = p0 + srcDir * distance;
		else
			p2 = p3 + dstDir * distance;

		m_curve.resize(CONNECTOR_NUM_POINTS);
		Coord2 bbmin, bbmax;
		createBezierSpline(p0, p1, p2, p3, CONNECTOR_NUM_POINTS, m_curve.data(), bbmin, bbmax);
		silentSetFrame(bbmin.x - 3.0f, bbmin.y - 3.0f, bbmax.x + 3.0f, bbmax.y + 3.0f);

		Widget::onExtentChanged();
	}


	void NodeConnector::setSource(NodeHandlePtr _node)
	{
		if(m_sourceNode != _node && m_destNode != _node)
		{
			if(m_sourceNode)
			{
				m_sourceNode->removeEdge(this);
				if(m_destNode && m_destNode->m_controller)
					m_destNode->m_controller->onDisconnect(m_destNode.get(), m_sourceNode.get());
			}
			if(_node)
			{
				if(_node->addEdge(NodeConnectorPtr{this}, m_destNode.get()))
					m_sourceNode = _node;
			}
			else m_sourceNode = nullptr;
			onExtentChanged();
		}
	}
	
	void NodeConnector::setDest(NodeHandlePtr _node)
	{
		if(m_destNode != _node && m_sourceNode != _node)
		{
			if(m_destNode)
			{
				m_destNode->removeEdge(this);
				if(m_sourceNode && m_sourceNode->m_controller)
					m_sourceNode->m_controller->onDisconnect(m_sourceNode.get(), m_destNode.get());
			}
			if(_node)
			{
				if(_node->addEdge(NodeConnectorPtr{this}, m_sourceNode.get()))
					m_destNode = _node;
			}
			else m_destNode = nullptr;
			onExtentChanged();
		}
	}


	void NodeConnector::setMouseAttached(const bool _atSrc)
	{
		HandleState newState = _atSrc ? HandleState::TMP_SRC : HandleState::TMP_DST;
		if(newState != m_tmpHandleState)
		{
			m_tmpHandleState = newState;
			eiAssert(m_tmpHandleState == HandleState::TMP_SRC || m_tmpHandleState == HandleState::TMP_DST, "One of the two ends must be selected.");
			s_tmpMouseNode->setPosition(GUIManager::getMouseState().position);
			onExtentChanged();
			// Get exclusive focus
			GUIManager::setMouseFocus(this, true);
		}
	}

	const NodeHandle* NodeConnector::getOther(const NodeHandle* _this) const
	{
		if(m_sourceNode.get() == _this) return m_destNode.get();
		return m_sourceNode.get();
	}

	NodeHandle* NodeConnector::getOther(const NodeHandle* _this)
	{
		if(m_sourceNode.get() == _this) return m_destNode.get();
		return m_sourceNode.get();
	}





	WidgetConnector::WidgetConnector() :
		Clickable(this),
		m_sourceColor(1.0f),
		m_destColor(1.0f),
		m_sourceAngle(0.0f),
		m_destAngle(0.0f),
		m_stiffness(1.0f/3.0f),
		m_changed(false)
	{
		m_region = this;
	}

	void WidgetConnector::draw() const
	{
		if(m_changed)
			recomputeCurve();
		GUIManager::theme().drawLine(m_curve.data(), (int)m_curve.size(), Vec4(m_sourceColor, 1.0f), Vec4(m_destColor, 1.0f));
	}


	void WidgetConnector::setSource(ConstWidgetPtr _node, float _angle)
	{
		m_sourceNode = _node;
		m_sourceAngle = _angle;
		recomputeSizeAndAnchors();
	}
	
	
	void WidgetConnector::setDest(ConstWidgetPtr _node, float _angle)
	{
		m_destNode = _node;
		m_destAngle = _angle;
		recomputeSizeAndAnchors();
	}


	void WidgetConnector::recomputeSizeAndAnchors()
	{
		if(m_destNode && m_sourceNode)
		{
			const Vec2 c0 = m_destNode->center();
			const Vec2 c1 = m_sourceNode->center();
			// We don't really care where we are anchored, we only need the resize events for recomputations.
			setAnchors(m_destNode.get(),
				(c0.x <= c1.x ? SIDE_FLAGS::LEFT : SIDE_FLAGS::RIGHT)
				| (c0.y <= c1.y ? SIDE_FLAGS::BOTTOM : SIDE_FLAGS::TOP)
			);
			setAnchors(m_sourceNode.get(),
				(c0.x < c1.x ? SIDE_FLAGS::RIGHT : SIDE_FLAGS::LEFT)
				| (c0.y < c1.y ? SIDE_FLAGS::TOP : SIDE_FLAGS::BOTTOM)
			);
			setFrame(ei::min(c0.x, c1.x), ei::min(c0.y, c1.y), ei::max(c0.x, c1.x), ei::max(c0.y, c1.y));
		}
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

	void WidgetConnector::recomputeCurve() const
	{
		Vec2 p0 = m_sourceNode->center();
		Vec2 p3 = m_destNode->center();
		Vec2 adir(cos(m_sourceAngle), sin(m_sourceAngle)); // TODO: scale with widget size
		Vec2 bdir(cos(m_destAngle), sin(m_destAngle));

		// Find the borders of the source/dest widget
		const IRegion* region = m_sourceNode->getRegion();
		findWidgetBorder(region, p0, adir * (sum(m_sourceNode->size()) / 4.0f));
		region = m_destNode->getRegion();
		findWidgetBorder(region, p3, bdir * (sum(m_destNode->size()) / 4.0f));

		// Create the Bezier spline
		float nodeDistance = len(p0 - p3) * m_stiffness;
		Vec2 p1 = p0 + adir * nodeDistance;
		Vec2 p2 = p3 + bdir * nodeDistance;
		m_curve.resize(CONNECTOR_NUM_POINTS);
		Coord2 bbmin, bbmax;
		createBezierSpline(p0, p1, p2, p3, CONNECTOR_NUM_POINTS, m_curve.data(), bbmin, bbmax);
		m_changed = false;

		const_cast<WidgetConnector*>(this)->silentSetFrame(bbmin.x - 3.0f, bbmin.y - 3.0f, bbmax.x + 3.0f, bbmax.y + 3.0f);
	}


	bool WidgetConnector::isMouseOver(const Coord2 & _mousePos) const
	{
		// Early exit if not on the frame
		if(!RefFrame::isMouseOver(_mousePos))
			return false;

		for(uint i = 0; i < m_curve.size()-1; ++i)
		{
			Segment2D line;
			line.a = Vec2(m_curve[i]);
			line.b = Vec2(m_curve[i+1]);
			// Test if the _mousePos is close to the line segment l0-l1
			if(distanceSq(line, _mousePos) <= 9.0f) {
				return true;
			}
		}
		return false;
	}



	void WidgetConnector::onExtentChanged()
	{
		Widget::onExtentChanged();
		if(!m_sourceNode || !m_destNode) return;
		m_changed = true;
	}

}} // namespace ca::gui
