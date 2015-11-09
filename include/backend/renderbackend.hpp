#pragma once

namespace cag {

	/// Interface for elementary rendering opertions.
	class IRenderBackend
	{
	public:
		virtual void drawText();
		virtual void drawRect(CustomShader);
		virtual void drawTextureRect();
	};

} // namespace cag