#pragma once
#include <queue>

namespace vk
{
	class DescriptorSet;
	class DescriptorSetLayout;
}

class AkBindlessResourcesManager
{
public:
	static constexpr int16_t kMaxBindlessResources = std::numeric_limits<int16_t>::max();
	static constexpr int16_t kInvalidBindlessIndex = -1;

	static void Initialize();
	static void Deinitialize();

	static void AddBuffer(class AkBuffer* buffer);
	static void AddTexture(class AkTexture* texture);

	static const vk::DescriptorSet& GetBuffersDescriptorSet();
	static const vk::DescriptorSet& GetTexturesDescriptorSet();
	static const vk::DescriptorSet& GetSamplersDescriptorSet();

	static const vk::DescriptorSetLayout& GetBuffersDescriptorSetLayout();
	static const vk::DescriptorSetLayout& GetTexturesDescriptorSetLayout();
	static const vk::DescriptorSetLayout& GetSamplersDescriptorSetLayout();

private:
	static inline std::atomic_int16_t sBuffersCount = 0;
	static inline std::queue<int16_t> sBuffersFreeList = {};

	static inline std::atomic_int16_t sTexturesCount = 0;
	static inline std::queue<int16_t> sTexturesFreeList = {};
};