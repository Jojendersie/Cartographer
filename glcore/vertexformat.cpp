#include "vertexformat.hpp"

namespace ca { namespace cc {

bool isIntegerFormat(PrimitiveFormat _format)
{
	return _format == PrimitiveFormat::INT8
		|| _format == PrimitiveFormat::INT16
		|| _format == PrimitiveFormat::INT32
		|| _format == PrimitiveFormat::UINT8
		|| _format == PrimitiveFormat::UINT16
		|| _format == PrimitiveFormat::UINT32;
}

bool isFloatFormat(PrimitiveFormat _format)
{
	return _format == PrimitiveFormat::FLOAT
		|| _format == PrimitiveFormat::INTR10G10B10A2
		|| _format == PrimitiveFormat::R11G11B10F
		|| _format == PrimitiveFormat::UINTR10G10B10A2;
}

int attributeSize(const VertexAttribute& _attr)
{
	switch(_attr.type)
	{
		case PrimitiveFormat::FLOAT: return 4 * _attr.numComponents; break;
		case PrimitiveFormat::R11G11B10F: return 4; break;
		case PrimitiveFormat::INT8: return _attr.numComponents; break;
		case PrimitiveFormat::UINT8: return _attr.numComponents; break;
		case PrimitiveFormat::INT16: return 2 * _attr.numComponents; break;
		case PrimitiveFormat::UINT16: return 2 * _attr.numComponents; break;
		case PrimitiveFormat::INT32: return 4 * _attr.numComponents; break;
		case PrimitiveFormat::UINT32: return 4 * _attr.numComponents; break;
		case PrimitiveFormat::INTR10G10B10A2: return 4; break;
		case PrimitiveFormat::UINTR10G10B10A2: return 4; break;
	}
	return -1;
}

}} // namespace ca::cc
