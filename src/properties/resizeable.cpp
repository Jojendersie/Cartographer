#pragma once

#include "properties/resizeable.hpp"

namespace ca { namespace gui {

	Resizeable::Resizeable(RefFrame* _selfFrame, Anchorable* _anchorable)
	{
	}

	bool Resizeable::processInput(const MouseState& _mouseState)
	{
		return false;
	}

}} // namespace ca::gui
