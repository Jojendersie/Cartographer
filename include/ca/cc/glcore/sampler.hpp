#pragma once

#include <ei/elementarytypes.hpp>

namespace ca { namespace cc {
	
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
			REPEAT = 0x2901,	///< GL_REPEAT
			MIRROR = 0x8370,	///< GL_MIRRORED_REPEAT
			CLAMP  = 0x812F,	///< GL_CLAMP_TO_EDGE
			BORDER = 0x812D,	///< GL_CLAMP_TO_BORDER
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

}} // namespace ca::cc
