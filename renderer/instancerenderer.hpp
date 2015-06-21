#pragma once

#include "glcore/vertexformat.hpp"

#include <ei/vector.hpp>
#include <vector>

namespace MiR {
	
	/// Batch renderer for models.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class InstanceRenderer
	{
	public:
		enum class PrimitiveType
		{
			POINTS,
			LINES,
			TRIANGLES,
			QUADS
		};

		/// Creation determines the vertex format and primitive type.
		InstanceRenderer(PrimitiveType _type, const VertexAttribute* _attributes, int _numAttributes);
		
		/// Create a new static mesh (e.g. a sprite)
		/// \details This is similar to the OpenGL immediate mode. You can fill the data of a vertex
		///		and emit it. The number of vertices is determined by the primitive type (see ctor).
		///		The put must be called for each attribute (see ctor vertex declaration) in a vertex
		///		and must match the defined type in order type and size.
		/// \returns The new mesh ID.
		int beginDef();
			void put(int _attrIndex, const ei::Vec2& _value);
			void put(int _attrIndex, const ei::Vec3& _value);
			void put(int _attrIndex, const ei::Vec4& _value);
			void put(int _attrIndex, const uint32 _value);
			/// Go to the next vertex/primitive. Also checks if the vertex is valid.
			void emit();
		/// Upload mesh
		void endDef();
		
		/// \param [in] _position Position in world (x,y) and z for the "layer".
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		void newInstance(int _meshID, const ei::Vec3& _position, float _rotation);
		
		/// Single instanced draw call for all instances.
		void draw() const;

	private:
		struct MeshDefinition
		{
			unsigned vertexOffset;
			unsigned indexOffset;		///< Offset in the index buffer in bytes
			unsigned vertexCount;
			unsigned indexCount;		///< Number of indices in this instance
			unsigned numInstances;
		};

		unsigned m_vao;		///< OpenGL vertex array object
		unsigned m_vbo;		///< OpenGL vertex buffer object
		unsigned m_ibo;		///< OpenGL index buffer object with 32bit indices
		unsigned m_vertexSize;	///< Computed from vertex declaration
		GLPrimitiveType m_type;	///< OpenGL primitive type
		unsigned m_numIndices;	
		unsigned m_numVertices;
		std::vector<MeshDefinition> m_instances;
	};

} // namespace MiR