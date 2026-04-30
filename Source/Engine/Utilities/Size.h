#pragma once
#include <sstream>
#include <iomanip>

constexpr inline size_t SizeKB(const size_t value)
{
	return value * static_cast<size_t>(1024);
}

constexpr inline size_t SizeMB(const size_t value)
{
	return SizeKB(value) * static_cast<size_t>(1024);
}

constexpr inline size_t SizeGB(const size_t value)
{
	return SizeMB(value) * static_cast<size_t>(1024);
}

inline std::string ByteSizeToShortString(const size_t bytes)
{
	std::stringstream stringStream = {};
	stringStream << std::fixed;
	stringStream << std::setprecision(2);

	if (bytes >= SizeGB(1))
	{
		double sizeInGB = bytes / static_cast<double>(SizeGB(1));
		stringStream << sizeInGB << " GB";
	}
	else if (bytes >= SizeMB(1))
	{
		double sizeInMB = bytes / static_cast<double>(SizeMB(1));
		stringStream << sizeInMB << " MB";
	}
	else if (bytes >= SizeKB(1))
	{
		double sizeInKB = bytes / static_cast<double>(SizeKB(1));
		stringStream << sizeInKB << " KB";
	}
	else
	{
		stringStream << bytes << " B";
	}

	return stringStream.str();
}