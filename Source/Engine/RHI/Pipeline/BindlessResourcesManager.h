#pragma once
#include <queue>
#include <atomic>
#include <limits>

namespace vk
{
	class DescriptorSet;
	class DescriptorSetLayout;
}

class AkBindlessResourcesManager
{
private:
	friend class Awki;
	static void Initialize();
	static void Deinitialize();

public:
	static constexpr int32_t kMaxBindlessResources = std::numeric_limits<uint16_t>::max();
	static constexpr int32_t kInvalidBindlessIndex = -1;

	static void AddBuffer(class AkBuffer* buffer);
	static void AddTexture(class AkTexture* texture);
	static void AddSampler(class AkSampler* sampler);

	static void RemoveBuffer(class AkBuffer* buffer);
	static void RemoveTexture(class AkTexture* texture);
	static void RemoveSampler(class AkSampler* sampler);

	static const vk::DescriptorSet& GetDescriptorSet();
	static const vk::DescriptorSetLayout& GetDescriptorSetLayout();

private:
	static inline std::atomic_int32_t sBuffersCount = 0;
	static inline std::queue<int32_t> sBuffersFreeList = {};

	static inline std::atomic_int32_t sTexturesCount = 0;
	static inline std::queue<int32_t> sTexturesFreeList = {};

	static inline std::atomic_int32_t sSamplersCount = 0;
	static inline std::queue<int32_t> sSamplersFreeList = {};
};