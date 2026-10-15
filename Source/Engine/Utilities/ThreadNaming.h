#pragma once

#include <string>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#undef DELETE
#elif defined(__APPLE__)
#include <pthread.h>
#elif defined(__linux__)
#include <pthread.h>
#include <sys/prctl.h>
#endif

inline void SetThreadName(std::string_view name)
{
#if defined(_WIN32)
	std::wstring wname(name.begin(), name.end());
	SetThreadDescription(GetCurrentThread(), wname.c_str());
#elif defined(__APPLE__)
	pthread_setname_np(name.data());
#elif defined(__linux__)
	prctl(PR_SET_NAME, name.data(), 0, 0, 0);
#endif
}