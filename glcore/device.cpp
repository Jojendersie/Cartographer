#include "opengl.hpp"
#include "device.hpp"

namespace MiR {

	BlendOp Device::s_blendOp[8] 				= {};
	BlendFactor Device::s_srcFactor[8] 			= {};
	BlendFactor Device::s_dstFactor[8] 			= {};
	CullMode Device::s_cullMode 				= ;
	CullMode Device::s_fillMode 				= ;
	int Device::s_stencilRef					= ;
	StencilOp Device::s_stencilFailBack			= ;
	StencilOp Device::s_zfailBack				= ;
	StencilOp Device::s_passBack				= ;
	StencilOp Device::s_stencilFailFront		= ;
	StencilOp Device::s_zfailFront 				= ;
	StencilOp Device::s_passFront 				= ;
	ComparisonFunc Device::s_stencilFuncFront	= ;
	ComparisonFunc Device::s_stencilFuncBack	= ;
	ComparisonFunc Device::s_zFunc				= ;
	bool Device::s_zEnable						= ;

	// TODO: glCall(glEnable, GL_BLEND);
	void Device::setBlendOp(BlendOp _operation, int _target = 0)
	{
		if(s_blendOp[_target] != _operation)
		{
			glCall(glBlendEquationi, _target, unsigned(_operation));
			s_blendOp[_target] = _operation;
		}
	}

	void Device::setBlendFactor(BlendFactor _srcFactor, BlendFactor _dstFactor, int _target = 0)
	{
		if(s_srcFactor[_target] != _srcFactor || s_dstFactor[_target] != _dstFactor)
		{
			glCall(glBlendFunci, _target, unsigned(_srcFactor), unsigned(_dstFactor));
			s_srcFactor[_target] = _srcFactor;
			s_dstFactor[_target] = _dstFactor;
		}
	}


				
	void Device::setFillMode(FillMode _mode)
	{
		if(s_fillMode != _mode)
		{
			glCall(glPolygonMode, GL_FRONT_AND_BACK, int(_mode));
			s_fillMode = _mode;
		}
	}

	void Device::setCullMode(CullMode _mode)
	{
		if(s_cullMode != _mode)
		{
			if(_mode == CullMode::NONE)
				glCall(glDisable, GL_CULL_FACE);
			else {
				glCall(glEnable, GL_CULL_FACE);
				glCall(glCullFace, int(_mode));
			}
			s_cullMode = _mode;
		}
	}

} // namespace MiR