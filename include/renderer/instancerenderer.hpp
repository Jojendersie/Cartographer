#pragma once

#include "glcore/vertexformat.hpp"

#include <ei/vector.hpp>
#include <vector>

namespace ca { namespace cc {
	
	/// Batch renderer for models.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class InstanceRenderer
	{
	public:
		/// Creation determines the vertex format and primitive type.
		InstanceRenderer(GLPrimitiveType _type, const VertexAttribute* _attributes, int _numAttributes);
		
		/// Create a new static mesh (e.g. a sprite)
		/// \details This is similar to the OpenGL immediate mode. You can fill the data of a vertex
		///		and emit it. The number of vertices is determined by the primitive type (see ctor).
		///		The put must be called for each attribute (see ctor vertex declaration) in a vertex
		///		and must match the defined type in order type and size.
		/// \returns The new mesh ID.
		int beginDef();
			void put(int _attrIndex, const float _value);
			void put(int _attrIndex, const ei::Vec2& _value);
			void put(int _attrIndex, const ei::Vec3& _value);
			void put(int _attrIndex, const ei::Vec4& _value);
			void put(int _attrIndex, const uint32 _value);
			/// Go to the next vertex/primitive. Also checks if the vertex is valid.
			void emit();
			/// Finish the primitive in case of LINE_STRIPE and TRIANGLE_STRIPE
			void endPrimitive();
		/// Upload mesh
		void endDef();
		
		/// \param [in] _position Position in world (x,y) and z for the "layer".
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		void newInstance(int _meshID, const ei::Vec3& _position, const ei::Quaternion& _rotation);

		//void setInstance()

		/// Clear all existing instances (recommended for fully dynamic buffers)
		void clearInstances();
		
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

		struct AttributeDefinition
		{
			PrimitiveFormat type;
			int numComponents;
			int offset;
			bool normalize;
		};

		struct MeshInstance
		{
			ei::Quaternion rotation;
			ei::Vec3 position;
		};

		unsigned m_vao;		///< OpenGL vertex array object
		unsigned m_vbo;		///< OpenGL vertex buffer object
		unsigned m_ibo;		///< OpenGL index buffer object with 32bit indices
		unsigned m_vertexSize;		///< Number of 32bit words per vertex, computed from vertex declaration
		unsigned m_vboInstances;	///< Instance data
		// This renderer does need types without stripe-mode to be able to separate geometry.
		GLPrimitiveType m_geometryType;	///< Type for geometry generation.
		GLPrimitiveType m_glType;	///< OpenGL primitive type.
		unsigned m_numIndices;	
		unsigned m_numVertices;
		std::vector<AttributeDefinition> m_attributes;
		std::vector<MeshDefinition> m_meshes;
		std::vector<MeshInstance> m_instances;
		std::vector<float> m_vertexData;
		std::vector<uint32> m_indexData;
		std::vector<float> m_currentVertex;
		int m_startNewPrimitive;	///< Used to control indexing of stripe typed geometry
		mutable bool m_dirty;		///< Is the instace buffer dirty?
	};

}} // namespace ca::cc
