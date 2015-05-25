#pragma once

#include <gl/glew.h>

enum class BlendOp
{
	ADD = GL_FUNC_ADD,
	SUBTRACT = GL_FUNC_SUBTRACT,
	REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
	MIN = GL_MIN,
	MAX = GL_MAX
};

enum class BlendFactor {
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_ALPHA = GL_SRC_ALPHA,
	INV_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	INV_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
	SRC_COLOR = GL_SRC_COLOR,
	INV_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	DST_COLOR = GL_DST_COLOR,
	INV_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	// TODO Color
};

enum class CullMode
{
	FRONT = GL_FRONT,
	BACK = GL_BACK,
	NONE
};

enum class FillMode
{
	SOLID = GL_FILL,
	WIREFRAME = GL_LINE
};

enum struct ComparisonFunc {
	NEVER = GL_NEVER,		///< Fail always
	LESS = GL_LESS,			///< Comparison function less '<'
	EQUAL = GL_EQUAL,		///< Comparison function equal '=='
	LEQUAL = GL_LEQUAL,		///< Comparison function less equal '<='
	GREATER = GL_GREATER,	///< Comparison function greater '>'
	GEQUAL = GL_GEQUAL,		///< Comparison function greater equal '>='
	NOTEQUAL = GL_NOTEQUAL,	///< Comparison function not equal '!='
	ALWAYS = GL_ALWAYS		///< Disables z-Testing / stencil test
};

enum struct StencilOp {
	KEEP = GL_KEEP,
	ZERO = GL_ZERO,
	REPLACE = GL_REPLACE,
	INC_WARP = GL_INCR_WRAP,
	DEC_WARP = GL_DECR_WRAP,
	INC_SAT = GL_INCR,
	DEC_SAT = GL_DECR,
	INVERT = GL_INVERT,
};

// Window and state machine.
class Device
{
public:
	/// Creates the one window (only one call allowed)
	static void createWindow(int _width, int _height, bool _fullScreen);
	
	// Optimized state changes (only calls gl.. if necessary)
	/// \param _operation A valid blend operation.
	/// \param _target The render target when MRT are used.
	static void setBlendOp(BlendOp _operation, int _target = 0);
	static void setBlendFactor(BlendFactor _srcFactor, BlendFactor _dstFactor, int _target = 0);
	static void setBlendColor();
	
	static void setFillMode(FillMode _mode);
	static void setCullMode(CullMode _mode);
	
	static void setStencilReference(int _value);
	static void setStencilOp(StencilOp _stencilFailBack, StencilOp _zfailBack, StencilOp _passBack,
							 StencilOp _stencilFailFront, StencilOp _zfailFront, StencilOp _passFront);
	static void setStencilFunc(ComparisonFunc _funcFront, ComparisonFunc _funcBack);
	static void setZFunc(ComparisonFunc _zFunc);
	static void setZWrite(bool _enable);
	
private:
	static BlendOp s_blendOp[8];
	static BlendFactor s_srcFactor[8], s_dstFactor[8];
	static CullMode s_cullMode;
	static FillMode s_fillMode;
	static int s_stencilRef;
	static StencilOp s_stencilFailBack, s_zfailBack, s_passBack;
	static StencilOp s_stencilFailFront, s_zfailFront, s_passFront;
	static ComparisonFunc s_stencilFuncFront, s_stencilFuncBack;
	static ComparisonFunc s_zFunc;
	static bool s_zEnable;
};