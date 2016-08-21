#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	typedef pa::RefPtr<class NodeConnector> NodeConnectorPtr;

	/// A small handle for node based editors which can connect to other nodes
	/// usin spline connectors.
	class NodeHandle : public Widget
	{
	public:
		NodeHandle();

		/// Implement the draw method
		void draw() const override;

		/// The rotation determines the starting direction of all edges.
		/// \param [in] _angle Angle in radiant where 0 is showing right.
		void setRotation(float _angle) { m_angle = _angle; }

		/// Each node can have an individual color to distinguish classes/compatibilities.
		void setColor(const ei::Vec3& _color) { m_color = _color; }
		const ei::Vec3& color() const { return m_color; }

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

	typedef pa::RefPtr<NodeHandle> NodeHandlePtr;

	/// Spline connector as edges of the node graph.
	class NodeConnector : public Widget, public IRegion
	{
	public:
		NodeConnector();

		/// Implement the draw method
		void draw() const override;

		/// Recompute the reference frame dependent on the two nodes
		void refitToAnchors() override;

		/// Tear off from a node if clicked an snap (back) if released.
		bool processInput(const MouseState& _mouseState) override;

		void setSource(NodeHandlePtr _node)		{m_sourceNode = _node;}
		void setDest(NodeHandlePtr _node)		{m_destNode = _node;}
	protected:
		NodeHandlePtr m_sourceNode;
		NodeHandlePtr m_destNode;

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

}} // namespace ca::gui
