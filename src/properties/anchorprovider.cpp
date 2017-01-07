#include "ca/gui/properties/anchorprovider.hpp"

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
		m_anchors[SIDE::LEFT]->position   = _selfFrame.left();
		m_anchors[SIDE::RIGHT]->position  = _selfFrame.right();
		m_anchors[SIDE::BOTTOM]->position = _selfFrame.bottom();
		m_anchors[SIDE::TOP]->position    = _selfFrame.top();
	}

	AnchorPtr BorderAnchorProvider::findClosestAnchor(float _position, SearchDirection _direction) const
	{
		if(_direction <= SearchDirection::RIGHT)
		{
			float dL = m_anchors[SIDE::LEFT]->position - _position;
			float dR = m_anchors[SIDE::RIGHT]->position - _position;
			if(_direction == SearchDirection::HORIZONTAL)
				return (abs(dL) < abs(dR)) ? m_anchors[SIDE::LEFT] : m_anchors[SIDE::RIGHT];
			eiAssert(dL <= dR, "Anchors are not ordered left to right!");
			if(_direction == SearchDirection::RIGHT)
				return (dL >= 0.0f) ? m_anchors[SIDE::LEFT] : (dR >= 0.0f ? m_anchors[SIDE::RIGHT] : nullptr);
			if(_direction == SearchDirection::LEFT)
				return (dR <= 0.0f) ? m_anchors[SIDE::RIGHT] : (dL <= 0.0f ? m_anchors[SIDE::LEFT] : nullptr);
		} else {
			float dB = m_anchors[SIDE::BOTTOM]->position - _position;
			float dT = m_anchors[SIDE::TOP]->position - _position;
			if(_direction == SearchDirection::VERTICAL)
				return (abs(dB) < abs(dT)) ? m_anchors[SIDE::BOTTOM] : m_anchors[SIDE::TOP];
			eiAssert(dB <= dT, "Anchors are not ordered bottom to top!");
			if(_direction == SearchDirection::UP)
				return (dB >= 0.0f) ? m_anchors[SIDE::BOTTOM] : (dT >= 0.0f ? m_anchors[SIDE::TOP] : nullptr);
			if(_direction == SearchDirection::DOWN)
				return (dT <= 0.0f) ? m_anchors[SIDE::TOP] : (dB <= 0.0f ? m_anchors[SIDE::BOTTOM] : nullptr);
		}
		return nullptr;
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
		m_someChanged = true;

		if( !m_hAnchors.empty() )
		{
			if( m_hAnchors.size() == 1)
			{
				m_hAnchors[0]->position = (_selfFrame.left() + _selfFrame.right()) * 0.5f;
			} else {
				float hgap = _selfFrame.width() / (m_hAnchors.size() - 1);
				for(size_t i = 0; i < m_hAnchors.size(); ++i)
					m_hAnchors[i]->position = _selfFrame.left() + i * hgap;
			}
		}

		if( !m_vAnchors.empty() )
		{
			if( m_vAnchors.size() == 1)
			{
				m_vAnchors[0]->position = (_selfFrame.bottom() + _selfFrame.top()) * 0.5f;
			} else {
				float vgap = _selfFrame.height() / (m_vAnchors.size() - 1);
				for(size_t i = 0; i < m_vAnchors.size(); ++i)
					m_vAnchors[i]->position = _selfFrame.bottom() + i * vgap;
			}
		}
	}

	AnchorPtr GridAnchorProvider::findClosestAnchor(float _position, SearchDirection _direction) const
	{
		// Linear search (number of anchors is assumed to be small)
		AnchorPtr minAnchor;
		float minDist = 1e10f;
		if(_direction <= SearchDirection::RIGHT)
		{
			if(m_hAnchors.empty()) return AnchorPtr(nullptr);
			for(size_t i = 0; i < m_hAnchors.size(); ++i)
			{
				float d = m_hAnchors[i]->position - _position;
				if((_direction == SearchDirection::HORIZONTAL && abs(d) < minDist)
					|| (_direction == SearchDirection::LEFT && d <= 0.0f && abs(d) < minDist)
					|| (_direction == SearchDirection::RIGHT && d >= 0.0f && d < minDist))
				{
					minAnchor = m_hAnchors[i];
					minDist = abs(d);
				}
			}
		} else {
			for(size_t i = 0; i < m_vAnchors.size(); ++i)
			{
				if(m_vAnchors.empty()) return AnchorPtr(nullptr);
				float d = m_vAnchors[i]->position - _position;
				if((_direction == SearchDirection::VERTICAL && abs(d) < minDist)
					|| (_direction == SearchDirection::DOWN && d <= 0.0f && abs(d) < minDist)
					|| (_direction == SearchDirection::UP && d >= 0.0f && d < minDist))
				{
					minAnchor = m_vAnchors[i];
					minDist = abs(d);
				}
			}
		}
		return move(minAnchor);
	}

}} // namespace ca::gui