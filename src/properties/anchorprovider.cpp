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

	void BorderAnchorProvider::recomputeAnchors(const RefFrame& _selfFrame)
	{
		m_someChanged = true;
		m_anchors[SIDE::LEFT]->position   = Coord2(_selfFrame.left(),  (_selfFrame.bottom() + _selfFrame.top()) * 0.5f);
		m_anchors[SIDE::RIGHT]->position  = Coord2(_selfFrame.right(), (_selfFrame.bottom() + _selfFrame.top()) * 0.5f);
		m_anchors[SIDE::BOTTOM]->position = Coord2((_selfFrame.left() + _selfFrame.right()) * 0.5f, _selfFrame.bottom());
		m_anchors[SIDE::TOP]->position    = Coord2((_selfFrame.left() + _selfFrame.right()) * 0.5f, _selfFrame.top());
	}



	GridAnchorProvider::GridAnchorProvider(int _gridDimX, int _gridDimY)
	{
		for(int i = 0; i < _gridDimX; ++i)
			m_hAnchors.push_back(move(std::make_shared<AnchorPoint>(this)));
		for(int i = 0; i < _gridDimY; ++i)
			m_vAnchors.push_back(move(std::make_shared<AnchorPoint>(this)));
	}
	
	GridAnchorProvider::~GridAnchorProvider()
	{
		// Cause a future deletion of the anchors
		for(auto& it : m_hAnchors)
			it->host = nullptr;
		for(auto& it : m_vAnchors)
			it->host = nullptr;
	}

	void GridAnchorProvider::recomputeAnchors(const class RefFrame& _selfFrame)
	{
		ei::Vec2 center( (_selfFrame.left() + _selfFrame.right()) * 0.5f, (_selfFrame.bottom() + _selfFrame.top()) * 0.5f );
		m_someChanged = true;

		if( !m_hAnchors.empty() )
		{
			if( m_hAnchors.size() == 1)
			{
				m_hAnchors[0]->position = center;
			} else {
				float hgap = _selfFrame.width() / (m_hAnchors.size() - 1);
				for(size_t i = 0; i < m_hAnchors.size(); ++i)
					m_hAnchors[i]->position = ei::Vec2(_selfFrame.left() + i * hgap, center.y);
			}
		}

		if( !m_vAnchors.empty() )
		{
			if( m_vAnchors.size() == 1)
			{
				m_vAnchors[0]->position = center;
			} else {
				float vgap = _selfFrame.height() / (m_vAnchors.size() - 1);
				for(size_t i = 0; i < m_vAnchors.size(); ++i)
					m_vAnchors[i]->position = ei::Vec2(center.x, _selfFrame.bottom() + i * vgap);
			}
		}
	}

}} // namespace ca::gui