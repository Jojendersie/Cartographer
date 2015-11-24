#pragma once

namespace ca { namespace cc {

	enum struct PrimitiveFormat
	{
		FLOAT = 0x1406,				///< GL_FLOAT
		R11G11B10F = 0x8C3B,		///< GL_UNSIGNED_INT_10F_11F_11F_REV
		INT8 = 0x1400,				///< GL_BYTE
		UINT8 = 0x1401,				///< GL_UNSIGNED_BYTE
		INT16 = 0x1402,				///< GL_SHORT
		UINT16 = 0x1403,			///< GL_UNSIGNED_SHORT
		INT32 = 0x1404,				///< GL_INT
		UINT32 = 0x1405,			///< GL_UNSIGNED_INT
		INTR10G10B10A2 = 0x8D9F,	///< GL_INT_2_10_10_10_REV
		UINTR10G10B10A2 = 0x8368,	///< GL_UNSIGNED_INT_2_10_10_10_REV
		HALF = 0x140B,				///< GL_HALF_FLOAT
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
		POINTS = 0x0000,	///< GL_POINTS
		LINES = 0x0001,		///< GL_LINES
		TRIANGLES = 0x0004,	///< GL_TRIANGLES
	};

}} // namespace ca::cc
