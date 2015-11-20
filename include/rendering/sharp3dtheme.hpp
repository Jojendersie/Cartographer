#pragma once

#include "theme.hpp"

namespace ca { namespace gui {

	struct Sharp3DProperties
	{
		int borderWidth;	/// Width of border around buttons... 0 or 1 look best.
		// Colors...
	};

	/// A look with sharp shapes and moderate gradients
	class Sharp3DTheme: public ITheme
	{
	public:
		Sharp3DTheme(const Sharp3DProperties& _desc);
	};

}} // namespace ca::gui
