#include <Awki.h>
#include <Core/Log.h>

#include <Platform/Events.h>
#include <RHI/Buffers/Buffer.h>
#include <RHI/Pipeline/Shader.h>
#include <RHI/Pipeline/Material.h>
#include <RHI/Textures/RenderTarget.h>
#include <RHI/Pipeline/RasterizerState.h>
#include <RHI/CommandBuffers/CommandBuffer.h>
#include <RHI/Pipeline/PipelineStateObject.h>
#include <Utilities/Shaders/ShaderCompiler.h>

#include <print>
#include <glm/vec3.hpp>

struct UnlitMaterial
{
	glm::vec3 color;
};

Awki* m_Engine = nullptr;
AkShader* m_Shader = nullptr;
AkMaterial* m_Material = nullptr;
AkPipelineStateObject* m_PSO = nullptr;
AkConstantBuffer* m_UnlitConstantBuffer = nullptr;

AkShaderCompiler m_ShaderCompiler = {};
UnlitMaterial m_ColorBuffer = { .color = glm::vec3(0.f, 1.f, 0.7f) };

static void OnEngineStart()
{
	try
	{
		AkRenderTarget* backBuffer = m_Engine->GetMainSwapchain()->GetCurrentBackBufferRenderTarget();
		m_UnlitConstantBuffer = new AkConstantBuffer(m_ColorBuffer);

		const AkShaderCompileOptions options = { .path = "Resources/Shaders/Shader.slang" };
		AkShaderByteCode byteCode = m_ShaderCompiler.CompileShader(options);
		
		m_Shader = new AkShader(byteCode.GetByteCode(), byteCode.GetSize());
		
		m_Material = new AkMaterial(m_Shader);
		m_Material->SetConstantBuffer(m_UnlitConstantBuffer, 0);

		m_PSO = new AkPipelineStateObject(m_Material, backBuffer);
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
	commandBuffer->DrawMaterialNoMesh(m_Material, m_PSO, 3);
	commandBuffer->EndRendering();

	commandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::RENDER_TARGET, AkResourceState::PRESENT);
}

static void OnEngineShutdown()
{
	delete m_PSO;
	delete m_Shader;
	delete m_Material;
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