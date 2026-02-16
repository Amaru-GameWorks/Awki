#include <Awki.h>
#include <Core/Log.h>
#include <RHI/Textures/Texture.h>
#include <RHI/Textures/RenderTarget.h>
#include <RHI/CommandBuffers/CommandBuffer.h>

#include <print>

static void OnEngineStart()
{
	AkLogTrace("Hello from Start!");
}

static void OnFrameRender(AkCommandBuffer* commandBuffer, AkRenderTarget* backBuffer)
{
	AkTexture* colorTexture = backBuffer->GetColorTexture();
	commandBuffer->TransitionTexture(colorTexture, AkResourceState::UNDEFINED, AkResourceState::RENDER_TARGET);

	commandBuffer->BeginRendering(backBuffer);
	commandBuffer->EndRendering();

	commandBuffer->TransitionTexture(colorTexture, AkResourceState::RENDER_TARGET, AkResourceState::PRESENT);
}

static void OnEngineShutdown()
{
	AkLogTrace("Hello from Shutdown!");
}

int main(int /*argc*/, char** /*argv*/)
{
	std::shared_ptr<Awki> engine = nullptr;

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

		engine = std::make_shared<Awki>(descriptor);
		engine->GetOnEngineStart().Add(OnEngineStart);
		engine->GetOnFrameRender().Add(OnFrameRender);
		engine->GetOnEngineShutdown().Add(OnEngineShutdown);
	}
	catch (const std::exception& exception)
	{
		std::println("Failed to initialize Awki engine: {}", exception.what());
		return EXIT_FAILURE;
	}

	engine->Run();
	return EXIT_SUCCESS;
}