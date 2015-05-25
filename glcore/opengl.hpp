#pragma once

namespace MiR {
	
	/// Check OpenGL for an error and report it if necessary.
	/// \details This call may stall the CPU/GPU due to glGetError().
	/// \returns true if an error occurred.
	bool GLError(const char*  _openGLFunctionName);
	
//#define NAME_OF(GLFunction) (#GLFunction)
	
	/// OpenGL call with additional checks.
	template<typename ReturnType, typename T...>
	ReturnType glCall(ReturnType (_function*)(T...), T... _args)
	{
		auto ret = _function(_args);
#ifdef DEBUG
		GLError(#GLFunction);
#endif
		return ret;
	}

	/// No return overload of OpenGL call with additional checks.
	template<typename T...>
	void glCall(void (_function*)(T...), T... _args)
	{
		_function(_args);
#ifdef DEBUG
		GLError(#GLFunction);
#endif
	}
	
} // namespace MiR