#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	typedef pa::RefPtr<class NodeConnector> NodeConnectorPtr;

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

		/// The rotation determines the starting direction of all edges.
		/// \param [in] _angle Angle in radiant where 0 is showing right.
		void setRotation(float _angle) { m_angle = _angle; }

		/// Each node can have an individual color to distinguish classes/compatibilities.
		void setColor(const ei::Vec3& _color) { m_color = _color; }
		const ei::Vec3& color() const { return m_color; }

		/// Get a direction vector in wich connectors should start
		ei::Vec2 getConnectorDirection() const;

		/// Set the radius in which a connector can snap to the node.
		static void setConnectorSnapRadius(Coord _radius);

	private:
		std::vector<NodeConnectorPtr> m_edges;
		ei::Vec3 m_color;
		float m_angle;
	};

	typedef pa::RefPtr<NodeHandle> NodeHandlePtr;

	/// Spline connector as edges of the node graph.
	class NodeConnector : public Widget, public Clickable
	{
	public:
		NodeConnector();

		/// Implement the draw method
		void draw() const override;

		/// Tear off from a node if clicked an snap (back) if released.
		bool processInput(const MouseState& _mouseState) override;

		void setSource(NodeHandlePtr _node)		{m_sourceNode = _node;}
		void setDest(NodeHandlePtr _node)		{m_destNode = _node;}

		virtual const IRegion* getRegion() const override { return this; }

		/// Set the bending strength at the start points.
		/// The stiffness effectivly determines the relative distance of the
		/// Bezier-control points. The default is 1.0/3.0
		void setStiffness(float _stiffness) { m_stiffness = _stiffness; }

	protected:
		NodeHandlePtr m_sourceNode;
		NodeHandlePtr m_destNode;
		float m_stiffness;

		enum class HandleState
		{
			ATTACHED,					///< Use source and dest node (ignore tmp), even if one of the nodes does not exist.
			TMP_SRC,					///< Replace src optically by tmp
			TMP_DST						///< Replace dest optically by tmp
		};
		HandleState m_tmpHandleState;

		/// Implementation of IRegion::isMouseOver.
		bool isMouseOver(const Coord2& _mousePos) const override;

		mutable bool m_isMouseOver;		///< Store the result of the last test
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

		// TODO: call refitToAnchors after each set?
		float getSourceAngle() const { return m_sourceAngle; }
		void setSourceAngle(float _angle) { m_sourceAngle = _angle; }
		float getDestAngle() const { return m_destAngle; }
		void setDestAngle(float _angle) { m_destAngle = _angle; }

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

		void onExtentChanged(const CHANGE_FLAGS::Val _changes) override;

		mutable bool m_isMouseOver;		///< Store the result of the last test
	//	mutable float m_mouseT;			///< Store curve parameter 't' of last isMouseOver test. This is undefined if m_isMouseOver is false.
	};

	typedef pa::RefPtr<WidgetConnector> WidgetConnectorPtr;

}} // namespace ca::gui
