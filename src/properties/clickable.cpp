#pragma once

#include "properties/clickable.hpp"

namespace cag {

	Clickable::Clickable(RefFrame* _selfFrame) :
		m_clickRegion(_selfFrame),
		m_deleteRegion(false)
	{
	}

} // namespace cag