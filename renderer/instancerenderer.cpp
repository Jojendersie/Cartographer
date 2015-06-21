#include "instancerenderer.hpp"
#include "glcore/opengl.hpp"

namespace MiR {

InstanceRenderer::InstanceRenderer(PrimitiveType _type, const VertexAttribute* _attributes, int _numAttributes)
{
	glCall(glGenVertexArrays, 1, &m_vao);
	glCall(glBindVertexArray, m_vao);

	// Create buffer without data for now
	glCall(glGenBuffers, 1, &m_vbo);
	glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

	// Define the vertex format
	for(int i = 0; i < _numAttributes; ++i)
	{
		glCall(glEnableVertexAttribArray, i);
		if( isFloatFormat(_attributes[i].type) || _attributes[i].normalize )
			glCall(glVertexAttribPointer, unsigned(i), _attributes[i].numComponents, GLenum(_attributes[i].type), GLboolean(_attributes[i].normalize), 0, nullptr);
		else
			glCall(glVertexAttribIPointer, unsigned(i), _attributes[i].numComponents, GLenum(_attributes[i].type), 0, nullptr);
	}
}

void InstanceRenderer::draw() const
{
	for(auto& it : m_instances)
	{
		//glBindBufferBase();//??
		glCall(glDrawElementsInstanced,
			unsigned(m_type),
			it.indexCount,
			GL_UNSIGNED_INT,
			(void*)it.indexOffset,
			it.numInstances);
	}
}

} // namespace MiR