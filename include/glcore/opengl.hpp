#pragma once

#include "core/error.hpp"
#include <gl/glew.h>
#include <type_traits>
#include <string>

namespace ca { namespace cc {

	/// Load OpenGL functions and set enable debug extension (call after context creation)
	//init();
	
	/// Check OpenGL for an error and report it if necessary.
	/// \details This call may stall the CPU/GPU due to glGetError().
	/// \returns true if an error occurred.
	bool GLError(const char*  _openGLFunctionName);

	/// OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto _glCall(const char* _functionName, FunctionType _function, Args... _args) -> typename std::enable_if<!std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		if(!_function) { error(("Function '" + std::string(_functionName) + "' not loaded!").c_str()); return 0; }
		auto ret = _function(_args...);
#ifdef DEBUG
		GLError(_functionName);
#endif
		return ret;
	}

	/// No return overload of OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto _glCall(const char* _functionName, FunctionType _function, Args... _args) -> typename std::enable_if<std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		if(!_function) { error(("Function '" + std::string(_functionName) + "' not loaded!").c_str()); return; }
		_function(_args...);
#ifdef DEBUG
		GLError(_functionName);
#endif
	}
	
#define glCall(_function, ...) _glCall(#_function, _function, __VA_ARGS__)

}} // namespace ca::cc
