#include "DirectAcyclicRenderGraph.h"

#include "Core/Log.h"
#include "RenderPipeline.h"
#include "RHI/ResourceAliaser.h"

#include <ranges>
#include <algorithm>
#include <unordered_map>

bool LifetimeOverlaps(const AkResourceLifetime& lhs, const AkResourceLifetime& rhs)
{
	return std::max(lhs.firstPass, rhs.firstPass) <= std::min(lhs.lastPass, rhs.lastPass);
}

bool SortDFS(size_t node, std::vector<bool>& visited, std::vector<bool>& stack, std::vector<size_t>& sortedPassIndices, const std::vector<std::vector<size_t>>& adjacencyList)
{
	stack[node] = true;
	visited[node] = true;

	for (size_t neighbor : adjacencyList[node])
	{
		if (!visited[neighbor])
		{
			if (SortDFS(neighbor, visited, stack, sortedPassIndices, adjacencyList))
				return true;
		}
		else if (stack[neighbor])
		{
			return true;
		}
	}

	stack[node] = false;
	sortedPassIndices.push_back(node);
	return false;
}

std::vector<std::vector<size_t>> ComputeAdjacencyList(const std::vector<AkRenderPass*>& renderPasses)
{
	std::vector<std::vector<size_t>> adjacencyList(renderPasses.size(), {});

	std::unordered_map<AkPipelineResourceId, size_t> lastWriter;
	std::unordered_map<AkPipelineResourceId, std::vector<size_t>> currentReaders;
	std::unordered_map<AkRenderPipeline*, std::unordered_map<AkId, AkPipelineResourceId>> rendePiplineExportedResources;

	for (size_t i = 0; i < renderPasses.size(); ++i)
	{
		AkRenderPass* renderPass = renderPasses[i];
		AkRenderPipeline* renderPipeline = renderPass->GetRenderPipeline();

		const std::unordered_map<AkId, AkResourceState>& importedResources = renderPass->GetImportedResources();
		if (!importedResources.empty())
		{
			for (auto& [resourcesPipeline, exportedResources] : rendePiplineExportedResources)
			{
				if (resourcesPipeline != renderPipeline)
				{
					for (auto& [resourceTag, resourceState] : importedResources)
					{
						if (exportedResources.contains(resourceTag))
						{
							const AkPipelineResourceId exportedResourceId = exportedResources[resourceTag];
							renderPass->Reads(exportedResourceId, resourceState);
						}
					}
				}
			}
		}

		const std::unordered_map<AkId, AkPipelineResourceId>& renderPassExportedResources = renderPass->GetExportedResources();
		std::unordered_map<AkId, AkPipelineResourceId>& renderPipelineExports = rendePiplineExportedResources[renderPipeline];
		renderPipelineExports.insert_range(renderPassExportedResources);

		for (const AkResourceUsage& usage : renderPass->GetResourcesUsage())
		{
			if (usage.access == AkResourceUsage::AccessType::READ)
			{
				if (lastWriter.contains(usage.id))
				{
					size_t parent = lastWriter[usage.id];
					adjacencyList[parent].push_back(i);
				}
				currentReaders[usage.id].push_back(i);
			}
			else if (usage.access == AkResourceUsage::AccessType::WRITE)
			{
				if (currentReaders.contains(usage.id))
				{
					for (size_t pass : currentReaders[usage.id])
						adjacencyList[pass].push_back(i);

					currentReaders[usage.id].clear();
				}

				if (lastWriter.contains(usage.id))
				{
					size_t parent = lastWriter[usage.id];
					adjacencyList[parent].push_back(i);
				}

				lastWriter[usage.id] = i;
			}
		}
	}

	return adjacencyList;
}

std::vector<AkRenderPass*> ComputeSortedRenderPasses(const std::vector<AkRenderPass*>& renderPasses, const std::vector<std::vector<size_t>>& adjacencyList)
{
	std::vector<bool> visited(renderPasses.size(), false);
	std::vector<bool> stack(renderPasses.size(), false);

	std::vector<size_t> sortedPassIndices;
	for (size_t i = 0; i < renderPasses.size(); ++i)
	{
		if (!visited[i])
		{
			if (SortDFS(i, visited, stack, sortedPassIndices, adjacencyList))
			{
				AkLogCritical("Circular dependency detected in Render Graph!");
			}
		}
	}
	std::reverse(sortedPassIndices.begin(), sortedPassIndices.end());

	std::vector<AkRenderPass*> sortedRenderPasses;
	sortedRenderPasses.reserve(sortedPassIndices.size());

	for (size_t passIndex : sortedPassIndices)
		sortedRenderPasses.push_back(renderPasses[passIndex]);

	return sortedRenderPasses;
}

