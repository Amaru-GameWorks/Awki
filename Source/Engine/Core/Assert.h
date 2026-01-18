#pragma once
#include "Core/Log.h"
#include "Utilities/Macros.h"

#include <string_view>

#define AkRaise(message)					{ AkAssertMessageBox::ShowError(message); AkLogCritical(message); }
#define AkAssert(condition, message)		{ if(!(condition)){ AkAssertMessageBox::ShowError(message); AkLogCritical(message); }}
#define AkSoftAssert(condition, message)	{ if(!(condition)){ [[maybe_unused]] static auto COUNTER_CONCAT(AkSoftAsertUsed) = [](){ AkAssertMessageBox::ShowWarning(message); DEBUG_BREAK(); return 0; }(); AkLogWarning(message); }}

class AkAssertMessageBox
{
public:
	static void ShowError(const std::string_view message);
	static void ShowWarning(const std::string_view message);
};