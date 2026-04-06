#include <Awki.h>
#include <Core/Log.h>

#include <Platform/Events.h>
#include <RHI/Buffers/Buffer.h>
#include <RHI/Pipeline/Shader.h>
#include <RHI/Textures/Texture.h>
#include <RHI/Pipeline/Material.h>
#include <RHI/Textures/RenderTarget.h>
#include <RHI/CommandBuffers/CommandBuffer.h>
#include <Utilities/Shaders/ShaderCompiler.h>

#include <print>
#include <glm/vec3.hpp>
#include <glm/gtc/type_aligned.hpp>

struct RandomData
{
	glm::packed_vec3 color1 = glm::vec3(1.f, 0.f, 1.f);
	glm::packed_vec3 color2 = glm::vec3(1.f, 0.f, 0.f);
	glm::packed_vec3 color3 = glm::vec3(0.f, 0.f, 1.f);
	glm::packed_vec3 color4 = glm::vec3(1.f, 0.f, 1.f);
};

struct alignas(16) UnlitMaterial
{
	glm::packed_vec3 color;
	uint32_t bufferHandle;
	uint32_t textureHandle;
};

Awki* m_Engine = nullptr;
AkShader* m_Shader = nullptr;
AkTexture* m_Texture = nullptr;
AkMaterial* m_Material = nullptr;
AkConstantBuffer* m_UnlitConstantBuffer = nullptr;
AkStructuredBuffer* m_RandomDataBuffer = nullptr;

AkShaderCompiler m_ShaderCompiler = {};
UnlitMaterial m_ColorBuffer = { .color = glm::packed_vec3(0.f, 1.f, 0.7f), .bufferHandle = 0, .textureHandle = 15 };
RandomData m_Random = {};

static void OnEngineStart()
{
	try
	{
		const AkShaderCompileOptions options = { .path = "Resources/Shaders/Shader.slang" };
		AkShaderByteCode byteCode = m_ShaderCompiler.CompileShader(options);

		m_Shader = new AkShader(byteCode);
		m_Material = new AkMaterial(m_Shader);
		
		m_RandomDataBuffer = new AkStructuredBuffer(m_Random);
		
		AkTextureDescriptor descriptor = {};
		descriptor.flags |= AkTextureFlags_ALLOW_UNORDERED_ACCESS;

		uint32_t pixel = 0xFFFFFFFF;
		m_Texture = new AkTexture(descriptor, reinterpret_cast<uint8_t*>(&pixel));

		m_ColorBuffer.bufferHandle = m_RandomDataBuffer->GetBindlessIndex();
		m_ColorBuffer.textureHandle = m_Texture->GetBindlessIndex();
		m_UnlitConstantBuffer = new AkConstantBuffer(m_ColorBuffer);

		m_Material->SetConstantBuffer(m_UnlitConstantBuffer, 0);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to initialize resources: {}", exception.what());
		AkEvents::TriggerQuit();
	}
}

static void OnFrameRender(AkCommandBuffer* commandBuffer, AkRenderTarget* backBuffer)
{
	commandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::UNDEFINED, AkResourceState::RENDER_TARGET);

	commandBuffer->BeginRendering(backBuffer);
	commandBuffer->DrawPrimitive(m_Material, AkPrimitiveType::TRIANGLES, 3);
	commandBuffer->EndRendering();

	commandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::RENDER_TARGET, AkResourceState::PRESENT);
}

static void OnEngineShutdown()
{
	delete m_Shader;
	delete m_Material;
	delete m_Texture;
	delete m_RandomDataBuffer;
	delete m_UnlitConstantBuffer;
}

int main(int /*argc*/, char** /*argv*/)
{
	try
	{
		AkInstanceDescriptor descriptor =
		{
			.gameName = "Awesome Game",
			.gameVersion = {0, 0, 1},
			.windowDescriptor = {
				.name = descriptor.gameName,
				.flags = AkWindowFlag_RESIZABLE,
				.width = 1920,
				.height = 1080
			}
		};

		m_Engine = new Awki(descriptor);
		m_Engine->GetOnEngineStart().Add(OnEngineStart);
		m_Engine->GetOnFrameRender().Add(OnFrameRender);
		m_Engine->GetOnEngineShutdown().Add(OnEngineShutdown);
	}
	catch (const std::exception& exception)
	{
		std::println("Failed to initialize Awki engine: {}", exception.what());
		return EXIT_FAILURE;
	}

	m_Engine->Run();
	delete m_Engine;

	return EXIT_SUCCESS;
}