std::unordered_map<AkPipelineResourceId, AkResourceLifetime> ComputeResourceLifetimes(const std::vector<AkRenderPass*>& renderPasses)
{
	std::unordered_map<AkPipelineResourceId, AkResourceLifetime> lifeTimes = {};
	for (size_t timelineIdx = 0; timelineIdx < renderPasses.size(); ++timelineIdx)
	{
		AkRenderPass* renderPass = renderPasses[timelineIdx];
		for (const AkResourceUsage& usage : renderPass->GetResourcesUsage())
		{
			lifeTimes[usage.id].firstPass = std::min(lifeTimes[usage.id].firstPass, timelineIdx);
			lifeTimes[usage.id].lastPass = std::max(lifeTimes[usage.id].lastPass, timelineIdx);
		}
	}

	return lifeTimes;
}

std::vector<AkAliasingGroup> CreateAliasingGroups(const std::vector<AkRenderPass*>& renderPasses, const std::unordered_map<AkPipelineResourceId, AkResourceLifetime>& resourceLifetimes)
{
	std::vector<AkResourceRequest> requests = {};
	for (auto& renderPass : renderPasses)
		requests.insert_range(requests.end(), renderPass->GetResourceRequests());

	std::sort(requests.begin(), requests.end(), [](const AkResourceRequest& lhs, const AkResourceRequest& rhs)
	{ return lhs.size > rhs.size; });

	std::vector<AkAliasingGroup> aliasingGroups;
	for (const AkResourceRequest& request : requests)
	{
		bool groupFound = false;
		const AkResourceLifetime& resourceLifetime = resourceLifetimes.at(request.id);

		if (!request.persistent)
		{
			for (AkAliasingGroup& group : aliasingGroups)
			{
				if (!group.persistent && group.renderGroups.contains(request.renderGroup))
				{
					for (AkAliasingRequest& groupRequest : group.requests)
					{
						const AkResourceLifetime& groupRequestResourceLifetime = resourceLifetimes.at(groupRequest.resourceRequest.id);
						if (!LifetimeOverlaps(resourceLifetime, groupRequestResourceLifetime))
						{
							const size_t offset = groupRequest.offset + groupRequest.consumed;
							if (offset + request.size <= groupRequest.resourceRequest.size)
							{
								groupRequest.consumed += request.size;

								group.renderGroups.insert(request.renderGroup);
								group.lifetime.firstPass = std::min(group.lifetime.firstPass, resourceLifetime.firstPass);
								group.lifetime.lastPass = std::max(group.lifetime.lastPass, resourceLifetime.lastPass);
								HashCombine(group.hash, request);

								AkAliasingRequest& aliasingRequest = group.requests.emplace_back();
								aliasingRequest.resourceRequest = request;
								aliasingRequest.offset = offset;
								groupFound = true;
								break;
							}
						}
					}
				}

				if (groupFound)
					break;
			}
		}

		if (!groupFound)
		{
			AkAliasingGroup& newGroup = aliasingGroups.emplace_back();
			newGroup.persistent = request.persistent;
			newGroup.renderGroups.insert(request.renderGroup);
			newGroup.lifetime.firstPass = std::min(newGroup.lifetime.firstPass, resourceLifetime.firstPass);
			newGroup.lifetime.lastPass = std::max(newGroup.lifetime.lastPass, resourceLifetime.lastPass);
			HashCombine(newGroup.hash, request);

			AkAliasingRequest& aliasingRequest = newGroup.requests.emplace_back();
			aliasingRequest.resourceRequest = request;
			aliasingRequest.offset = 0;
		}
	}

	return aliasingGroups;
}

