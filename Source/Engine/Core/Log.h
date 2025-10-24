#pragma once
#include <fmt/format.h>

#include <string>
#include <string_view>
#include <source_location>

#if DEBUG
#	ifdef _MSC_VER
#		define DEBUG_BREAK() __debugbreak()
#	elif defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
#		define DEBUG_BREAK() __builtin_debugtrap()
#	else
#		include <signal.h>
#		define DEBUG_BREAK() raise(SIGTRAP)
#	endif
#else
#	define DEBUG_BREAK()
#endif

#define AkLogInfo(format, ...)		{ Log::AddLogEntry(LogLevel::INFO, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogTrace(format, ...)		{ Log::AddLogEntry(LogLevel::TRACE, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogWarning(format, ...)	{ Log::AddLogEntry(LogLevel::WARNING, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogError(format, ...)		{ Log::AddLogEntry(LogLevel::ERROR, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogCritical(format, ...)	{ Log::AddLogEntry(LogLevel::CRITICAL, std::source_location::current(), format , ##__VA_ARGS__); DEBUG_BREAK(); }

enum class LogLevel
{
	INFO,
	TRACE,
	WARNING,
	ERROR,
	CRITICAL
};

class Log
{
public:
	static bool Initialize();

	template<typename ...Arguments>
	static void AddLogEntry(LogLevel logLevel, const std::source_location& sourceLocation, const fmt::format_string<Arguments...>& format, Arguments&&... arguments)
	{
		std::string message = fmt::format(format, arguments...);
		Print(logLevel, sourceLocation, message);
	}

private:
	static void Print(LogLevel logLevel, const std::source_location& sourceLocation, std::string_view message);
};