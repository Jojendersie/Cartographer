#include "glcore/opengl.hpp"
#include "renderer/linerenderer.hpp"

static const ca::cc::VertexAttribute LINE_VERTEX_ATTRIBUTES[2] = {
	{ca::cc::PrimitiveFormat::FLOAT, 3, false},
	{ca::cc::PrimitiveFormat::UINT8, 4, true}
};

namespace ca { namespace cc {
	
	LineRenderer::LineRenderer() :
		m_GPUBuffer(GLPrimitiveType::LINES,
			LINE_VERTEX_ATTRIBUTES,
			2, 4)
	{
	}

	void LineRenderer::beginLine()
	{
		m_lineVertexCount = 0;
	}

	void LineRenderer::putVertex(const ei::Vec3& _position, const ei::Vec4& _color)
	{
		// Continue a stripe by douplicating the last vertex index
		if(m_lineVertexCount++ > 1)
		{
			m_lineIndices.push_back(m_lineIndices.back());
		}
		m_lineIndices.push_back(m_lineData.size());

		LineVertex newVertex;
		newVertex.position = _position;
		newVertex.color = uint8(ei::clamp(_color.x, 0.0f, 1.0f) * 255.0f);

		m_lineData.push_back(newVertex);
	}

	void LineRenderer::endLine()
	{
		// Include the new data in the next upload
		m_dirty = true;
	}

	void LineRenderer::clearLines()
	{
		m_lineData.clear();
		m_lineIndices.clear();
	}

	void LineRenderer::draw() const
	{
		if(m_dirty)
		{
			m_GPUBuffer.setData(m_lineData.data(), m_lineData.size() * sizeof(LineVertex));
			m_GPUBuffer.setIndexData(m_lineIndices.data(), m_lineIndices.size() * sizeof(uint32));
			m_dirty = false;
		}

		m_GPUBuffer.draw();
	}
}}