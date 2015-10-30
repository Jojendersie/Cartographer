#pragma once

#include <gl/glew.h>
#include <ei/elementarytypes.hpp>

namespace cac {
	
	/// A sampler defines how a texture is sampled (border handling and scaling)
	class Sampler
	{
	public:
		enum class Filter
		{
			POINT,
			LINEAR
		};

		enum class Border
		{
			REPEAT = GL_REPEAT,
			MIRROR = GL_MIRRORED_REPEAT,
			CLAMP = GL_CLAMP_TO_EDGE,
			BORDER = GL_CLAMP_TO_BORDER
		};

		Sampler(Filter _minFilter, Filter _magFilter, Filter _mipFilter, Border _borderHandling = Border::REPEAT, uint _maxAnisotropy = 1);
		Sampler(const Sampler&) = delete;
		Sampler& operator = (const Sampler&) = delete;
		~Sampler();

		uint getID() const { return m_samplerID; }

		void bind(uint _textureSlot) const;

	private:
		uint m_samplerID;
		Filter m_minFilter, m_magFilter, m_mipFilter;
		Border m_borderHandling;
		uint m_maxAnisotropy;
	};

} // namespace cac
