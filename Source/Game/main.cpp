#include <Awki.h>
#include <Core/Log.h>

#include <Platform/Events.h>
#include <RHI/Buffers/Model.h>
#include <RHI/Pipeline/Shader.h>
#include <RHI/Textures/Texture.h>
#include <RHI/Pipeline/Material.h>
#include <Utilities/Shaders/ShaderCompiler.h>
#include <Utilities/Models/GltfModelDecoder.h>
#include <Utilities/Textures/TextureDecoder.h>
#include <Graphics/RenderPipeline/StandardRenderPipeline.h>

#include <ECS/GameEntity.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/Transform.h>

#include <print>
#include <glm/vec4.hpp>

struct UnlitMaterial
{
	glm::vec4 color;
	int32_t diffuseHandle;
};

Awki* m_Engine = nullptr;
AkModel* m_Model = nullptr;
AkShader* m_Shader = nullptr;
AkTexture* m_Texture = nullptr;
AkMaterial* m_Material = nullptr;
AkShaderCompiler m_ShaderCompiler = {};

AkGameEntity m_CameraId = {};

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

		UnlitMaterial primitiveMaterial =
		{
			.color = glm::vec4(1.f, 1.f, 1.f, 1.f),
			.diffuseHandle = m_Texture->GetBindlessIndex()
		};
		m_Material->SetData(primitiveMaterial);

		m_CameraId = AkRegistry::CreateEntity();
		m_CameraId.AddComponent<AkTransform>();
		
		AkCamera* camera = m_CameraId.AddComponent<AkCamera>();
		camera->SetRenderOrder(0);
		camera->SetRenderPipelineHash<AkStandardRenderPipeline>();
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to initialize resources: {}", exception.what());
		AkEvents::TriggerQuit();
	}
}

static void OnEngineShutdown()
{
	delete m_Model;
	delete m_Shader;
	delete m_Texture;
	delete m_Material;
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