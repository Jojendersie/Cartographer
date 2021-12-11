#include "ca/cc/renderer/instancerenderer.hpp"
#include "ca/cc/glcore/opengl.hpp"
#include <ca/pa/log.hpp>
#include <string>

using namespace ei;

namespace ca { namespace cc {

	using namespace pa;

InstanceRenderer::InstanceRenderer(GLPrimitiveType _type, const VertexAttribute* _attributes, int _numAttributes) :
	m_geometryType(_type),
	m_startNewPrimitive(0)
{
	if(_type == GLPrimitiveType::POINTS)
		m_glType = GLPrimitiveType::POINTS;
	else if(_type == GLPrimitiveType::LINES || _type == GLPrimitiveType::LINE_STRIPE)
		m_glType = GLPrimitiveType::LINES;
	else if(_type == GLPrimitiveType::TRIANGLES || _type == GLPrimitiveType::TRIANGLE_STRIPE)
		m_glType = GLPrimitiveType::TRIANGLES;

	glCall(glGenVertexArrays, 1, &m_vao);
	glCall(glBindVertexArray, m_vao);

	// Create buffer without data for now
	glCall(glGenBuffers, 1, &m_vbo);
	glCall(glGenBuffers, 1, &m_vboInstances);
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

	// Define the vertex format
	m_vertexSize = 0;
	for(int i = 0; i < _numAttributes; ++i)
	{
		AttributeDefinition internalDef;
		internalDef.type = _attributes[i].type;
		internalDef.numComponents = _attributes[i].numComponents;
		internalDef.offset = m_vertexSize;
		internalDef.normalize = _attributes[i].normalize;
		m_attributes.push_back(internalDef);
		m_vertexSize += attributeSize(_attributes[i]) / 4;
	}

	for(int i = 0; i < _numAttributes; ++i)
	{
		glCall(glEnableVertexAttribArray, i);
		glCall(glVertexAttribDivisor, unsigned(i), 0);
		if( isFloatFormat(_attributes[i].type) || _attributes[i].normalize )
			glCall(glVertexAttribPointer,
					unsigned(i),
					_attributes[i].numComponents,
					GLenum(_attributes[i].type),
					GLboolean(_attributes[i].normalize),
					m_vertexSize * sizeof(float),
					(GLvoid*)(m_attributes[i].offset * sizeof(float)));
		else
			glCall(glVertexAttribIPointer,
					unsigned(i),
					_attributes[i].numComponents,
					GLenum(_attributes[i].type),
					m_vertexSize * sizeof(float),
					(GLvoid*)(m_attributes[i].offset * sizeof(float)));
	}
	// Buffer for later put() commands.
	m_currentVertex.resize(m_vertexSize);

	// Add declaration for instance data
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vboInstances);
	glCall(glEnableVertexAttribArray, _numAttributes);
	glCall(glVertexAttribPointer, _numAttributes, 4, GL_FLOAT, GL_FALSE, 28, nullptr);
	glCall(glVertexAttribDivisor, _numAttributes, 1);
	glCall(glEnableVertexAttribArray, _numAttributes+1);
	glCall(glVertexAttribPointer, _numAttributes+1, 3, GL_FLOAT, GL_FALSE, 28, (GLvoid*)(4 * sizeof(float)));
	glCall(glVertexAttribDivisor, _numAttributes+1, 1);

	glCall(glGenBuffers, 1, &m_ibo);
}

int InstanceRenderer::beginDef()
{
	// Create new mesh definition without vertices so far.
	MeshDefinition meshDef;
	meshDef.indexCount = 0;
	meshDef.indexOffset = (unsigned)m_indexData.size() * 4;
	meshDef.numInstances = 0;
	meshDef.vertexCount = 0;
	meshDef.vertexOffset = (unsigned)m_vertexData.size();
	m_meshes.push_back(meshDef);
	return (int)m_meshes.size()-1;
}

void InstanceRenderer::put(int _attrIndex, const float _value)
{
	if( m_attributes[_attrIndex].numComponents != 1 )
	{
		logError("Vertex attribute ", _attrIndex, " is incompatible with put(float)! The component count is wrong.");
		return;
	}

	if( m_attributes[_attrIndex].type == PrimitiveFormat::FLOAT )
		*(m_currentVertex.data() + m_attributes[_attrIndex].offset) = _value;
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::INT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<int32*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = (int32)(_value * std::numeric_limits<int32>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::UINT32 )
		*reinterpret_cast<uint32*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = (uint32)(_value * std::numeric_limits<uint32>::max());
	else logError("Vertex attribute ", _attrIndex, " is incompatible with put(float)! The format cannot be casted.");
}

void InstanceRenderer::put(int _attrIndex, const Vec2& _value)
{
	if( m_attributes[_attrIndex].numComponents != 2 )
	{
		logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec2)! The component count is wrong.");
		return;
	}

