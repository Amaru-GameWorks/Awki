#pragma once
#include <sstream>

template<typename T>
constexpr inline T SizeKB(const T value)
{
	return value * static_cast<T>(1024);
}

template<typename T>
constexpr inline T SizeMB(const T value)
{
	return SizeKB<T>(value) * static_cast<T>(1024);
}

template<typename T>
constexpr inline T SizeGB(const T value)
{
	return SizeMB<T>(value) * static_cast<T>(1024);
}

template<typename T>
static std::string ByteSizeToShortString(const T bytes)
{
	std::stringstream stringStream = {};
	stringStream << std::fixed;
	stringStream << std::setprecision(2);

	if (bytes >= SizeGB<T>(1))
	{
		double sizeInGB = bytes / SizeGB<double>(1);
		stringStream << sizeInGB << " GB";
	}
	else if (bytes >= SizeMB<T>(1))
	{
		double sizeInMB = bytes / SizeMB<double>(1);
		stringStream << sizeInMB << " MB";
	}
	else if (bytes >= SizeKB<T>(1))
	{
		double sizeInKB = bytes / SizeKB<double>(1);
		stringStream << sizeInKB << " KB";
	}
	else
	{
		stringStream << bytes << " B";
	}

	return stringStream.str();
}