#ifdef CHARCOAL_BACKEND
#include "backend/charcoal.hpp"
#include "properties/refframe.hpp"
#include "guimanager.hpp"
#include <glcore/opengl.hpp>

using namespace ei;

namespace ca { namespace gui {

	enum class UniformLocations
	{
		PROJECTION_MATRIX = 0,
		POS_A = 1,
		POS_B = 2,
		COLOR_A = 3,
		COLOR_B = 4,
	};

	const char* VS_SPRITE = R"(
		#version 330

		layout(location = 0) in vec4 in_texCoords;
		layout(location = 1) in uvec2 in_textureHandle;
		layout(location = 3) in vec3 in_position;
		layout(location = 5) in vec4 in_scale;
		layout(location = 7) in uvec4 in_clippingRect;
		layout(location = 8) in vec4 in_posAB;
		layout(location = 9) in vec4 in_colorA;
		layout(location = 10) in vec4 in_colorB;
		layout(location = 11) in int in_gradientMode;
	
		layout(location = 0) out vec4 out_texCoords;
		layout(location = 1) out uvec2 out_textureHandle;
		layout(location = 2) out vec3 out_position;
		layout(location = 3) out vec4 out_scale;
		layout(location = 4) out uvec4 out_clippingRect;
		layout(location = 5) out vec4 out_posAB;
		layout(location = 6) out vec4 out_colorA;
		layout(location = 7) out vec4 out_colorB;
		layout(location = 8) out int out_gradientMode;

		void main()
		{
			out_texCoords = in_texCoords;
			out_textureHandle = in_textureHandle;
			out_position = in_position;
			out_scale = in_scale;
			out_clippingRect = in_clippingRect;
			out_posAB = in_posAB;
			out_colorA = in_colorA;
			out_colorB = in_colorB;
			out_gradientMode = in_gradientMode;
		}
	)";
	const char* GS_SPRITE = R"(
		#version 330

		layout(location = 0) in vec4 in_texCoords[1];
		layout(location = 1) in uvec2 in_textureHandle[1];
		layout(location = 2) in vec3 in_position[1];
		layout(location = 3) in vec4 in_scale[1];
		layout(location = 4) in uvec4 in_clippingRect[1];
		layout(location = 5) in vec4 in_posAB[1];
		layout(location = 5) in vec4 in_colorA[1];
		layout(location = 7) in vec4 in_colorB[1];
		layout(location = 8) in int in_gradientMode[1];

		layout(location = 0) uniform mat4 c_projection;

		layout(points) in;
		layout(triangle_strip, max_vertices = 4) out;
		layout(location = 0) out vec2 out_texCoord;
		layout(location = 1) out flat uvec2 out_textureHandle;
		layout(location = 2) out flat uvec4 out_clippingRect;
		layout(location = 3) out flat vec4 out_posAB;
		layout(location = 4) out flat vec4 out_colorA;
		layout(location = 5) out flat vec4 out_colorB;
		layout(location = 6) out flat int out_gradientMode

