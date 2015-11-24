#pragma once

namespace ca { namespace gui {
	
	/// Set a new error handler. The default is std::cerr.
	/// \details The error handler should insert line-breaks on its own.
	void setErrorCallback(void (*_onError)(const char* _message));
	
	/// Use the error callback. You don't need line breaks.
	void error(const char* _message);
		
}} // namespace ca::gui
