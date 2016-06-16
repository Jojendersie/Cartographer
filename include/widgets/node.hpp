#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	typedef std::shared_ptr<class NodeConnector> NodeConnectorPtr;

	/// A small handle for node based editors which can connect to other nodes
	/// usin spline connectors.
	class NodeHandle : public Widget
	{
	public:
		NodeHandle();

		/// Implement the draw method
		void draw() override;

		/// The rotation determines the starting direction of all edges.
		/// \param [in] _angle Angle in radiant where 0 is showing right.
		void setRotation(float _angle) { m_angle = _angle; }

		/// Each node can have an individual color to distinguish classes/compatibilities.
		void setColor(const ei::Vec3& _color) { m_color = _color; }

		/// Attach entire element (moveable) to an anchor.
		//void setAnchoring(AnchorPtr _anchor);
		/// Modified automated anchoring, which attaches only two of four anchors
		void autoAnchor(const class IAnchorProvider* _anchorProvider);

		/// Get a direction vector in wich connectors should start
		ei::Vec2 getConnectorDirection() const;
	private:
		std::vector<NodeConnectorPtr> m_edges;
		ei::Vec3 m_color;
		float m_angle;

		// Delete some of the functions which do not make sense for point-like
		// handles.
		void setAnchoring(SIDE::Val _side, AnchorPtr _anchor) = delete;
		void setHorizontalAnchorMode(Anchorable::Mode _mode) = delete;
		void setVerticalAnchorMode(Anchorable::Mode _mode) = delete;
		void setAnchorModes(Anchorable::Mode _mode) = delete;
		void setAnchorModes(Anchorable::Mode _horizontalMode, Anchorable::Mode _verticalMode) = delete;
		void setAnchorProvider(std::unique_ptr<IAnchorProvider> _anchorProvider) = delete;
		IAnchorProvider* getAnchorProvider() const = delete;
	};

	typedef std::shared_ptr<NodeHandle> NodeHandlePtr;

	/// Spline connector as edges of the node graph.
	class NodeConnector : public Widget
	{
	public:
		NodeConnector();

		/// Implement the draw method
		void draw() override;

		/// Recompute the reference frame dependent on the two nodes
		void refitToAnchors() override;

		void setSource(NodeHandlePtr _node)		{m_sourceNode = _node;}
		void setDest(NodeHandlePtr _node)		{m_destNode = _node;}
	protected:
		NodeHandlePtr m_sourceNode;
		NodeHandlePtr m_destNode;
	};

	typedef std::shared_ptr<NodeConnector> NodeConnectorPtr;

}} // namespace ca::gui
