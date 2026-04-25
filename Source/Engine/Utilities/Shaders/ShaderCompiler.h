#pragma once
#include "Utilities/ForwardStorage.h"

#include <vector>
#include <filesystem>

struct AkShaderModuleInfo
{
	std::string name;
	std::filesystem::path path;
};

struct AkShaderCompileOptions
{
	std::filesystem::path path = {};
	std::vector<AkShaderModuleInfo> modules = {};
};

class AkShaderData
{
public:
	AkShaderData();
	~AkShaderData();

	size_t GetByteCodeSize() const;
	const uint8_t* GetByteCode() const;
	const struct AkShaderReflection& GetReflection() const;

	operator bool() const;

private:
	friend class AkShaderCompiler;
	ForwardStorage<struct AkShaderByteCodeStorage, 96> m_Storage;
};

class AkShaderCompiler
{
public:
	AkShaderCompiler();
	~AkShaderCompiler();

	AkShaderData CompileShader(const AkShaderCompileOptions& compileOptions);

private:
	ForwardStorage<struct AkShaderCompilerStorage, 16> m_Storage;
};