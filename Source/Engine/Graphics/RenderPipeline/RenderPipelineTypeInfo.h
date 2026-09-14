#pragma once
#include "RenderPipeline.h"
#include "Utilities/Hash.h"

#include <memory>
#include <functional>

static constexpr size_t kInvalidRenderPipelineHash = static_cast<size_t>(-1);

template<typename T>
struct AkRenderPipelineTypeInfo
{ };

class AkRenderPipelineTypeInfoDatabase
{
public:
	template<typename T>
	static size_t Get()
	{
		return AkRenderPipelineTypeInfo<T>::TypeId();
	}

	template<typename T>
	static void Set()
	{
		constexpr size_t typeId = AkRenderPipelineTypeInfo<T>::TypeId();
		sRenderPipelineConstructors[typeId] = []() { return std::make_unique<T>(); };
	}

	template<typename T>
	static std::unique_ptr<AkRenderPipeline> Create()
	{
		constexpr size_t typeId = AkRenderPipelineTypeInfo<T>::TypeId();
		return std::move(sRenderPipelineConstructors[typeId]());
	}

	static std::unique_ptr<AkRenderPipeline> Create(size_t renderPipelineHash)
	{
		if (renderPipelineHash == kInvalidRenderPipelineHash)
			return nullptr;

		return std::move(sRenderPipelineConstructors[renderPipelineHash]());
	}

private:
	static inline std::unordered_map<size_t, std::function<std::unique_ptr<AkRenderPipeline>()>> sRenderPipelineConstructors;
};

#define REGISTER_RENDERPIPELINE(renderPipeline)	class renderPipeline; template<> struct AkRenderPipelineTypeInfo<renderPipeline>						\
												{																										\
													AkRenderPipelineTypeInfo() { AkRenderPipelineTypeInfoDatabase::Set<renderPipeline>(); }				\
													static constexpr const char* Name() { return #renderPipeline; }										\
													static constexpr size_t TypeId() { return FNV1aHash(#renderPipeline); }								\
												};																										\
												static inline AkRenderPipelineTypeInfo<renderPipeline> s##renderPipeline##TypeInfo##StaticInitializer;