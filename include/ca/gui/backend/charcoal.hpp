#ifdef CHARCOAL_BACKEND
#pragma once

#include "renderbackend.hpp"
#include <ca/charcoal.hpp>
#include <memory>

namespace ca { namespace gui {

	/// Backend implementation that uses the Cartographer-Charcoal renderer.
	class CharcoalBackend : public IRenderBackend
	{
	public:
		CharcoalBackend(const char* _fontFile);
		~CharcoalBackend();

		virtual void beginDraw() override;
		virtual void endDraw() override;
		virtual void beginLayer(const ei::IVec4& _clippingRect) override;
		virtual void drawText(const ei::Vec2& _position, const char* _text, float _size, const ei::Vec4& _color, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f, bool _roundToPixel = false) override;
		virtual ei::Rect2D getTextBB(const ei::Vec2& _position, const char* _text, float _size, float _alignX = 0.0f, float _alignY = 0.0f, float _rotation = 0.0f) override;
		virtual void drawRect(const class RefFrame& _rect, const ei::Vec4& _color) override;
		virtual void drawRect(const class RefFrame& _rect, const ei::Vec2& _a, const ei::Vec2& _b, const ei::Vec4& _colorA, const ei::Vec4& _colorB, GradientMode _mode = GradientMode::LINEAR) override;
		virtual void drawTextureRect(const class RefFrame& _rect, uint64 _texture, float _opacity, bool _tiling) override;
		virtual void drawTriangle(const ei::Triangle2D& _triangle, const ei::Vec4& _color0, const ei::Vec4& _color1, const ei::Vec4& _color2) override;
		virtual uint64 getTexture(const char* _textureFile, bool _smooth) override;
		virtual ei::IVec2 getTextureSize(uint64 _handle) override;
		virtual void drawLine(const ei::Vec3* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB) override;

	private:
#pragma pack(push, 1)
		struct AdditionalVertexInfo
		{
			ei::Vec2 a;
			ei::Vec2 b;
			ei::Vec<uint8, 4> colorA;
			ei::Vec<uint8, 4> colorB;
			ei::Vec<uint8, 4> colorC;
			int8 gradientType;				///< Uniform=0, IRenderBackend::GradientMode, Triangle=4
		};
#pragma pack(pop)

		std::unique_ptr<cc::FontRenderer> m_fontRenderer;
		std::unique_ptr<cc::SpriteRenderer> m_spriteRenderer;	///< Uber-shader for textured, colored and gradient rectangles/triangles
		std::unique_ptr<cc::LineRenderer> m_lineRenderer;
		std::vector<ei::Vec2> m_spriteSizes;
		cc::Program m_spriteShader;
		cc::Program m_fontShader;
		cc::Program m_lineShader;
		cc::Sampler m_linearSampler;
		cc::Sampler m_pointSampler;
		uint m_extraVBO;
		std::vector<AdditionalVertexInfo> m_perInstanceData;

		// Drawcalls are batched in the three renderers. This command makes the real drawcalls
		// and clears the renderers.
		void flush();
	};

}} // namespace ca::gui

#endif