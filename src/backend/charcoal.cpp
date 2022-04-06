#ifdef CHARCOAL_BACKEND
#include "ca/gui/backend/charcoal.hpp"
#include "ca/gui/guimanager.hpp"
#include <ca/cc/glcore/opengl.hpp>

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
		#version 410

		layout(location = 0) in vec4 in_texCoords;
		layout(location = 1) in uvec2 in_textureHandle;
		layout(location = 3) in vec3 in_position;
		layout(location = 5) in vec4 in_scale;
		layout(location = 7) in vec4 in_posAB;
		layout(location = 8) in vec4 in_colorA;
		layout(location = 9) in vec4 in_colorB;
		layout(location = 10) in vec4 in_colorC;
		layout(location = 11) in int in_gradientMode;
	
		layout(location = 0) out vec4 out_texCoords;
		layout(location = 1) out uvec2 out_textureHandle;
		layout(location = 2) out vec3 out_position;
		layout(location = 3) out vec4 out_scale;
		layout(location = 4) out vec4 out_posAB;
		layout(location = 5) out vec4 out_colorA;
		layout(location = 6) out vec4 out_colorB;
		layout(location = 7) out vec4 out_colorC;
		layout(location = 8) out int out_gradientMode;

		void main()
		{
			out_texCoords = in_texCoords;
			out_textureHandle = in_textureHandle;
			out_position = in_position;
			out_scale = in_scale;
			out_posAB = in_posAB;
			out_colorA = in_colorA;
			out_colorB = in_colorB;
			out_colorC = in_colorC;
			out_gradientMode = in_gradientMode;
		}
	)";
	const char* GS_SPRITE = R"(
		#version 430

		layout(location = 0) in vec4 in_texCoords[1];
		layout(location = 1) in uvec2 in_textureHandle[1];
		layout(location = 2) in vec3 in_position[1];
		layout(location = 3) in vec4 in_scale[1];
		layout(location = 4) in vec4 in_posAB[1];
		layout(location = 5) in vec4 in_colorA[1];
		layout(location = 6) in vec4 in_colorB[1];
		layout(location = 7) in vec4 in_colorC[1];
		layout(location = 8) in int in_gradientMode[1];

		layout(location = 0) uniform mat4 c_projection;

		layout(points) in;
		layout(triangle_strip, max_vertices = 4) out;
		layout(location = 0) out vec2 out_texCoord;
		layout(location = 1) out flat uvec2 out_textureHandle;
		layout(location = 2) out flat vec4 out_posAB;
		layout(location = 3) out flat vec4 out_colorA;
		layout(location = 4) out flat vec4 out_colorB;
		layout(location = 5) out vec4 out_colorInterpolated;
		layout(location = 6) out flat int out_gradientMode;

		void main()
		{
			out_textureHandle = in_textureHandle[0];
			out_posAB = in_posAB[0];
			out_colorA = in_colorA[0];
			out_colorB = in_colorB[0];
			out_colorInterpolated = vec4(0.0);
			out_gradientMode = in_gradientMode[0];

			// Triangle mode?
			if(in_gradientMode[0] == 4)
			{
				gl_Position = vec4(in_position[0].xy, 0, 1) * c_projection;
				out_colorInterpolated = in_colorA[0];
				EmitVertex();
				gl_Position = vec4(in_posAB[0].xy, 0, 1) * c_projection;
				out_colorInterpolated = in_colorB[0];
				EmitVertex();
				gl_Position = vec4(in_posAB[0].zw, 0, 1) * c_projection;
				out_colorInterpolated = in_colorC[0];
				EmitVertex();
			} else {
				vec3 worldBasePos = in_position[0];
				worldBasePos.y += 1/16.0f; // Offset helps with pixel perfect rendering
				// Bottom-Left
				out_texCoord = in_texCoords[0].xy;
				vec3 worldPos = worldBasePos;
				worldPos.xy += in_scale[0].xy;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Bottom-Right
				out_texCoord = in_texCoords[0].zy;
				worldPos = worldBasePos;
				worldPos.xy += in_scale[0].zy;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Top-Left
				out_texCoord = in_texCoords[0].xw;
				worldPos = worldBasePos;
				worldPos.xy += in_scale[0].xw;
				gl_Position = vec4(worldPos, 1) * c_projection;
				EmitVertex();

				// Top-Right
				out_texCoord = in_texCoords[0].zw;
				worldPos = worldBasePos;
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
		layout(location = 2) in flat vec4 in_posAB;
		layout(location = 3) in flat vec4 in_colorA;
		layout(location = 4) in flat vec4 in_colorB;
		layout(location = 5) in vec4 in_colorInterpolated;
		layout(location = 6) in flat int in_gradientMode;

		layout(location = 0, index = 0) out vec4 out_color;

		void main()
		{
			vec4 color = vec4(0.0);
			// position A and B are given in sprite relative coordinates (i.e. texture coordinates)
			switch(in_gradientMode)
			{
			case 0: // Uniform color
				color = in_colorA;
				break;
			case 1: { // IRenderBackend::GradientMode::LINEAR
				// Project to a line between A and B
				// Texture coordinates and cagui coordinates must be toggled in y direction
				vec2 AtoB = in_posAB.zy - in_posAB.xw;
				float t = dot(AtoB, in_texCoord - in_posAB.xw) / dot(AtoB, AtoB);
				t = clamp(t, 0.0, 1.0);
				color = mix(in_colorA, in_colorB, t);
				break; }
			case 2: { // IRenderBackend::GradientMode::CIRCULAR
				// Compute relative distance to A
				vec2 AtoX = in_texCoord - in_posAB.xy;
				vec2 AtoB = in_posAB.zw - in_posAB.xy;
				float dsq = dot(AtoX, AtoX) / dot(AtoB, AtoB);
				float t = min(sqrt(dsq), 1.0);
				color = mix(in_colorA, in_colorB, t);
				break; }
			case 3: { // IRenderBackend::GradientMode::RECTANGULAR
				vec2 AtoXAbs = abs(in_texCoord - 0.5);
				//vec2 rectSize = abs(in_posAB.zw - in_posAB.xy);
				// Normalize to a 0-1 distance within the rect
				AtoXAbs *= 2.0;
				// Overfill and underfill
				vec2 finalDist = (AtoXAbs - in_posAB.xy) / (1.0 + in_posAB.zw - in_posAB.xy);
				float maxDist = max(0, max(finalDist.x, finalDist.y));
				color = mix(in_colorA, in_colorB, maxDist);
				break; }
			case 4:
				color = in_colorInterpolated;
				break;
			}
			// Premultiply color alpha
			color.rgb *= color.a;
			// Is there a texture?
			// If so it is never used with gradients together -> use in_posAB.xy
			// for tiling.
			if(in_textureHandle.x != 0 || in_textureHandle.y != 0)
			{
				sampler2D tex = sampler2D(in_textureHandle);
				//vec2 pixelSize = 1.0 / textureSize(tex, 0);
				color *= texture(tex, in_texCoord * in_posAB.xy);
			}

			if(color.a < 0.005) discard;
			out_color = color;
		}
	)";

	const char* VS_FONT = R"(
		#version 450

		layout(location = 0) in vec4 in_texCoords;
		layout(location = 1) in vec2 in_size;
		layout(location = 2) in vec4 in_color;
		layout(location = 3) in vec3 in_position;
		layout(location = 4) in float in_rotation;

		layout(location = 0) out vec4 out_texCoords;
		layout(location = 1) out vec2 out_size;
		layout(location = 2) out vec4 out_color;
		layout(location = 3) out vec3 out_position;
		layout(location = 4) out float out_rotation;

		void main()
		{
			out_texCoords = in_texCoords;
			out_size = in_size;
			out_color = in_color;
			out_position = in_position;
			out_rotation = in_rotation;
		}
	)";
	const char* GS_FONT = R"(
		#version 450

		layout(location = 0) in vec4 in_texCoords[1];
		layout(location = 1) in vec2 in_size[1];
		layout(location = 2) in vec4 in_color[1];
		layout(location = 3) in vec3 in_position[1];
		layout(location = 4) in float in_rotation[1];

		layout(location = 0) uniform mat4 c_viewProjection;

		layout(points) in;
		layout(triangle_strip, max_vertices = 4) out;
		layout(location = 0) out vec2 out_texCoord;
		layout(location = 1) out flat vec4 out_color;

		void main()
		{
			out_color = in_color[0];
	
			mat2 rot;
			rot[0][0] = rot[1][1] = cos(in_rotation[0]);
			rot[1][0] = sin(in_rotation[0]);
			rot[0][1] = - rot[1][0];
	
			vec2 toRight = rot * vec2(in_size[0].x, 0);
			vec2 toTop   = rot * vec2(0, in_size[0].y);
	
			// Bottom-Left
			out_texCoord = in_texCoords[0].xy;
			vec3 worldPos = in_position[0];
			gl_Position = vec4(worldPos, 1) * c_viewProjection;
			EmitVertex();

			// Bottom-Right
			out_texCoord = in_texCoords[0].zy;
			worldPos = in_position[0];
			worldPos.xy += toRight;
			gl_Position = vec4(worldPos, 1) * c_viewProjection;
			EmitVertex();

			// Top-Left
			out_texCoord = in_texCoords[0].xw;
			worldPos = in_position[0];
			worldPos.xy += toTop;
			gl_Position = vec4(worldPos, 1) * c_viewProjection;
			EmitVertex();

			// Top-Right
			out_texCoord = in_texCoords[0].zw;
			worldPos = in_position[0];
			worldPos.xy += toRight + toTop;
			gl_Position = vec4(worldPos, 1) * c_viewProjection;
			EmitVertex();

			EndPrimitive();
		}
	)";
	const char* PS_FONT = R"(
		#version 450

		layout(location = 0) in vec2 in_texCoord;
		layout(location = 1) in flat vec4 in_color;

		// A distance field or mask
		layout(binding = 0) uniform sampler2D tx_character;

		layout(location = 0, index = 0) out vec4 out_color;

		void main()
		{
			// Use the bitmap-font as if it is a distance field.
			// Compute the antialiasing-alpha based on the distance to 0.5.
			// https://www.reddit.com/r/gamedev/comments/2879jd/just_found_out_about_signed_distance_field_text/
			float dist = texture(tx_character, in_texCoord, -2.0).x - 0.5;
			float aa = length( vec2(dFdx(dist), dFdy(dist)) );
			float alpha = smoothstep( -aa, aa, dist );
			// Alpha-test (this is correct for distance fields and masks)
			if(alpha < 0.005) discard;
			out_color = in_color * alpha;
		}
	)";

	const char* VS_LINE = R"(
		#version 450

		layout(location = 0) in vec3 in_position;
		layout(location = 1) in vec4 in_color;

		layout(location = 0) uniform mat4 c_projection;

		layout(location = 0) out vec4 out_color;

		void main()
		{
			out_color = in_color;
			gl_Position = vec4(in_position, 1) * c_projection;
		}
	)";
	const char* PS_LINE = R"(
		#version 450

		layout(location = 0) in vec4 in_color;

		layout(location = 0, index = 0) out vec4 out_color;

		void main()
		{
			out_color = in_color;
		}
	)";

	CharcoalBackend::CharcoalBackend(const char* _fontFile) :
		m_fontRenderer(new cc::FontRenderer),
		m_linearSampler(cc::Sampler::Filter::LINEAR, cc::Sampler::Filter::LINEAR, cc::Sampler::Filter::LINEAR, cc::Sampler::Border::REPEAT),
		m_pointSampler(cc::Sampler::Filter::POINT, cc::Sampler::Filter::POINT, cc::Sampler::Filter::POINT, cc::Sampler::Border::REPEAT)
	{
		m_spriteShader.attach( cc::ShaderManager::get(VS_SPRITE, cc::ShaderType::VERTEX, false) );
		m_spriteShader.attach( cc::ShaderManager::get(GS_SPRITE, cc::ShaderType::GEOMETRY, false) );
		m_spriteShader.attach( cc::ShaderManager::get(PS_SPRITE, cc::ShaderType::FRAGMENT, false) );
		m_spriteShader.link();

		m_fontShader.attach( cc::ShaderManager::get(VS_FONT, cc::ShaderType::VERTEX, false) );
		m_fontShader.attach( cc::ShaderManager::get(GS_FONT, cc::ShaderType::GEOMETRY, false) );
		m_fontShader.attach( cc::ShaderManager::get(PS_FONT, cc::ShaderType::FRAGMENT, false) );
		m_fontShader.link();

		m_lineShader.attach( cc::ShaderManager::get(VS_LINE, cc::ShaderType::VERTEX, false) );
		m_lineShader.attach( cc::ShaderManager::get(PS_LINE, cc::ShaderType::FRAGMENT, false) );
		m_lineShader.link();

		m_fontRenderer->loadCaf(_fontFile);
		m_lineRenderer.reset(new cc::LineRenderer);

		m_spriteRenderer.reset(new cc::SpriteRenderer);
		// Create extra VBO for additional instance data
		// This must be done directly after the creation, as it needs the VAO to be bound
		cc::glCall(glGenBuffers, 1, &m_extraVBO);
		cc::glCall(glBindBuffer, GL_ARRAY_BUFFER, m_extraVBO);
		// 4 x float for posA and posB
		cc::glCall(glEnableVertexAttribArray, 7);
		cc::glCall(glVertexAttribPointer, 7, 4, GLenum(cc::PrimitiveFormat::FLOAT), GL_FALSE, 29, (GLvoid*)(0));
		// 4 x uint8 normalized as colorA
		cc::glCall(glEnableVertexAttribArray, 8);
		cc::glCall(glVertexAttribPointer, 8, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_TRUE, 29, (GLvoid*)(16));
		// 4 x uint8 normalized as colorB
		cc::glCall(glEnableVertexAttribArray, 9);
		cc::glCall(glVertexAttribPointer, 9, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_TRUE, 29, (GLvoid*)(20));
		// 4 x uint8 normalized as colorC
		cc::glCall(glEnableVertexAttribArray, 10);
		cc::glCall(glVertexAttribPointer, 10, 4, GLenum(cc::PrimitiveFormat::UINT8), GL_TRUE, 29, (GLvoid*)(24));
		// 1 x int8 gradient mode
		cc::glCall(glEnableVertexAttribArray, 11);
		cc::glCall(glVertexAttribIPointer, 11, 1, GLenum(cc::PrimitiveFormat::INT8), 29, (GLvoid*)(28));

		// Define geometry for triangles and rectangles
		m_spriteRenderer->defSprite(0.0f, 0.0f, 0); // Standard rect without texture
		m_spriteSizes.push_back( Vec2(1.0f) );
	}

	CharcoalBackend::~CharcoalBackend()
	{
		cc::glCall(glDeleteBuffers, 1, &m_extraVBO);
	}

	void CharcoalBackend::beginDraw()
	{
		// Create projection matrix and set in the program
		Mat4x4 viewProj = ei::orthographicGL(0.0f, (float)GUIManager::getWidth(), 0.0f, (float)GUIManager::getHeight(), 0.0f, 1.0f);
		m_spriteShader.setUniform(0, viewProj);
		m_fontShader.setUniform(0, viewProj);
		m_lineShader.setUniform(0, viewProj);

		// Prepare pipeline
		cc::Device::setCullMode(cc::CullMode::BACK);
		cc::Device::setZFunc(cc::ComparisonFunc::ALWAYS);
		cc::Device::setZWrite(false);
	}

	void CharcoalBackend::endDraw()
	{
		// Execute pending draw calls
		flush();

		// Reset unusual pipeline states
		cc::Device::disableScissorTest();
	}

	void gui::CharcoalBackend::beginLayer(const ei::IVec4& _clippingRect)
	{
		/*eiAssertWeak(_clippingRect.x >= 0 && _clippingRect.z >= 0
			&& _clippingRect.y <= GUIManager::getWidth() && _clippingRect.w <= GUIManager::getHeight(),
			"Clipping region is (partially) not visible.");*/
		// The upcomming state change avoids more buffering
		flush();
		cc::Device::scissorTest(
			max(0, _clippingRect.x),
			max(0, _clippingRect.z),
			min(_clippingRect.y - _clippingRect.x, GUIManager::getWidth()-1),
			min(_clippingRect.w - _clippingRect.z, GUIManager::getHeight()-1));
	}

	void gui::CharcoalBackend::drawText(const Vec2& _position, const char* _text, float _size, const Vec4& _color, float _alignX, float _alignY, float _rotation, bool _roundToPixel)
	{
		m_fontRenderer->draw(Vec3(_position, 0.0f), _text, _size, _color, _rotation, _alignX, _alignY, _roundToPixel);
	}

	ei::Rect2D CharcoalBackend::getTextBB(const ei::Vec2& _position, const char* _text, float _size, float _alignX, float _alignY, float _rotation, bool _roundToPixel)
	{
		return m_fontRenderer->getBoundingBox(Vec3(_position, 0.0f), _text, _size, _rotation, _alignX, _alignY, _roundToPixel);
	}

	uint CharcoalBackend::getTextCharacterPosition(const ei::Vec2 & _findPosition, const ei::Vec2 & _textPosition, const char * _text, float _size, float _alignX, float _alignY, float _rotation, bool _roundToPixel)
	{
		return m_fontRenderer->findPosition(_findPosition, _textPosition, _text, _size, _rotation, _alignX, _alignY, _roundToPixel);
	}

	void gui::CharcoalBackend::drawRect(const ei::Rect2D& _rect, const Vec4& _color)
	{
		m_spriteRenderer->newInstance(0, Vec3(_rect.min.x, _rect.min.y, 0.0f), 0.0f,
			_rect.max - _rect.min);

		AdditionalVertexInfo info;
		info.a = Vec2(0.0f);
		info.b = Vec2(0.0f);
		info.colorA = Vec<uint8, 4>(saturate(_color) * 255.0f);
		info.colorB = Vec<uint8, 4>(0);
		info.colorC = Vec<uint8, 4>(0);
		info.gradientType = 0;
		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawRect(const ei::Rect2D& _rect, const Vec2& _a, const Vec2& _b, const Vec4& _colorA, const Vec4& _colorB, GradientMode _mode)
	{
		m_spriteRenderer->newInstance(0, Vec3(_rect.min.x, _rect.min.y, 0.0f), 0.0f,
			_rect.max - _rect.min);

		AdditionalVertexInfo info;
		info.a = saturate(_a);
		info.b = _b;
		info.colorA = Vec<uint8, 4>(saturate(_colorA) * 255.0f);
		info.colorB = Vec<uint8, 4>(saturate(_colorB) * 255.0f);
		info.colorC = Vec<uint8, 4>(0);
		info.gradientType = (int)_mode + 1;
		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawTextureRect(const ei::Rect2D& _rect, uint64 _texture, float _opacity, bool _tiling)
	{
		//Vec2 spriteScale((_rect.width() - 1.0f) / m_spriteSizes[(int)_texture].x, (_rect.height() - 1.0f) / m_spriteSizes[(int)_texture].y);
		const Vec2 spriteScale = (_rect.max - _rect.min) / m_spriteSizes[(int)_texture];

		AdditionalVertexInfo info;
		info.a = _tiling ? spriteScale : Vec2(1.0f);//Vec2((m_spriteSizes[(int)_texture] - 1.0f) / m_spriteSizes[(int)_texture]);
		info.b = Vec2(0.0f);
		info.colorA = Vec<uint8, 4>(255, 255, 255, uint8(saturate(_opacity) * 255.0f));
		info.colorB = Vec<uint8, 4>(0);
		info.colorC = Vec<uint8, 4>(0);
		info.gradientType = 0;

		m_spriteRenderer->newInstance((int)_texture, Vec3(_rect.min.x, _rect.min.y, 0.0f),
			0.0f, spriteScale);

		m_perInstanceData.push_back(info);
	}

	void gui::CharcoalBackend::drawTriangle(const Triangle2D& _triangle, const Vec4& _color0, const Vec4& _color1, const Vec4& _color2)
	{
		m_spriteRenderer->newInstance(0, Vec3(_triangle.v0, 0.0f), 0.0f, Vec2(0.0f));

		AdditionalVertexInfo info;
		info.a = _triangle.v1;
		info.b = _triangle.v2;
		info.colorA = Vec<uint8, 4>(saturate(_color0) * 255.0f);
		info.colorB = Vec<uint8, 4>(saturate(_color1) * 255.0f);
		info.colorC = Vec<uint8, 4>(saturate(_color2) * 255.0f);
		info.gradientType = 4;
		m_perInstanceData.push_back(info);
	}

	uint64 gui::CharcoalBackend::getTexture(const char* _textureFile, bool _smooth)
	{
		cc::Texture2D::Handle tex = cc::Texture2DManager::get(_textureFile,
			_smooth ? m_linearSampler : m_pointSampler);
		m_spriteSizes.push_back( Vec2((float)tex->getWidth(), (float)tex->getHeight()) );
		return m_spriteRenderer->defSprite(0.0f, 0.0f, tex);
	}

	ei::IVec2 CharcoalBackend::getTextureSize(uint64 _handle)
	{
		return ei::IVec2(m_spriteSizes[size_t(_handle)]);
	}

	void CharcoalBackend::drawLine(const ei::Vec2* _positions, int _numPositions, const ei::Vec4& _colorA, const ei::Vec4& _colorB)
	{
		m_lineRenderer->beginLine();
		for(int i = 0; i < _numPositions; ++i)
			m_lineRenderer->putVertex(Vec3{_positions[i], 0.0f}, lerp(_colorA, _colorB, i / (_numPositions - 1.0f)));
		m_lineRenderer->endLine();
	}

	void CharcoalBackend::flush()
	{
		// Only if there are instances
		if(!m_spriteRenderer->isEmpty())
		{
			cc::glCall(glBindBuffer, GL_ARRAY_BUFFER, m_extraVBO);
			cc::glCall(glBufferData, GL_ARRAY_BUFFER, m_perInstanceData.size() * sizeof(AdditionalVertexInfo), m_perInstanceData.data(), GL_DYNAMIC_DRAW);
			// Enable alpha blending (permultiplied)
			cc::Device::enableBlending(true);
			cc::Device::setBlendFactor(cc::BlendFactor::ONE, cc::BlendFactor::INV_SRC_ALPHA);
			m_spriteShader.use();
			m_spriteRenderer->draw();
					
			m_spriteRenderer->clearInstances();
			m_perInstanceData.clear();
		}

		if(!m_lineRenderer->isEmpty())
		{
			// TODO: alpha blending?
			// TODO: anti aliasing?
			m_lineShader.use();
			m_lineRenderer->draw();
			m_lineRenderer->clearLines();
		}

		if(!m_fontRenderer->isEmpty())
		{
			// Enable alpha blending (permultiplied)
			cc::Device::enableBlending(true);
			cc::Device::setBlendFactor(cc::BlendFactor::ONE, cc::BlendFactor::INV_SRC_ALPHA);
			m_fontShader.use();
			m_fontRenderer->present();

			m_fontRenderer->clearText();
		}
	}

}} // namespace ca::gui

#endif