	if( m_attributes[_attrIndex].type == PrimitiveFormat::FLOAT )
		*reinterpret_cast<Vec2*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = _value;
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::INT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<IVec2*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = IVec2(_value * std::numeric_limits<int32>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::UINT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<UVec2*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = UVec2(_value * std::numeric_limits<uint32>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::INT16 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<uint32*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = uint32(_value.x * std::numeric_limits<int16>::max()) << 16 | uint32(_value.y * std::numeric_limits<int16>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::UINT16 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<uint32*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = uint32(_value.x * std::numeric_limits<uint16>::max()) << 16 | uint32(_value.y * std::numeric_limits<uint16>::max());
	else logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec2)! The format cannot be casted.");
}

void InstanceRenderer::put(int _attrIndex, const ei::Vec3& _value)
{
	if( m_attributes[_attrIndex].numComponents != 3 )
	{
		logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec3)! The component count is wrong.");
		return;
	}

	if( m_attributes[_attrIndex].type == PrimitiveFormat::FLOAT )
		*reinterpret_cast<Vec3*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = _value;
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::INT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<IVec3*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = IVec3(_value * std::numeric_limits<int32>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::UINT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<UVec3*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = UVec3(_value * std::numeric_limits<uint32>::max());
	// TODO allow and convert to R11G11B10F
	else logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec3)! The format cannot be casted.");
}

void InstanceRenderer::put(int _attrIndex, const ei::Vec4& _value)
{
	if( m_attributes[_attrIndex].numComponents != 4 )
	{
		logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec4)! The component count is wrong.");
		return;
	}

	if( m_attributes[_attrIndex].type == PrimitiveFormat::FLOAT )
		*reinterpret_cast<Vec4*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = _value;
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::INT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<IVec4*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = IVec4(_value * std::numeric_limits<int32>::max());
	else if( m_attributes[_attrIndex].type == PrimitiveFormat::UINT32 && m_attributes[_attrIndex].normalize )
		*reinterpret_cast<UVec4*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = UVec4(_value * std::numeric_limits<uint32>::max());
	// TODO INT16 and INT8 normalized formats
	// TODO allow and convert special formats with alpha channel
	else logError("Vertex attribute ", _attrIndex, " is incompatible with put(vec4)! The format cannot be casted.");
}

void InstanceRenderer::put(int _attrIndex, const uint32 _value)
{
	if( !(m_attributes[_attrIndex].type == PrimitiveFormat::INT8 && m_attributes[_attrIndex].numComponents == 4)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::UINT8 && m_attributes[_attrIndex].numComponents == 4)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::INT16 && m_attributes[_attrIndex].numComponents == 2)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::UINT16 && m_attributes[_attrIndex].numComponents == 2)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::INT32 && m_attributes[_attrIndex].numComponents == 1)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::UINT32 && m_attributes[_attrIndex].numComponents == 1)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::R11G11B10F && m_attributes[_attrIndex].numComponents == 1)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::INTR10G10B10A2 && m_attributes[_attrIndex].numComponents == 1)
		&& !(m_attributes[_attrIndex].type == PrimitiveFormat::UINTR10G10B10A2 && m_attributes[_attrIndex].numComponents == 1))
	{
		logError("Vertex attribute ", _attrIndex, " is incompatible with put(uint32)!");
		return;
	}

	*reinterpret_cast<uint32*>(m_currentVertex.data() + m_attributes[_attrIndex].offset) = _value;
}

