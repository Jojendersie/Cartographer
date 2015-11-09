#pragma once

#include "theme.hpp"

namespace cag {

	struct SharpProperties
	{
		int borderWidth;	/// Width of border around buttons... 0 or 1 look best.
		// Colors...
	};

	/// A look with sharp shapes and moderate gradients
	class SharpTheme: public ITheme
	{
	public:
		SharpTheme(const SharpProperties& _desc);
	};

} // namespace cag