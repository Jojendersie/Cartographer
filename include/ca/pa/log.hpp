#pragma once

#include <string>

#ifndef CA_LOG_LEVEL
#define CA_LOG_LEVEL 1
#endif

namespace ca { namespace pa {
	
	enum class LogSeverity {
		PEDANTIC,
		INFO,
		WARNING,
		ERROR,
		FATAL_ERROR,
	};
	
	typedef void (*MessageHandlerFunc)(LogSeverity _severity, const std::string& _message);
	
	/// Add a function which is called for each occuring message.
	/// \details All message handler functions are called in order of insertion.
	void registerMessageHandler(MessageHandlerFunc _func);
	
	/// Remove the default output to std::cerr. If a message is send when no handler
	/// is registered, the default handler is enabled again.
	void disableStdHandler();
	
	namespace details {
		
		// This one calls all the callbacks
		void logMessage(LogSeverity _severity, std::string& _msg);
		
		// This one builds the message string
		template<typename T, typename... ArgTs>
		void logMessage(LogSeverity _severity, std::string& _msg, T&& _head, ArgTs... _tail)
		{
			_msg += std::to_string(_head);
			logMessage(_severity, _msg, _tail...);
		}

		// Overloads for some types
		template<typename... ArgTs>
		void logMessage(LogSeverity _severity, std::string& _msg, const char* _head, ArgTs... _tail)
		{
			_msg += _head;
			logMessage(_severity, _msg, _tail...);
		}
		template<typename... ArgTs>
		void logMessage(LogSeverity _severity, std::string& _msg, const std::string& _head, ArgTs... _tail)
		{
			_msg += _head;
			logMessage(_severity, _msg, _tail...);
		}
	}

#if CA_LOG_LEVEL <= 0
	template<typename... ArgTs>
	void logPedantic(ArgTs... _args)
	{
		std::string msg;
		msg.reserve(500);
		details::logMessage(LogSeverity::PEDANTIC, msg, _args...);
	}
#else
	template<typename... ArgTs>
	void logPedantic(ArgTs... _args) {}
#endif

#if CA_LOG_LEVEL <= 1
	template<typename... ArgTs>
	void logInfo(ArgTs... _args)
	{
		std::string msg;
		msg.reserve(500);
		details::logMessage(LogSeverity::INFO, msg, _args...);
	}
#else
	template<typename... ArgTs>
	void logInfo(ArgTs... _args) {}
#endif

#if CA_LOG_LEVEL <= 2
	template<typename... ArgTs>
	void logWarning(ArgTs... _args)
	{
		std::string msg;
		msg.reserve(500);
		details::logMessage(LogSeverity::WARNING, msg, _args...);
	}
#else
	template<typename... ArgTs>
	void logWarning(ArgTs... _args) {}
#endif

	template<typename... ArgTs>
	void logError(ArgTs... _args)
	{
		std::string msg;
		msg.reserve(500);
		details::logMessage(LogSeverity::ERROR, msg, _args...);
	}
	
	template<typename... ArgTs>
	void logFatal(ArgTs... _args)
	{
		std::string msg;
		msg.reserve(500);
		details::logMessage(LogSeverity::FATAL_ERROR, msg, _args...);
	}
	
}} // namespace ca::pa