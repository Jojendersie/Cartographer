#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	typedef pa::RefPtr<class NodeConnector> NodeConnectorPtr;
	class NodeHandle;


	/// Interface to react to node connection changes
	class IConnectorController : public pa::ReferenceCountable
	{
	public:
		virtual ~IConnectorController() = default;

		/// Called if a connector is connected to a node handle to react
		// or reject to the connection.
		/// \param [in] _hdlThis The node handle on the controller sided end of the connector.
		/// \param [in] _hdlOther The node handle on the other end of the connector.
		/// \return false if the connection should not be accepted.
		virtual bool onConnect(NodeHandle* _hdlThis, NodeHandle* _hdlOther) = 0;

		/// Called if a connector is broken up.
		/// \param [in] _hdl The node handle on the other end of the connector (that gets disconnected).
		virtual void onDisconnect(NodeHandle* _hdlThis, NodeHandle* _hdlOther) = 0;
	};

	typedef pa::RefPtr<class IConnectorController> ConnectorControllerPtr;


	/// A small handle for node based editors which can connect to other nodes
	/// usin spline connectors.
	class NodeHandle : public Widget, public Clickable
	{
		/// Internal default constructor. May register the handle in the global
		/// list for node searches.
		NodeHandle(bool _register);
		friend class NodeConnector;
	public:
		NodeHandle() : NodeHandle(true) {}
		~NodeHandle();

		/// Implement the draw method
		void draw() const override;

		/// Create new connectors on button down
		bool processInput(const struct MouseState& _mouseState) override;

		void onExtentChanged() override;

		/// The rotation determines the starting direction of all edges.
		/// \param [in] _angle Angle in radiant where 0 is showing right.
		void setRotation(float _angle) { m_angle = _angle; }

		/// Each node can have an individual color to distinguish classes/compatibilities.
		void setColor(const ei::Vec3& _color) { m_color = _color; }
		const ei::Vec3& color() const { return m_color; }

		/// Get a direction vector in which connectors should start
		ei::Vec2 getConnectorDirection() const;

		/// Set the radius in which a connector can snap to the node.
		static void setConnectorSnapRadius(Coord _radius);

		/// Sets the controller to react on connection changes.
		/// Note that it is sufficient to set a controller on one side of each
		/// desired connection. Otherwise there will be two calls for each of the
		/// ends if both involved nodes have a controller.
		/// \param [in] _controller A controller instance that will be owned by the
		///		node handle.
		void setConnectorController(ConnectorControllerPtr _controller);
		const IConnectorController* getController() const { return m_controller.get(); }

		/// Detach all connections
		void disconnectAll();

	private:
		std::vector<NodeConnectorPtr> m_edges;
		ConnectorControllerPtr m_controller;
		ei::Vec3 m_color;
		float m_angle;

		// Remove a connector from the edge list on this side!
		// This will not change the node on the other end of the connection.
		void removeEdge(NodeConnector* e);
		bool addEdge(NodeConnectorPtr e, NodeHandle* other);
	};

	typedef pa::RefPtr<NodeHandle> NodeHandlePtr;


	/// Spline connector as edges of the node graph.
	class NodeConnector : public Widget, public Clickable
	{
		friend class NodeHandle;
	public:
		NodeConnector();
		~NodeConnector();

		/// Implement the draw method
		void draw() const override;

		/// Tear off from a node if clicked an snap (back) if released.
		bool processInput(const MouseState& _mouseState) override;

		void onExtentChanged() override;

		/// Attach one end of the connection to a node handle or detach via (nullptr).
		void setSource(NodeHandlePtr _node);
		void setDest(NodeHandlePtr _node);

		/// Bring into a state where one of the two points follows the mouse
		void setMouseAttached(const bool _atSrc);

		virtual const IRegion* getRegion() const override { return this; }

		/// Set the bending strength at the start points.
		/// The stiffness effectivly determines the relative distance of the
		/// Bezier-control points. The default is 1.0/3.0
		void setStiffness(float _stiffness) { m_stiffness = _stiffness; }

		/// Regardless of source and dest get the other one compared to
		/// the current one.
		const NodeHandle* getOther(const NodeHandle* _this) const;
		NodeHandle* getOther(const NodeHandle* _this);

	protected:
		NodeHandlePtr m_sourceNode;
		NodeHandlePtr m_destNode;
		float m_stiffness;
		std::vector<ei::Vec2> m_curve;

		enum class HandleState
		{
			ATTACHED,					///< Use source and dest node (ignore tmp), even if one of the nodes does not exist.
			TMP_SRC,					///< Replace src optically by tmp
			TMP_DST						///< Replace dest optically by tmp
		};
		HandleState m_tmpHandleState;

		/// Implementation of IRegion::isMouseOver.
		bool isMouseOver(const Coord2& _mousePos) const override;

		void removeThis();

		mutable bool m_isMouseOver;		///< Store the result of the last test (because it is expensive)
		mutable float m_mouseT;			///< Store curve parameter 't' of last isMouseOver test. This is undefined if m_isMouseOver is false.
	};

	/// Spline connector which can connect any Widget.
	/// In contrast to the NodeConnector it does not allow interaction in form of snapping
	/// and reassignment.
	class WidgetConnector : public Widget, public Clickable
	{
	public:
		WidgetConnector();

		/// Implement the draw method.
		/// The curve will visually start at the border of the widged region.
		void draw() const override;

		/// \param [in] _angle Direction in radiant in which the edge will start.
		///		0 is on the right side.
		void setSource(ConstWidgetPtr _node, float _angle);
		void setDest(ConstWidgetPtr _node, float _angle);

		float getSourceAngle() const { return m_sourceAngle; }
		void setSourceAngle(float _angle) { m_changed = m_sourceAngle != _angle;  m_sourceAngle = _angle; }
		float getDestAngle() const { return m_destAngle; }
		void setDestAngle(float _angle) { m_changed = m_destAngle != _angle; m_destAngle = _angle; }

		/// Sets the color for the entire edge.
		/// It is also possible to use an gradient (setSourceColor, setDestColor).
		void setColor(const ei::Vec3& _color) { m_sourceColor = m_destColor = _color; }
		const ei::Vec3& getColor() const { return m_sourceColor; }
		void setSourceColor(const ei::Vec3& _color) { m_sourceColor = _color; }
		const ei::Vec3& getSourceColor() const { return m_sourceColor; }
		void setDestColor(const ei::Vec3& _color) { m_destColor = _color; }
		const ei::Vec3& getDestColor() const { return m_destColor; }

		/// Set the bending strength at the start points.
		/// The stiffness effectivly determines the relative distance of the
		/// Bezier-control points. The default is 1.0/3.0
		void setStiffness(float _stiffness) { m_stiffness = _stiffness; }

		virtual const IRegion* getRegion() const override { return this; }
	protected:
		ConstWidgetPtr m_sourceNode;
		ConstWidgetPtr m_destNode;
		ei::Vec3 m_sourceColor;
		ei::Vec3 m_destColor;
		float m_sourceAngle;
		float m_destAngle;
		float m_stiffness;

		mutable std::vector<ei::Vec2> m_curve;
		mutable bool m_changed;

		void recomputeSizeAndAnchors();
		void recomputeCurve() const;

		/// Implementation of IRegion::isMouseOver.
		bool isMouseOver(const Coord2& _mousePos) const override;

		void onExtentChanged() override;

		mutable bool m_isMouseOver;		///< Store the result of the last test
	//	mutable float m_mouseT;			///< Store curve parameter 't' of last isMouseOver test. This is undefined if m_isMouseOver is false.
	};

	typedef pa::RefPtr<WidgetConnector> WidgetConnectorPtr;

}} // namespace ca::gui
