#include "vertexformat.hpp"

namespace MiR {

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

} // namespace MiR