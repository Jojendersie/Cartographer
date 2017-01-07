#pragma once

#include "ca/cc/utilities/half.hpp"
#include "ca/cc/glcore/geometrybuffer.hpp"

#include <ei/vector.hpp>
#include <vector>

namespace ca { namespace cc {

	/// Batch renderer for polylines.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class LineRenderer
	{
	public:
		/// Initialize the renderer once.
		LineRenderer();

		void beginLine();
			void putVertex(const ei::Vec3& _position, const ei::Vec4& _color);
		void endLine();

		/// Clear all existing lines (recommended for fully dynamic buffers)
		void clearLines();
		
		/// Single draw call for all lines.
		void draw() const;

		/// Check if there are any instances to draw
		bool isEmpty() const { return m_lineData.empty(); }

	private:
		struct LineVertex
		{
			ei::Vec3 position;
			uint32 color;
		};

		mutable GeometryBuffer m_GPUBuffer;
		std::vector<LineVertex> m_lineData;
		std::vector<uint32> m_lineIndices;
		mutable bool m_dirty;
		int m_lineVertexCount;	// Number of vertices since last beginLine()
	};

}} // namespace ca::cc
