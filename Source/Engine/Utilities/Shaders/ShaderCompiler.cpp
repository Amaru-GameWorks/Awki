#include "ShaderCompiler.h"
#include "Core/Log.h"
#include "RHI/Pipeline/Shader.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <array>
#include <cstring>

static constexpr int32_t kSlangShiftKindUnorderedAccess = 0;
static constexpr int32_t kSlangShiftKindSampler = 1;
static constexpr int32_t kSlangShiftKindShaderResource = 2;
static constexpr int32_t kSlangShiftKindConstantBuffer = 3;

struct AkShaderByteCodeStorage
{
	AkShaderReflection reflection = {};
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
		{ slang::CompilerOptionName::Optimization,			{ slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_DEFAULT }},
		{ slang::CompilerOptionName::VulkanInvertY,			{ slang::CompilerOptionValueKind::Int, 1 }},
		{ slang::CompilerOptionName::ForceCLayout,			{ slang::CompilerOptionValueKind::Int, 1 }}
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

AkShaderData AkShaderCompiler::CompileShader(const AkShaderCompileOptions& compileOptions)
{
	Slang::ComPtr<slang::IBlob> diagnosticsBlob;
	slang::IModule* mainModule = m_Storage->session->loadModuleFromSource("Main", compileOptions.path.string().c_str(), nullptr, diagnosticsBlob.writeRef());

	if (!mainModule && diagnosticsBlob)
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	std::vector<slang::IComponentType*> componentTypes = { mainModule };
	for (const auto& [name, path] : compileOptions.modules)
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

	AkShaderData shaderData = {};
	AkShaderReflection& reflection = shaderData.m_Storage->reflection;
	slang::ProgramLayout* shaderReflection = linkedProgram->getLayout();

	unsigned int parameterCount = shaderReflection->getParameterCount();
	for (unsigned int i = 0; i < parameterCount; ++i)
	{
		slang::VariableLayoutReflection* parameter = shaderReflection->getParameterByIndex(i);
		const slang::ParameterCategory parameterCategory = parameter->getCategory();
		slang::TypeLayoutReflection* typeLayout = parameter->getTypeLayout();
		
		unsigned int set = 0;
		unsigned int binding = 0;
	
		if (parameterCategory == slang::ParameterCategory::SubElementRegisterSpace)
			set = parameter->getBindingIndex();
		else
		{
			set = parameter->getBindingSpace();
			binding = parameter->getBindingIndex();
		}

		if (parameterCategory == slang::ParameterCategory::PushConstantBuffer)
		{
			slang::TypeLayoutReflection* pushConstantLayout = typeLayout->getElementTypeLayout();
			reflection.pushConstantSize = static_cast<uint32_t>(pushConstantLayout->getSize());

			const unsigned int fieldCount = pushConstantLayout->getFieldCount();
			for (unsigned int j = 0; j < fieldCount; ++j)
			{
				slang::VariableLayoutReflection* field = pushConstantLayout->getFieldByIndex(j);
				slang::TypeReflection* fieldType = field->getType();

				if (strcmp(fieldType->getName(), "MaterialHandle") == 0)
				{
					slang::VariableReflection* genericType = fieldType->getGenericContainer()->getTypeParameter(0);
					slang::TypeReflection* concreteType = fieldType->getGenericContainer()->getConcreteType(genericType);
					slang::TypeLayoutReflection* concreteTypeLayout = shaderReflection->getTypeLayout(concreteType);
					reflection.materialDataSize = static_cast<uint32_t>(concreteTypeLayout->getSize());
					break;
				}
			}
		}
		else
		{
			if (set <= 0)
				continue;

			if (typeLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock)
			{
				slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();
				slang::VariableLayoutReflection* elementVariableLayout = typeLayout->getElementVarLayout();
				const slang::ParameterCategory elementCategory = elementVariableLayout->getCategory();

				if (elementCategory == slang::ParameterCategory::Uniform)
				{
					AkSetReflection& setReflection = reflection.descriptorSets[set];
					AkConstantBufferReflection& constantBuffer = setReflection.constantBuffers[0];
					constantBuffer.size = elementTypeLayout->getSize();
					++reflection.constantBuffersCount;
				}
				else
				{
					const unsigned int fieldCount = elementTypeLayout->getFieldCount();
					for (unsigned int j = 0; j < fieldCount; ++j)
					{
						slang::VariableLayoutReflection* field = elementTypeLayout->getFieldByIndex(j);
						slang::TypeLayoutReflection* fieldTypeLayout = field->getTypeLayout();
						const slang::TypeReflection::Kind kind = fieldTypeLayout->getKind();

						if (kind == slang::TypeReflection::Kind::ConstantBuffer || kind == slang::TypeReflection::Kind::Struct)
						{
							const uint32_t fieldBinding = static_cast<uint32_t>(field->getOffset(slang::ParameterCategory::ConstantBuffer));

							AkSetReflection& setReflection = reflection.descriptorSets[set];
							if (setReflection.constantBuffers.contains(fieldBinding))
								++reflection.constantBuffersCount;

							AkConstantBufferReflection& constantBuffer = setReflection.constantBuffers[fieldBinding];
							if(kind == slang::TypeReflection::Kind::Struct)
								constantBuffer.size += fieldTypeLayout->getSize();
							else
								constantBuffer.size += fieldTypeLayout->getElementTypeLayout()->getSize();
						}
					}
				}
			}
			else if (typeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer)
			{
				AkSetReflection& setReflection = reflection.descriptorSets[set];
				AkConstantBufferReflection& constantBuffer = setReflection.constantBuffers[binding];
				constantBuffer.size = typeLayout->getElementTypeLayout()->getSize();
				++reflection.constantBuffersCount;
			}
		}
	}

	Slang::ComPtr<slang::IBlob> compiledCode = {};
	result = linkedProgram->getTargetCode(0, compiledCode.writeRef(), diagnosticsBlob.writeRef());

	if (SLANG_FAILED(result))
	{
		const char* errorMessage = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
		throw std::runtime_error(errorMessage);
	}

	compiledCode.swap(shaderData.m_Storage->byteCode);
	return shaderData;
}

AkShaderData::AkShaderData()
{ }

AkShaderData::~AkShaderData()
{ }

size_t AkShaderData::GetByteCodeSize() const
{
	return m_Storage->byteCode->getBufferSize();
}

const uint8_t* AkShaderData::GetByteCode() const
{
	return static_cast<const uint8_t*>(m_Storage->byteCode->getBufferPointer());
}

const AkShaderReflection& AkShaderData::GetReflection() const
{
	return m_Storage->reflection;
}

AkShaderData::operator bool() const
{
	return m_Storage->byteCode;
}