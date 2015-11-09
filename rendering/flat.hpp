#pragma once

#include <ei/vector.hpp>
#include "theme.hpp"

namespace cag {

	struct FlatProperties
	{
		Vec4 backgroundColor;	///< Color for buttons...
		Vec4 foregroundColor;	///< Color for arrows...
		Vec4 textColor;			///< Color for text
		Vec4 textBackColor;		///< Color for text areas...
		Vec4 hoverBackgroundColor;	///< Color for buttons... when mouse is over
		Vec4 hoverTextColor;	///< Color for texts when mouse is over
	};

	/// Super simple look without borders or gradients.
	class FlatTheme: public ITheme
	{
	public:
		FlatTheme(const FlatProperties& _desc);
	};

} // namespace cag