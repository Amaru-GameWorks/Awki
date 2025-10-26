#include "Log.h"

#include <fmt/os.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

#include <filesystem>

fmt::ostream& GetOutputStream()
{
	static fmt::ostream sLogFile = fmt::output_file("Awki.log");
	return sLogFile;
}

bool AkLog::Initialize()
{
	try
	{
		if (std::filesystem::exists("Awki.log"))
			std::filesystem::rename("Awki.log", "AwkiLast.log");

		fmt::ostream& logFile = GetOutputStream();
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

		const std::source_location sourceLocation = std::source_location::current();
		const std::string_view filePath = sourceLocation.file_name();
		const std::string_view fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());

		logFile.print("[{:%T}][INFO][{}:{}] Awki AkLog Started\n", now, fileNameOnly, sourceLocation.line());
		fmt::print(fg(fmt::color::cornflower_blue), "[{:%T}][INFO][{}:{}] Awki AkLog Started\n", now, fileNameOnly, sourceLocation.line());
		return true;
	}
	catch (const std::system_error& error)
	{
		const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold, "[{:%T}][Error] Failed to initialize output log file: {}!\n", now, error.what());
		return false;
	}
}

void AkLog::Deinitialize()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	const std::source_location sourceLocation = std::source_location::current();
	const std::string_view filePath = sourceLocation.file_name();
	const std::string_view fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());

	fmt::ostream& logFile = GetOutputStream();
	logFile.print("[{:%T}][INFO][{}:{}] Awki AkLog Ended\n", now, fileNameOnly, sourceLocation.line());
	fmt::print(fg(fmt::color::cornflower_blue), "[{:%T}][INFO][{}:{}] Awki AkLog Ended\n", now, fileNameOnly, sourceLocation.line());
}

void AkLog::Print(LogLevel logLevel, const std::source_location& sourceLocation, std::string_view message)
{
	constexpr fmt::text_style kTextStyles[5] =
	{
		fg(fmt::color::cornflower_blue),						//INFO
		fg(fmt::color::white),									//TRACE
		fg(fmt::color::yellow), 								//WARNING
		fg(fmt::color::indian_red) | fmt::emphasis::bold,		//ERROR
		fg(fmt::color::dark_red) | bg(fmt::color::white_smoke)	//CRITICAL
	};

	constexpr const char* kLogLevel[5] =
	{
		"INFO",
		"TRACE",
		"WARNING",
		"ERROR",
		"CRITICAL"
	};

	static fmt::ostream& sLogFile = GetOutputStream();
	const uint32_t logLevelIndex = static_cast<uint32_t>(logLevel);
	const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	
	const std::string_view filePath = sourceLocation.file_name();
	const std::string_view fileNameOnly = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.size());
	const std::string fullMessage = fmt::format("[{:%T}][{}][{}:{}] {}", now, kLogLevel[logLevelIndex], fileNameOnly, sourceLocation.line(), message);

	sLogFile.print("{}\n", fullMessage);
	fmt::print(kTextStyles[logLevelIndex], "{}\n", fullMessage);
}