#pragma once

#include <gl/glew.h>
#include <type_traits>

namespace MiR {
	
	/// Check OpenGL for an error and report it if necessary.
	/// \details This call may stall the CPU/GPU due to glGetError().
	/// \returns true if an error occurred.
	bool GLError(const char*  _openGLFunctionName);
	
#define NAME_OF(GLFunction) (#GLFunction)

	/// OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto glCall(FunctionType _function, Args... _args) -> typename std::enable_if<!std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		auto ret = _function(_args...);
#ifdef DEBUG
		GLError(NAME_OF(GLFunction));
#endif
		return ret;
	}

	/// No return overload of OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto glCall(FunctionType _function, Args... _args) -> typename std::enable_if<std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		_function(_args...);
#ifdef DEBUG
		GLError(NAME_OF(GLFunction));
#endif
	}
	
} // namespace MiR