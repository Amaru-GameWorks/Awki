#include "GameThread.h"
#include "FrameData.h"
#include "RenderThread.h"
#include "Platform/Events.h"
#include "Core/EngineContext.h"

#include "ECS/Registry.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Transform.h"

static size_t sFrameIndex = 0;

void AkGameThread::Setup()
{
}

AkJob AkGameThread::Execute()
{ 
	while (true)
	{
		AkLogTrace("GameThread: {}", sFrameIndex);
		
		AkEvents::PollEvents();
		if (AkEvents::ShouldClose())
			break;

		co_await gFrameDataAvailable.Wait();

		AkFrameData& frameData = gFrameData[sFrameIndex % kMaxFramesInFlight];
		frameData.frameIndex = sFrameIndex;

		AkRegistry::GetView<AkTransform>().HierarchySort().ForEach([](AkTransform& transform)
		{
			transform.Update();
		});

		const glm::uvec2 windowResolution = AkEngineContext::GetMainWindow().GetSize();
		AkRegistry::GetView<AkTransform, AkCamera>().ForEach([windowResolution](AkTransform& transform, AkCamera& camera)
		{
			AkRenderTarget* renderTarget = camera.GetRenderTarget();
			const glm::uvec2 targetResolution = renderTarget ? renderTarget->GetSize() : windowResolution;
			camera.Update(transform, targetResolution);
		});

		AkJobSystem::Schedule(AkThreadType::Render, AkRenderThread::Execute(sFrameIndex++));
	}

	co_return;
}
