#pragma once

#include <ei/vector.hpp>

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
		
		// TODO: declare vertex format
		
		/// Create a new static mesh (e.g. a sprite)
		/// \details This is similar to the OpenGL immediate mode. You can fill the data of a vertex
		///		and emit it. The number of vertices is determined by the primitive type (see ctor).
		///		The put must be called for each attribute (see ctor vertex declaration) in a vertex
		///		and must match the defined type.
		/// \returns The new mesh ID.
		int beginDef();
			void put(int _attrIndex, const ei::Vec2& _value);
			void put(int _attrIndex, const ei::Vec3& _value);
			void put(int _attrIndex, const ei::Vec4& _value);
			void put(int _attrIndex, const ei::uint32 _value);
			/// Go to the next vertex/primitive. Also checks if the vertex is valid.
			void emit();
		/// Upload mesh
		void endDef();
		
		/// \param [in] _position Position in world (x,y) and z for the "layer".
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		void newInstance(int _meshID, const ei::Vec3& _position, float _rotation);
		
		/// Single instanced draw call for all instances.
		void draw() const;
	};

} // namespace MiR