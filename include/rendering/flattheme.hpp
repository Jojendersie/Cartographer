#pragma once

#include <ei/vector.hpp>
#include "theme.hpp"

namespace cag {

	struct FlatProperties
	{
		ei::Vec4 backgroundColor;		///< Color for buttons...
		ei::Vec4 foregroundColor;		///< Color for arrows...
		ei::Vec4 textColor;				///< Color for text
		ei::Vec4 textBackColor;			///< Color for text areas...
		ei::Vec4 hoverBackgroundColor;	///< Color for buttons... when mouse is over
		ei::Vec4 hoverTextColor;		///< Color for texts when mouse is over
	};

	/// Super simple look without borders or gradients.
	class FlatTheme: public ITheme
	{
	public:
		FlatTheme(const FlatProperties& _desc);
	};

} // namespace cag