		void main()
		{
			out_textureHandle = in_textureHandle[0];
			out_clippingRect = in_clippingRect[0];
			out_posAB = in_posAB[0];
			out_colorA = in_colorA[0];
			out_colorB = in_colorB[0];
			out_gradintMode = in_gradientMode[0];

			// Triangle mode?
			if(in_gradientMode == 4)
			{
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();
				gl_Position = vec4(in_posAB.xy, 0, 1) * c_projection;
				EmitVertex();
				gl_Position = vec4(in_posAB.zw, 0, 1) * c_projection;
				EmitVertex();
			} else {
				// Bottom-Left
				out_texCoord = in_texCoords[0].xy;
				vec3 worldPos = in_position[0];
				worldPos.xy += in_scale[0].xy;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Bottom-Right
				out_texCoord = in_texCoords[0].zy;
				worldPos = in_position[0];
				worldPos.xy += in_scale[0].zy;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Top-Left
				out_texCoord = in_texCoords[0].xw;
				worldPos = in_position[0];
				worldPos.xy += in_scale[0].xw;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Top-Right
				out_texCoord = in_texCoords[0].zw;
				worldPos = in_position[0];
				worldPos.xy += in_scale[0].zw;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();
			}

			EndPrimitive();
		}
	)";
	const char* PS_SPRITE = R"(
		#version 450
		#extension GL_ARB_bindless_texture : enable

		layout(location = 0) in vec2 in_texCoord;
		layout(location = 1) in flat uvec2 in_textureHandle;
		layout(location = 2) in flat uvec4 in_clippingRect;
		layout(location = 3) in flat vec4 in_posAB;
		layout(location = 4) in flat vec4 in_colorA;
		layout(location = 5) in flat vec4 in_colorB;
		layout(location = 6) in flat int in_gradientMode;

		layout(location = 0, index = 0) out vec4 out_color;

		void main()
		{
			// Pixel inside clipping region?
			if( gl_FragCoord.x < in_clippingRect.x || gl_FragCoord.x >= in_clippingRect.y
			  || gl_FragCoord.y < in_clippingRect.z || gl_FragCoord.y >= in_clippingRect.w )
				discard;

			vec4 color;
			// position A and B are given in sprite relative coordinates (i.e. texture coordinates)
			switch(in_gradientMode)
			{
			case 0: // Uniform color
			case 4:
				color = in_colorA;
				break;
			case 1: // IRenderBackend::GradientMode::LINEAR
				// Project to a line between A and B
				vec2 AtoB = in_posAB.zw - in_posAB.xy;
				float t = dot(AtoB, in_texCoord - in_posAB.xy) / dot(AtoB, AtoB);
				t = clamp(t, 0.0, 1.0);
				color = mix(in_colorA, in_colorB, t);
				break;
			case 2: // IRenderBackend::GradientMode::CIRCULAR
				// Compute relative distance to A
				vec2 AtoX = in_texCoord - in_posAB.xy;
				vec2 AtoB = in_posAB.zw - in_posAB.xy;
				float dsq = dot(AtoX, AtoX) / dot(AtoB, AtoB);
				float t = min(sqrt(dsq), 1.0);
				color = mix(in_colorA, in_colorB, t);
				break;
			case 3: // IRenderBackend::GradientMode::RECTANGULAR
				// Get smallest distance to one of the borders
				float mindist;
				mindist = max(0.0, in_texCoord.x - in_posAB.x);
				mindist = min(mindist, max(0.0, in_posAB.z - in_texCoord.x));
				mindist = min(mindist, max(0.0, in_texCoord.y - in_posAB.y));
				mindist = min(mindist, max(0.0, in_posAB.w - in_texCoord.y));
				// Normalize to maximal possible distance
				mindist /= min(in_posAB.z - in_posAB.x, in_posAB.w - in_posAB.y) * 0.5;
				color = mix(in_colorA, in_colorB, mindist);
				break;
			}
			// Is there a texture?
			if(in_textureHandle.x != 0 || in_textureHandle.y != 0)
			{
				sampler2D tex = sampler2D(in_textureHandle);
				color *= texture(tex, in_texCoord);
			}

			if(color.a < 0.005) discard;
			out_color = color;
		}
	)";

	CharcoalBackend::CharcoalBackend() :
		m_fontRenderer(new cc::FontRenderer),
		m_linearSampler(cc::Sampler::Filter::LINEAR, cc::Sampler::Filter::LINEAR, cc::Sampler::Filter::LINEAR),
		m_pointSampler(cc::Sampler::Filter::POINT, cc::Sampler::Filter::POINT, cc::Sampler::Filter::POINT),
		m_clipRegion(0xffff)
	{
		m_spriteShader.attach( cc::ShaderManager::get(VS_SPRITE, cc::ShaderType::VERTEX, false) );
		m_spriteShader.attach( cc::ShaderManager::get(GS_SPRITE, cc::ShaderType::GEOMETRY, false) );
		m_spriteShader.attach( cc::ShaderManager::get(PS_SPRITE, cc::ShaderType::FRAGMENT, false) );
		m_spriteShader.link();

		m_spriteRenderer.reset(new cc::SpriteRenderer);
		// Define geometry for triangles and rectangles
		m_spriteRenderer->defSprite(0.0f, 0.0f, 0); // Standard rect without texture

		// Create extra VBO for clipping rectangles
		cc::glCall(glGenBuffers, 1, &m_extraVBO);
		cc::glCall(glBindBuffer, GL_ARRAY_BUFFER, m_extraVBO);
		// 4 x uint16 for clipping l, r, b and t
		cc::glCall(glEnableVertexAttribArray, 7);
		cc::glCall(glVertexAttribPointer, 7, 4, GLenum(cc::PrimitiveFormat::UINT16), GL_FALSE, 33, (GLvoid*)(0));
		// 4 x float for posA and posB
		cc::glCall(glEnableVertexAttribArray, 8);
		cc::glCall(glVertexAttribPointer, 8, 4, GLenum(cc::PrimitiveFormat::FLOAT), GL_FALSE, 33, (GLvoid*)(8));
		// 4 x uint8 normalized as colorA
		cc::glCall(glEnableVertexAttribArray, 9);
		cc::glCall(glVertexAttribPointer, 9, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_TRUE, 33, (GLvoid*)(24));
		// 4 x uint8 normalized as colorB
		cc::glCall(glEnableVertexAttribArray, 10);
		cc::glCall(glVertexAttribPointer, 10, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_TRUE, 33, (GLvoid*)(28));
		// 1 x uint8 gradient mode
		cc::glCall(glEnableVertexAttribArray, 11);
		cc::glCall(glVertexAttribPointer, 11, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_FALSE, 33, (GLvoid*)(28));
	}

	CharcoalBackend::~CharcoalBackend()
	{
		cc::glCall(glDeleteBuffers, 1, &m_extraVBO);
	}

	void CharcoalBackend::beginDraw()
	{
		m_clipRegion = ei::Vec<uint16, 4>(0xffff);
		// Create projection matrix and set in the program
		Mat4x4 viewProj = ei::orthographicGL(0.0f, (float)GUIManager::getWidth(), 0.0f, (float)GUIManager::getHeight(), 0.0f, 1.0f);
		m_spriteShader.setUniform(0, viewProj);
	}

	void CharcoalBackend::endDraw()
	{
		// Execute pending draw calls
		flush();
	}

	void CharcoalBackend::beginLayer()
	{
		// TODO: less flush by detecting overlaps
		flush();
	}

	void gui::CharcoalBackend::setClippingRegion(const ei::IVec4& _clippingRect)
	{
		eiAssertWeak(_clippingRect.x >= 0 && _clippingRect.y >= 0
			&& _clippingRect.z < GUIManager::getWidth() && _clippingRect.w < GUIManager::getHeight(),
			"Clipping region is (partially) not visible.");
		m_clipRegion = ei::Vec<uint16, 4>(_clippingRect);
	}

	void gui::CharcoalBackend::drawText(const Vec2& _position, const char* _text, float _size, const Vec4& _color, float _alignX, float _alignY, float _rotation, bool _roundToPixel)
	{
		m_fontRenderer->draw(Vec3(_position, 0.0f), _text, _size, _color, _rotation, _alignX, _alignY, _roundToPixel);
	}

	void gui::CharcoalBackend::drawRect(const RefFrame& _rect, const Vec4& _color)
	{
		m_spriteRenderer->newInstance(0, Vec3(_rect.left(), _rect.bottom(), 0.0f), 0.0f, Vec2(_rect.width(), _rect.height()));

		AdditionalVertexInfo info;
		info.clipRegion = m_clipRegion;
		info.a = Vec2(0.0f);
		info.b = Vec2(0.0f);
		info.colorA = Vec<uint8, 4>(_color * 255.0f);
		info.colorB = Vec<uint8, 4>(0);
		info.gradientType = 0;
		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawRect(const RefFrame& _rect, const Vec2& _a, const Vec2& _b, const Vec4& _colorA, const Vec4& _colorB, GradientMode _mode)
	{
		m_spriteRenderer->newInstance(0, Vec3(_rect.left(), _rect.bottom(), 0.0f), 0.0f, Vec2(_rect.width(), _rect.height()));

		AdditionalVertexInfo info;
		info.clipRegion = m_clipRegion;
		info.a = _a;
		info.b = _b;
		info.colorA = Vec<uint8, 4>(_colorA * 255.0f);
		info.colorB = Vec<uint8, 4>(_colorB * 255.0f);
		info.gradientType = (int)_mode;
		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawTextureRect(const RefFrame& _rect, uint64 _texture, float _opacity)
	{
		m_spriteRenderer->newInstance((int)_texture, Vec3(_rect.left(), _rect.bottom(), 0.0f), 0.0f, Vec2(_rect.width(), _rect.height()));

		AdditionalVertexInfo info;
		info.clipRegion = m_clipRegion;
		info.a = Vec2(0.0f);
		info.b = Vec2(0.0f);
		info.colorA = Vec<uint8, 4>(0, 0, 0, uint8(_opacity * 255.0f));
		info.colorB = Vec<uint8, 4>(0);
		info.gradientType = 0;
		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawTriangle(const Triangle2D& _triangle, const Vec4& _color)
	{
		m_spriteRenderer->newInstance(0, Vec3(_triangle.v0, 0.0f), 0.0f, Vec2(0.0f));

		AdditionalVertexInfo info;
		info.clipRegion = m_clipRegion;
		info.a = _triangle.v1;
		info.b = _triangle.v2;
		info.colorA = Vec<uint8, 4>(_color * 255.0f);
		info.colorB = Vec<uint8, 4>(0);
		info.gradientType = 4;
		m_perInstanceData.push_back(info);
	}

	uint64 gui::CharcoalBackend::getTexture(const char* _textureFile, bool _smooth)
	{
		cc::Texture2D::Handle tex = cc::Texture2DManager::get(_textureFile,
			_smooth ? m_linearSampler : m_pointSampler);
		return m_spriteRenderer->defSprite(0.0f, 0.0f, tex);
	}

	void CharcoalBackend::flush()
	{
		// TODO: only if there are instances (is this checked by draw itself?)
		cc::glCall(glBindBuffer, GL_ARRAY_BUFFER, m_extraVBO);
		cc::glCall(glBufferData, GL_ARRAY_BUFFER, m_perInstanceData.size() * 8, m_perInstanceData.data(), GL_DYNAMIC_DRAW);
		m_spriteShader.use();
		m_spriteRenderer->draw();
		m_fontRenderer->present();

		m_spriteRenderer->clearInstances();
		m_fontRenderer->clearText();
		m_perInstanceData.clear();
	}

}} // namespace ca::gui

#endif