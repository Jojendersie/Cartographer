#include "properties/anchorprovider.hpp"

namespace ca { namespace gui {

	bool IAnchorProvider::m_someChanged = false;

	BorderAnchorProvider::BorderAnchorProvider()
	{
		for(int i = 0; i < 4; ++i)
			m_anchors[i] = std::make_shared<AnchorPoint>(this);
	}

	BorderAnchorProvider::~BorderAnchorProvider()
	{
		// Cause a future deletion of the four anchors
		for(int i = 0; i < 4; ++i)
			m_anchors[i]->host = nullptr;
	}

	void BorderAnchorProvider::replaceAnchors(const RefFrame& _selfFrame)
	{
		m_someChanged = true;
		m_anchors[SIDE::LEFT]->position   = Coord2(_selfFrame.left(),  (_selfFrame.bottom() + _selfFrame.top()) * 0.5f);
		m_anchors[SIDE::RIGHT]->position  = Coord2(_selfFrame.right(), (_selfFrame.bottom() + _selfFrame.top()) * 0.5f);
		m_anchors[SIDE::BOTTOM]->position = Coord2((_selfFrame.left() + _selfFrame.right()) * 0.5f, _selfFrame.bottom());
		m_anchors[SIDE::TOP]->position    = Coord2((_selfFrame.left() + _selfFrame.right()) * 0.5f, _selfFrame.top());
	}

}} // namespace ca::gui