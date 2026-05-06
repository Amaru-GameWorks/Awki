#pragma once
#include "Core/Log.h"
#include "Utilities/Macros.h"

#include <format>
#include <string_view>

#define AkRaise(message, ...)					{ AkAssertMessageBox::ShowError(message, ##__VA_ARGS__); AkLogCritical(message, ##__VA_ARGS__); }
#define AkAssert(condition, message, ...)		{ if(!(condition)){ AkAssertMessageBox::ShowError(message, ##__VA_ARGS__); AkLogCritical(message, ##__VA_ARGS__); }}
#define AkSoftAssert(condition, message, ...)	{ if(!(condition)){ [[maybe_unused]] static auto COUNTER_CONCAT(AkSoftAsertUsed) = [](){ AkAssertMessageBox::ShowWarning(message, ##__VA_ARGS__); DEBUG_BREAK(); return 0; }(); AkLogWarning(message, ##__VA_ARGS__); }}

class AkAssertMessageBox
{
public:

	template<typename ...Arguments>
	static void ShowError(const std::format_string<Arguments...>& format, Arguments&&... arguments)
	{
		std::string message = std::format(format, std::forward<Arguments>(arguments)...);
		ShowError(message);
	}

	template<typename ...Arguments>
	static void ShowWarning(const std::format_string<Arguments...>& format, Arguments&&... arguments)
	{
		std::string message = std::format(format, std::forward<Arguments>(arguments)...);
		ShowWarning(message);
	}

	static void ShowError(const std::string_view message);
	static void ShowWarning(const std::string_view message);
};