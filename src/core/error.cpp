#include "core/error.hpp"
#include <iostream>

namespace ca { namespace cc {
	
	void defaultHandler(const char* _message)
	{
		std::cerr << _message << std::endl;
	}
	
	static void (*s_onError)(const char* _message) = defaultHandler;
	
	void setErrorCallback(void (*_onError)(const char* _message))
	{
		s_onError("Switching error handler.");
		s_onError = _onError;
	}
	
	void error(const char* _message)
	{
		s_onError(_message);
	}
		
}} // namespace ca::cc
