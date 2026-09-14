#pragma once
#include "Utilities/PrintFormat/GLM.h"

#include <format>
#include <cstdlib>
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

constexpr void UnsafeExit()
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
#endif

	std::terminate();

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
}

#define AkLogInfo(format, ...)		{ AkLog::AddLogEntry(AkLogLevel::INFO, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogTrace(format, ...)		{ AkLog::AddLogEntry(AkLogLevel::TRACE, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogWarning(format, ...)	{ AkLog::AddLogEntry(AkLogLevel::WARNING, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogError(format, ...)		{ AkLog::AddLogEntry(AkLogLevel::ERROR, std::source_location::current(), format , ##__VA_ARGS__); }
#define AkLogCritical(format, ...)	{ AkLog::AddLogEntry(AkLogLevel::CRITICAL, std::source_location::current(), format , ##__VA_ARGS__); DEBUG_BREAK(); UnsafeExit(); }

enum class AkLogLevel : uint8_t
{
	INFO,
	TRACE,
	WARNING,
	ERROR,
	CRITICAL
};

class AkLog
{
private:
	friend class Awki;
	static bool Initialize();
	static void Deinitialize();

public:
	template<typename ...Arguments>
	static void AddLogEntry(AkLogLevel logLevel, const std::source_location& sourceLocation, const std::format_string<Arguments...>& format, Arguments&&... arguments)
	{
		std::string message = std::format(format, std::forward<Arguments>(arguments)...);
		Print(logLevel, sourceLocation, message);
	}

private:
	static void Print(AkLogLevel logLevel, const std::source_location& sourceLocation, std::string_view message);
};