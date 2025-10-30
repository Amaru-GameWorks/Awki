#pragma once

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