#include "ShaderCompiler.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <array>

static constexpr int32_t kSlangShiftKindUnorderedAccess = 0;
static constexpr int32_t kSlangShiftKindSampler = 1;
static constexpr int32_t kSlangShiftKindShaderResource = 2;

struct AkShaderByteCodeStorage
{
	Slang::ComPtr<slang::IBlob> byteCode = {};
};

struct AkShaderCompilerStorage
{
	Slang::ComPtr<slang::IGlobalSession> globalSession = {};
	Slang::ComPtr<slang::ISession> session = {};
};

AkShaderCompiler::AkShaderCompiler()
{
	slang::createGlobalSession(m_Storage->globalSession.writeRef());
	if (!m_Storage->globalSession)
		throw std::runtime_error("Failed to initialize Slang global session!");

	static constexpr std::array kCompilerOptions = std::to_array<slang::CompilerOptionEntry>
	({
		{ slang::CompilerOptionName::EmitSpirvMethod,		{ slang::CompilerOptionValueKind::Int, SLANG_EMIT_SPIRV_DIRECTLY }},
		{ slang::CompilerOptionName::Optimization,			{ slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_MAXIMAL }},
		{ slang::CompilerOptionName::VulkanInvertY,			{ slang::CompilerOptionValueKind::Int, 1 }},
		{ slang::CompilerOptionName::ForceCLayout,			{ slang::CompilerOptionValueKind::Int, 1 }},
		{ slang::CompilerOptionName::VulkanBindShiftAll,	{ slang::CompilerOptionValueKind::Int, kSlangShiftKindShaderResource, 32 }},
		{ slang::CompilerOptionName::VulkanBindShiftAll,	{ slang::CompilerOptionValueKind::Int, kSlangShiftKindUnorderedAccess, 64 }},
		{ slang::CompilerOptionName::VulkanBindShiftAll,	{ slang::CompilerOptionValueKind::Int, kSlangShiftKindSampler, 96 }}
	});

	static constexpr std::array kSearchPaths = std::to_array<const char*>
	({
		"Resources/Shaders",
		"Resources/Shaders/Modules"
	});

	const slang::TargetDesc targetDescription =
	{
		.format = SLANG_SPIRV,
		.profile = m_Storage->globalSession->findProfile("spirv_1_6")
	};

	const slang::SessionDesc sessionDescription =
	{
		.targets = &targetDescription,
		.targetCount = 1,
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
		.searchPaths = kSearchPaths.data(),
		.searchPathCount = static_cast<uint32_t>(kSearchPaths.size()),
		.compilerOptionEntries = const_cast<slang::CompilerOptionEntry*>(kCompilerOptions.data()),
		.compilerOptionEntryCount = static_cast<uint32_t>(kCompilerOptions.size())
	};

	m_Storage->globalSession->createSession(sessionDescription, m_Storage->session.writeRef());

	if (!m_Storage->session)
		throw std::runtime_error("Failed to create slang session!");
}

AkShaderCompiler::~AkShaderCompiler()
{
}

AkShaderByteCode AkShaderCompiler::CompileShader(const AkShaderCompileOptions& compileOptions)
{
	Slang::ComPtr<slang::IBlob> diagnosticsBlob;
	slang::IModule* mainModule = m_Storage->session->loadModuleFromSource("Main", compileOptions.path.string().c_str(), nullptr, diagnosticsBlob.writeRef());

	if (!mainModule && diagnosticsBlob)
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	std::vector<slang::IComponentType*> componentTypes;
	componentTypes.push_back(mainModule);

	for (const auto&[name, path] : compileOptions.modules)
	{
		slang::IModule* module = m_Storage->session->loadModuleFromSource(name.c_str(), path.string().c_str(), nullptr, diagnosticsBlob.writeRef());

		if (!module && diagnosticsBlob)
		{
			const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
			throw std::runtime_error(errorMessage);
		}

		componentTypes.push_back(module);
	}

	Slang::ComPtr<slang::IComponentType> composedProgram = {};
	SlangResult result = m_Storage->session->createCompositeComponentType(componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnosticsBlob.writeRef());

	if (SLANG_FAILED(result))
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	Slang::ComPtr<slang::IComponentType> linkedProgram = {};
	result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());

	if (SLANG_FAILED(result))
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	Slang::ComPtr<slang::IBlob> compiledCode = {};
	result = linkedProgram->getTargetCode(0, compiledCode.writeRef(), diagnosticsBlob.writeRef());

	if (SLANG_FAILED(result))
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	AkShaderByteCode byteCode = {};
	compiledCode.swap(byteCode.m_Storage->byteCode);
	return byteCode;
}

AkShaderByteCode::AkShaderByteCode()
{ }

AkShaderByteCode::~AkShaderByteCode()
{ }

size_t AkShaderByteCode::GetSize() const
{
	return m_Storage->byteCode->getBufferSize();
}

const uint8_t* AkShaderByteCode::GetByteCode() const
{
	return static_cast<const uint8_t*>(m_Storage->byteCode->getBufferPointer());
}

AkShaderByteCode::operator bool() const
{
	return m_Storage->byteCode;
}