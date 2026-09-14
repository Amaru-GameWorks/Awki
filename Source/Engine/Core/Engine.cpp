#include "Engine.h"
#include "Log.h"
#include "RHI/Device.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

#include "ECS/Registry.h"
#include "RHI/UploadManager.h"
#include "RHI/Samplers/Sampler.h"
#include "RHI/Pipeline/Material.h"
#include "RHI/Pipeline/PipelineStateManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

std::unique_ptr<AkSampler> gPointClampSampler;
std::unique_ptr<AkSampler> gLinearClampSampler;

void InitializeEngineResources()
{
	AkMaterial::InitializeGlobalBuffer();

	AkSamplerDescriptor samplerDescriptor = {};
	samplerDescriptor.filterMode = AkFilterMode::NEAREST;
	samplerDescriptor.SetWrapMode(AkWrapMode::CLAMP_TO_EDGE);
	gPointClampSampler = std::make_unique<AkSampler>(samplerDescriptor);

	samplerDescriptor.filterMode = AkFilterMode::LINEAR;
	samplerDescriptor.SetWrapMode(AkWrapMode::CLAMP_TO_EDGE);
	gLinearClampSampler = std::make_unique<AkSampler>(samplerDescriptor);
}

void FreeEngineResources()
{
	gPointClampSampler = nullptr;
	gLinearClampSampler = nullptr;

	AkMaterial::DeinitializeGlobalBuffer();
}

Awki::Awki(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		throw std::runtime_error("Failed to initialize Logging System!");

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		throw std::runtime_error("Failed to initialize Events System!");

	if (!AkDevice::Initialize())
		throw std::runtime_error("Failed to initialize RHI Device!");

	AkPipelineStateManager::Initialize();
	AkBindlessResourcesManager::Initialize();

	InitializeEngineResources();

	m_Window = std::make_unique<AkWindow>(descriptor.windowDescriptor);
	m_Swapchain = std::make_unique<AkSwapchain>(m_Window.get());
	
	m_Scheduler.SetWindow(m_Window.get());
	m_Scheduler.SetSwapchain(m_Swapchain.get());

	AkLogInfo("{} {} initializing", descriptor.gameName, descriptor.gameVersion);
}

Awki::~Awki()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);
	AkDevice::WaitIdle();

	m_Swapchain = nullptr;
	m_Window = nullptr;

	FreeEngineResources();
	AkUploadManager::ReleaseBuffers();

	AkBindlessResourcesManager::Deinitialize();
	AkPipelineStateManager::Deinitialize();
	AkDevice::Deinitialize();
	AkEvents::Deinitialize();
	AkLog::Deinitialize();
}

void Awki::Run()
{
	m_OnEngineStart.Broadcast();
	m_Scheduler.Run();
	m_OnEngineShutdown.Broadcast();
}