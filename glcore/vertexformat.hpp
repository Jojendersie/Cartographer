#pragma once

#include <gl/glew.h>

namespace ca { namespace cc {

	enum struct PrimitiveFormat
	{
		FLOAT = GL_FLOAT,
		R11G11B10F = GL_UNSIGNED_INT_10F_11F_11F_REV,
		INT8 = GL_BYTE,
		UINT8 = GL_UNSIGNED_BYTE,
		INT16 = GL_SHORT,
		UINT16 = GL_UNSIGNED_SHORT,
		INT32 = GL_INT,
		UINT32 = GL_UNSIGNED_INT,
		INTR10G10B10A2 = GL_INT_2_10_10_10_REV,
		UINTR10G10B10A2 = GL_UNSIGNED_INT_2_10_10_10_REV,
		HALF = GL_HALF_FLOAT
	};

	/// Check if glVertexAttribIPointer must be used
	bool isIntegerFormat(PrimitiveFormat _format);
	/// Check if glVertexAttribPointer must be used
	bool isFloatFormat(PrimitiveFormat _format);

	// An array of this attributes define a vertex.
	struct VertexAttribute
	{
		PrimitiveFormat type;
		int numComponents;	///< 1 to 4
		bool normalize;		///< Normalize integer values to [0,1] or [-1,1]
	};

	/// Compute the size of a single attribute.
	int attributeSize(const VertexAttribute& _attr);

	enum struct GLPrimitiveType
	{
		POINTS = GL_POINTS,
		LINES = GL_LINES,
		TRIANGLES = GL_TRIANGLES,
	};

}} // namespace ca::cc