void AkDirectAcyclicRenderGraph::Compile(const std::multimap<uint8_t, AkRenderPipeline*>& groupedRenderPipelines)
{
	std::vector<AkRenderPass*> renderPasses;
	std::vector<std::unordered_set<uint8_t>> renderGroupLookUp = {};

	for (auto& [renderOrder, renderPipeline] : groupedRenderPipelines)
	{
		bool groupFound = false;
		uint8_t renderGroup = {};

		for (auto [index, group] : std::views::enumerate(renderGroupLookUp))
		{
			if (!group.count(renderOrder))
			{
				renderGroup = static_cast<uint8_t>(index);
				group.insert(renderOrder);
				groupFound = true;
				break;
			}
		}

		if (!groupFound)
		{
			renderGroup = static_cast<uint8_t>(renderGroupLookUp.size());
			renderGroupLookUp.push_back({ renderOrder });
		}

		const std::vector<AkRenderPass*>& executingRenderPasses = renderPipeline->GetExecutingRenderPasses();
		renderPasses.insert_range(renderPasses.end(), executingRenderPasses);

		for (auto& renderPass : executingRenderPasses)
		{
			for (auto& request : renderPass->GetResourceRequests())
				request.renderGroup = renderGroup;
		}
	}

	const std::vector<std::vector<size_t>> adjacencyList = ComputeAdjacencyList(renderPasses);
	m_SortedRenderPasses = ComputeSortedRenderPasses(renderPasses, adjacencyList);

	std::unordered_map<AkPipelineResourceId, AkResourceLifetime> resourceLifetimes = ComputeResourceLifetimes(m_SortedRenderPasses);
	std::vector<AkAliasingGroup> frameAliasingGroups = CreateAliasingGroups(renderPasses, resourceLifetimes);

	try
	{
		std::vector<size_t> reUsableGroups;
		for (auto frameAliasingGroup = frameAliasingGroups.begin(); frameAliasingGroup != frameAliasingGroups.end();)
		{
			bool foundReusableGroup = false;
			for (const AkAliasingGroup& aliasingGroup : m_AliasingGroups)
			{
				if (aliasingGroup.hash == frameAliasingGroup->hash)
				{
					foundReusableGroup = true;
					reUsableGroups.push_back(aliasingGroup.hash);
					frameAliasingGroup = frameAliasingGroups.erase(frameAliasingGroup);
					break;
				}
			}

			if (!foundReusableGroup)
			{
				AkResourceAliaser::CreateAliasedResources(*frameAliasingGroup);
				++frameAliasingGroup;
			}
		}

		for (auto aliasingGroup = m_AliasingGroups.begin(); aliasingGroup != m_AliasingGroups.end();)
		{
			if (std::find(reUsableGroups.begin(), reUsableGroups.end(), aliasingGroup->hash) == reUsableGroups.end())
			{
				AkResourceAliaser::FreeAliasedResources(*aliasingGroup);
				aliasingGroup = m_AliasingGroups.erase(aliasingGroup);
				continue;
			}

			++aliasingGroup;
		}
	}
	catch (const std::exception& exception)
	{
		AkLogCritical("Failed to manage resource aliasing: {}", exception.what());
	}

	m_AliasingGroups.insert_range(m_AliasingGroups.end(), frameAliasingGroups);

	m_ManagedBuffers.clear();
	m_ManagedTextures.clear();
	
	for (const AkAliasingGroup& aliasingGroup : m_AliasingGroups)
	{
		m_ManagedBuffers.insert_range(aliasingGroup.buffers);
		m_ManagedTextures.insert_range(aliasingGroup.textures);
	}
}

void AkDirectAcyclicRenderGraph::Execute(std::vector<AkCommandBuffer*>& commandBuffers)
{
	commandBuffers.reserve(commandBuffers.size() + m_SortedRenderPasses.size());

	std::unordered_map<AkPipelineResourceId, AkResourceState> usedStates = {};

	for (AkRenderPass* renderPass : m_SortedRenderPasses)
	{
		AkCommandBuffer*& commandBuffer = commandBuffers.emplace_back(renderPass->GetCommandBuffer());
		commandBuffer->Begin();

		const std::vector<AkResourceUsage>& resourceUsage = renderPass->GetResourcesUsage();
		if (!resourceUsage.empty())
		{
			std::vector<AkResourceTransition> bufferTransitions = {};
			std::vector<AkResourceTransition> textureTransitions = {};

			for (const AkResourceUsage& resource : resourceUsage)
			{
				AkBuffer* managedBuffer = nullptr;
				AkTexture* managedTexture = nullptr;

				if (GetManagedResource(resource.id, managedBuffer, managedTexture))
				{
					AkResourceTransition& transition = textureTransitions.emplace_back();
					if (!usedStates.contains(resource.id))
					{
						transition.sourceState = AkResourceState::UNDEFINED;
						transition.destinationState = resource.state;
						usedStates[resource.id] = resource.state;
					}
					else
					{
						transition.sourceState = usedStates[resource.id];
						transition.destinationState = resource.state;
						usedStates[resource.id] = resource.state;
					}

					if (managedBuffer)
						transition.resource = managedBuffer;
					else
						transition.resource = managedTexture;
				}
			}

			commandBuffer->TransitionResources(bufferTransitions, textureTransitions);
		}

		renderPass->Execute(*this);
		commandBuffer->End();
	}
}

bool AkDirectAcyclicRenderGraph::GetManagedResource(AkPipelineResourceId id, AkBuffer*& managedBuffer, AkTexture*& managedTexture)
{
	if (m_ManagedBuffers.contains(id))
	{
		managedBuffer = m_ManagedBuffers[id];
		return true;
	}

	if (m_ManagedTextures.contains(id))
	{
		managedTexture = m_ManagedTextures[id];
		return true;
	}

	return false;
}

void AkDirectAcyclicRenderGraph::FreeManagedResources()
{
	for (const AkAliasingGroup& aliasingGroup : m_AliasingGroups)
		AkResourceAliaser::FreeAliasedResources(aliasingGroup);
}
