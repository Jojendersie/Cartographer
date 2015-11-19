#pragma once

#include "properties/resizeable.hpp"

namespace cag {

	Resizeable::Resizeable(RefFrame* _selfFrame, Anchorable* _anchorable)
	{
	}

	bool Resizeable::processInput(const MouseState& _mouseState)
	{
		return false;
	}

} // namespace cag