void InstanceRenderer::emit()
{
	m_vertexData.insert(m_vertexData.end(), m_currentVertex.begin(), m_currentVertex.end());
	++m_meshes.back().vertexCount;
	unsigned index = m_vertexData.size() / m_vertexSize - 1;
	unsigned idx_size = m_indexData.size();
	// If primitive finished add the indices
	if(m_geometryType == GLPrimitiveType::LINES)
	{
		m_indexData.push_back(index);
	} else if(m_geometryType == GLPrimitiveType::LINE_STRIPE)
	{
		// Continue stripe by adding the same index as start index
		if(m_startNewPrimitive == 2)
			m_indexData.push_back(index-1);
		else ++m_startNewPrimitive;
		m_indexData.push_back(index);
	} else if(m_geometryType == GLPrimitiveType::TRIANGLES)
	{
		m_indexData.push_back(index);
	} else if(m_geometryType == GLPrimitiveType::TRIANGLE_STRIPE)
	{
		// Continue stripe by copying the last two indices
		if(m_startNewPrimitive == 3)
		{
			// TODO: face orientation
			m_indexData.push_back(index-1);
			m_indexData.push_back(index-2);
		} else ++m_startNewPrimitive;
		m_indexData.push_back(index);
	}
	m_meshes.back().indexCount += m_indexData.size() - idx_size;
}

void InstanceRenderer::emitVertex()
{
	m_vertexData.insert(m_vertexData.end(), m_currentVertex.begin(), m_currentVertex.end());
	++m_meshes.back().vertexCount;
}

void InstanceRenderer::endPrimitive()
{
	m_startNewPrimitive = 0;
}

void InstanceRenderer::endDef()
{
	//m_meshes.back().vertexCount = (m_vertexData.size() - m_meshes.back().vertexOffset) / m_vertexSize;
	//m_meshes.back().indexCount = m_indexData.size() - (m_meshes.back().indexOffset / 4);

	// TODO: remove redundant vertices
	// TODO: improve cache order

	// Upload entire buffer because its size changed
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
	glCall(glBufferData, GL_ARRAY_BUFFER, m_vertexData.size() * 4, m_vertexData.data(), GL_STATIC_DRAW);

	glCall(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glCall(glBufferData, GL_ELEMENT_ARRAY_BUFFER, m_indexData.size() * 4, m_indexData.data(), GL_STATIC_DRAW);
}

void InstanceRenderer::defTriangle(uint32 _a, uint32 _b, uint32 _c)
{
	uint32 baseVertex = m_meshes.back().vertexOffset / m_vertexSize;
	m_indexData.push_back(_a + baseVertex);
	m_indexData.push_back(_b + baseVertex);
	m_indexData.push_back(_c + baseVertex);
	m_meshes.back().indexCount += 3;
}

void InstanceRenderer::newInstance(int _meshID, const ei::Vec3& _position, const ei::Quaternion& _rotation)
{
	MeshInstance instance;
	instance.rotation = _rotation;
	instance.position = _position;
	// Find the position where to insert, it depends on how many instances are within the other meshes
	int pos = 0;
	for(int i = 0; i <= _meshID; ++i)
		pos += m_meshes[i].numInstances;
	m_instances.insert(m_instances.begin() + pos, instance);
	++m_meshes[_meshID].numInstances;

	m_dirty = true;
}

void InstanceRenderer::clearGeometry()
{
	m_meshes.clear();
	m_vertexData.clear();
	m_indexData.clear();
	m_dirty = true;
}

void InstanceRenderer::clearInstances()
{
	for(int i = 0; i < (int)m_meshes.size(); ++i)
		m_meshes[i].numInstances = 0;
	m_instances.clear();
	m_dirty = true;
}

void InstanceRenderer::draw() const
{
	// Update instance data each frame - it could be dynamic
	if(m_dirty)
	{
		glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vboInstances);
		// TODO: subdata for dynamic stuff?
		glCall(glBufferData, GL_ARRAY_BUFFER, m_instances.size() * sizeof(MeshInstance), m_instances.data(), GL_DYNAMIC_DRAW);
		m_dirty = false;
	}

	glCall(glBindVertexArray, m_vao);
	glCall(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	int instanceOffset = 0;
	for(auto& it : m_meshes)
	{
		if(it.numInstances > 0)
		{
			// TODO: bind material
			glCall(glDrawElementsInstancedBaseInstance,
				unsigned(m_glType),
				it.indexCount,
				GL_UNSIGNED_INT,
				(void*)(uintptr_t)(it.indexOffset),
				it.numInstances,
				instanceOffset);
			instanceOffset += it.numInstances;
		}
	}
}

uint32 InstanceRenderer::getNumVertices(int _meshID) const
{
	return m_meshes[_meshID].vertexCount;
}

uint32 InstanceRenderer::getNumIndices(int _meshID) const
{
	return m_meshes[_meshID].indexCount;
}

}} // namespace ca::cc
