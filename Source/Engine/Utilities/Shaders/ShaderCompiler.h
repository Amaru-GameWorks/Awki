#pragma once
#include "Utilities/ForwardStorage.h"

#include <slang/slang.h>

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

class AkShaderByteCode
{
public:
	AkShaderByteCode();
	~AkShaderByteCode();

	size_t GetSize() const;
	const uint8_t* GetByteCode() const;

	operator bool() const;

private:
	friend class AkShaderCompiler;
	ForwardStorage<struct AkShaderByteCodeStorage, 8> m_Storage;
};

class AkShaderCompiler
{
public:
	AkShaderCompiler();
	~AkShaderCompiler();

	AkShaderByteCode CompileShader(const AkShaderCompileOptions& compileOptions);

private:
	ForwardStorage<struct AkShaderCompilerStorage, 16> m_Storage;
};