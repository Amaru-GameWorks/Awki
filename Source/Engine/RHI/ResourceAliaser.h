#pragma once
#include "Graphics/RenderPass/RenderPass.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

struct AkResourceLifetime
{
	size_t firstPass = std::numeric_limits<size_t>::max();
	size_t lastPass = 0;
};

struct AkAliasingRequest
{
	size_t offset = 0;
	size_t consumed = 0;
	AkResourceRequest resourceRequest = {};
};

namespace std
{
	template <>
	struct hash<AkAliasingRequest>
	{
		size_t operator()(const AkAliasingRequest& request) const
		{
			size_t hash = 0;
			const AkResourceRequest& resourceRequest = request.resourceRequest;

			if (resourceRequest.type == AkResourceRequest::Type::TEXTURE)
				hash = Hash(resourceRequest.descriptor.texture);
			else
				hash = Hash(resourceRequest.descriptor.buffer);

			HashCombine(hash, resourceRequest.id);
			HashCombine(hash, resourceRequest.size);
			HashCombine(hash, resourceRequest.persistent);
			HashCombine(hash, resourceRequest.renderGroup);
			return hash;
		}
	};
}

struct AkAliasingGroup
{
	size_t hash = 0;
	bool persistent = false;
	AkResourceLifetime lifetime = {};
	std::vector<AkAliasingRequest> requests = {};
	std::unordered_set<uint8_t> renderGroups = {};

	std::unordered_map<AkPipelineResourceId, class AkBuffer*> buffers;
	std::unordered_map<AkPipelineResourceId, class AkTexture*> textures;

	std::shared_ptr<struct AkAliasedResources> rhiResources = nullptr;
};

class AkResourceAliaser
{
public:
	static void CreateAliasedResources(struct AkAliasingGroup& aliasingGroup);
	static void FreeAliasedResources(const struct AkAliasingGroup& aliasingGroup);
};