#pragma once
#include "Platform/Interop.h"

#include <fmt/format.h>
#include <string_view>
#include <source_location>

#define AkLogInfo(format, ...)		{ AkLog::AddLogEntry(LogLevel::INFO, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogTrace(format, ...)		{ AkLog::AddLogEntry(LogLevel::TRACE, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogWarning(format, ...)	{ AkLog::AddLogEntry(LogLevel::WARNING, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogError(format, ...)		{ AkLog::AddLogEntry(LogLevel::ERROR, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogCritical(format, ...)	{ AkLog::AddLogEntry(LogLevel::CRITICAL, std::source_location::current(), format , ##__VA_ARGS__); DEBUG_BREAK(); }

enum class LogLevel
{
	INFO,
	TRACE,
	WARNING,
	ERROR,
	CRITICAL
};

class AkLog
{
public:
	static bool Initialize();
	static void Deinitialize();

	template<typename ...Arguments>
	static void AddLogEntry(LogLevel logLevel, const std::source_location& sourceLocation, const fmt::format_string<Arguments...>& format, Arguments&&... arguments)
	{
		std::string message = fmt::format(format, std::forward<Arguments>(arguments)...);
		Print(logLevel, sourceLocation, message);
	}

private:
	static void Print(LogLevel logLevel, const std::source_location& sourceLocation, std::string_view message);
};