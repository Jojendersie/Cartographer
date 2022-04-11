#include <ca/pa/log.hpp>
#include "ca/gui/properties/anchorable.hpp"

namespace ca { namespace gui {

	Anchor::~Anchor()
	{
		// Remove this element from the double linked list
		if(m_prev) m_prev->m_next = m_next;
		if(m_next) m_next->m_prev = m_prev;
	}

	void Anchor::detach()
	{
		// Remove this element from the double linked list
		if(m_prev) m_prev->m_next = m_next;
		if(m_next) m_next->m_prev = m_prev;
		m_reference = nullptr;
		m_next = nullptr;
	}

	void Anchor::attach(const IAnchorProvider* _target, Coord _targetPosition, Coord _anchorPosition, int _dimension)
	{
		relativePosition = _target->getRelativePosition(_dimension, _targetPosition);
		// Determine the absolute offset between current frame boundary and target point.
		// We reconstruct the target point from above relative to avoid issues if
		// the relative position does not reconstruct the initial position exactly.
		absoluteDistance = _anchorPosition - _target->getPosition(_dimension, relativePosition);
		_target->linkAnchor(*this);
	}

	float Anchor::getPosition(int _dimension) const
	{
		if(!reference()) return absoluteDistance;
		const float relRef = reference()->getPosition(_dimension, relativePosition);
		//reference->side(_a) + relativePosition * (reference->side(_b) - reference->side(_a));
		return relRef + absoluteDistance;
	}



	IAnchorProvider::~IAnchorProvider()
	{
		// Recursively null all the linked anchors
		Anchor* current = m_anchorListStart.m_next;
		while(current)
		{
			Anchor* next = current->m_next;
			current->m_next = nullptr;
			current->m_prev = nullptr;
			current = next;
		}
	}


	void IAnchorProvider::linkAnchor(Anchor& _anchor) const
	{
		if(_anchor.m_reference == this)
			return;
		if(_anchor.m_reference)
			_anchor.detach();
		_anchor.m_next = m_anchorListStart.m_next;
		_anchor.m_prev = &m_anchorListStart;
		_anchor.m_reference = this;
		if(_anchor.m_next)
			_anchor.m_next->m_prev = &_anchor;
		m_anchorListStart.m_next = &_anchor;
	}


	void IAnchorProvider::onExtentChanged()
	{
		// Do a level order update of all dependent content.
		Anchor* next = m_anchorListStart.m_next;
		while(next)
		{
			// This will silently update the size
			next->self()->refitToAnchors();
			next = next->m_next;
		}

		// Now trigger the true updates including the recursion
		next = m_anchorListStart.m_next;
		while(next)
		{
			next->self()->onExtentChanged();
			next = next->m_next;
		}
	}



	IAnchorable::IAnchorable()
	{
		m_geomVersion = s_globalGeomVersion;
		m_anchoringEnabled = true;
	}

	void IAnchorable::refitToAnchors()
	{
		matchGeomVersion();
	}

	void IAnchorable::resetAnchors()
	{
		matchGeomVersion();
	}

	void IAnchorable::setAnchorable(bool _enable)
	{
		if(_enable != m_anchoringEnabled)
			resetAnchors();
		m_anchoringEnabled = _enable;
	}

}} // namespace ca::gui