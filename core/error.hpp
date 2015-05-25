#pragma once

namespace Ca {
	
	/// Set a new error handler. The default is std::cerr.
	void setErrorCallback(void (_onError*)(const char* _message));
	
	/// Use the error callback.
	void error(const char* _message);
		
} // namespace Ca