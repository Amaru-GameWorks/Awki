#include "Log.h"
#include "Scheduling/JobSystemSynchronization.h"

#include <array>
#include <print>
#include <format>
#include <chrono>
#include <fstream>
#include <filesystem>

#if DEBUG && _MSC_VER
#include <Windows.h>
#endif

void WriteToLogFile(const std::string_view logMessage)
{
	static std::ofstream sLogFile("Awki.log", std::ios::out);
	sLogFile.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	sLogFile << logMessage << std::endl;
}

bool AkLog::Initialize()
{
	try
	{
		if (std::filesystem::exists("Awki.log"))
			std::filesystem::rename("Awki.log", "AwkiLast.log");

		const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		const std::source_location sourceLocation = std::source_location::current();
		
		const std::string filePath = sourceLocation.file_name();
		const std::string fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());
		const std::string logMessage = std::format("[{:%T}][INFO][{}:{}] Awki Log Started", now, fileNameOnly, sourceLocation.line());

		std::println("{}", logMessage);
		WriteToLogFile(logMessage);
		return true;
	}
	catch (const std::exception& exception)
	{
		const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		const std::source_location sourceLocation = std::source_location::current();

		const std::string filePath = sourceLocation.file_name();
		const std::string fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());
		
		std::println("[{:%T}][Error][{}:{}] Failed to initialize output log file: {}!", now, fileNameOnly, sourceLocation.line(), exception.what());
		return false;
	}
}

void AkLog::Deinitialize()
{
	const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	const std::source_location sourceLocation = std::source_location::current();

	const std::string filePath = sourceLocation.file_name();
	const std::string fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());
	const std::string logMessage = std::format("[{:%T}][INFO][{}:{}] Awki Log Ended", now, fileNameOnly, sourceLocation.line());
	
	std::println("{}", logMessage);
	WriteToLogFile(logMessage);
}

static inline AkJob PrintInternal(AkLogLevel logLevel, const std::source_location& sourceLocation, std::string_view message)
{
	static constexpr std::array kLogLevel =
	{
		"INFO",
		"TRACE",
		"WARNING",
		"ERROR",
		"CRITICAL"
	};

	const uint32_t logLevelIndex = static_cast<uint32_t>(logLevel);
	const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	const std::string filePath = sourceLocation.file_name();
	const std::string fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());
	const std::string logMessage = std::format("[{:%T}][{}][{}:{}] {}", now, kLogLevel[logLevelIndex], fileNameOnly, sourceLocation.line(), message);

	static AkTicketMachine sTicketMachine(AkThreadType::Worker);
	co_await sTicketMachine.WaitForTurn();

	std::println("{}", logMessage);
	WriteToLogFile(logMessage);

#if DEBUG && _MSC_VER
	OutputDebugString((logMessage + "\n").c_str());
#endif

	co_return;
}

void AkLog::Print(AkLogLevel logLevel, const std::source_location& sourceLocation, std::string_view message)
{
	PrintInternal(logLevel, sourceLocation, message);
}