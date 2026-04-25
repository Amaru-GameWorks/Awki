#include <Awki.h>
#include <Core/Log.h>

#include <Platform/Events.h>
#include <RHI/Buffers/Model.h>
#include <RHI/Buffers/Buffer.h>
#include <RHI/Pipeline/Shader.h>
#include <RHI/Textures/Texture.h>
#include <RHI/Pipeline/Material.h>
#include <RHI/Textures/RenderTarget.h>
#include <RHI/CommandBuffers/CommandBuffer.h>
#include <Utilities/Shaders/ShaderCompiler.h>
#include <Utilities/Models/GltfModelDecoder.h>
#include <Utilities/Textures/TextureDecoder.h>

#include <print>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct QuadMaterial
{
	glm::vec4 color;
	int32_t textureHandle;
};

struct UnlitMaterial
{
	glm::vec4 color;
	glm::mat4 viewProjection;
	int32_t diffuseHandle;
};

Awki* m_Engine = nullptr;
AkModel* m_Model = nullptr;
AkShader* m_Shader = nullptr;
AkTexture* m_Texture = nullptr;
AkMaterial* m_Material = nullptr;
AkConstantBuffer* m_PrimitiveMaterialBuffer = nullptr;

AkShaderCompiler m_ShaderCompiler = {};

static void OnEngineStart()
{
	try
	{
		const AkShaderCompileOptions options = { .path = "Resources/Shaders/UnlitMesh.slang" };
		AkShaderData shaderData = m_ShaderCompiler.CompileShader(options);

		m_Shader = new AkShader(shaderData);
		m_Material = new AkMaterial(m_Shader);

		std::unique_ptr<AkTextureDecoderInterface> textureDecoder = AkTextureDecoder::Decode("Resources/Textures/Minicular.png");
		m_Texture = new AkTexture(textureDecoder->GetDescriptor(), textureDecoder->GetData());

		std::unique_ptr<AkModelDecoderInterface> modelDecoder = AkModelDecoder::Decode("Resources/Models/Test.gltf");
		m_Model = new AkModel(modelDecoder);

		glm::uvec2 windowSize = m_Engine->GetMainWindow()->GetSize();
		glm::mat4 projection = glm::perspectiveFov(glm::radians(75.f), static_cast<float>(windowSize.x), static_cast<float>(windowSize.y), 0.001f, 1000.f);
		glm::mat4 view = glm::lookAt(glm::vec3(0.f, 10.f, -10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		UnlitMaterial primitiveMaterial =
		{
			.color = glm::vec4(1.f, 1.f, 1.f, 1.f),
			.viewProjection = projection * view,
			.diffuseHandle = m_Texture->GetBindlessIndex()
		};
		m_PrimitiveMaterialBuffer = new AkConstantBuffer(primitiveMaterial);
		m_Material->SetConstantBuffer(m_PrimitiveMaterialBuffer, 0);
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

	for (const AkMesh& mesh : m_Model->GetMeshes())
		commandBuffer->DrawMesh(const_cast<AkMesh*>(&mesh), m_Material);

	commandBuffer->EndRendering();

	commandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::RENDER_TARGET, AkResourceState::PRESENT);
}

static void OnEngineShutdown()
{
	delete m_Model;
	delete m_Shader;
	delete m_Texture;
	delete m_Material;
	delete m_PrimitiveMaterialBuffer;
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