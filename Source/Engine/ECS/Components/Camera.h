#pragma once
#include "ECS/ComponentTypeInfo.h"
#include "Graphics/RenderPipeline/RenderPipelineTypeInfo.h"

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

enum class AkProjectionType
{
	PERSPECTIVE,
	ORTOGRAPHIC
};

struct AkViewport
{
	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};

class AkTransform;

REGISTER_COMPONENT(AkCamera, AkTransform);
class AkCamera
{
public:
	void SetRenderOrder(uint8_t renderOrder);
	uint8_t GetRenderOrder() const;

	void SetProjectionType(const AkProjectionType& type);
	AkProjectionType GetProjectionType() const;

	void SetFieldOfView(const float& fieldOfView);
	float GetFieldOfView() const { return m_FOV; }

	void SetNearFarPlaneDistance(const glm::vec2& nearFarDistance);
	const glm::vec2& GetNearFarPlaneDistance() const { return m_NearFarDistance; }

	void SetViewport(const AkViewport& viewport);
	const AkViewport& GetViewport() const { return m_Viewport; }

	const glm::mat4& GetViewMatrix() const { return m_View; }
	const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }
	const glm::mat4& GetInverseViewProjectionMatrix() const { return m_InverseViewProjection; }

	void SetRenderTarget(class AkRenderTarget* renderTarget);
	class AkRenderTarget* GetRenderTarget() const { return m_RenderTarget; }

	void Update(class AkTransform& transform, const glm::uvec2& resolution);
	const glm::uvec2& GetRenderTargetResolution() { return m_LastResolution; }

	void SetRenderPipelineHash(size_t renderPipelineHash);
	size_t GetRenderPipelineHash() const { return m_RenderPipelineHash; }
	
	template<typename T> 
	void SetRenderPipelineHash()
	{ 
		m_RenderPipelineHash = AkRenderPipelineTypeInfoDatabase::Get<T>(); 
	}

private:
	bool m_Dirty = true;
	uint8_t m_RenderOrder = 0;

	class AkRenderTarget* m_RenderTarget = nullptr;
	size_t m_RenderPipelineHash = kInvalidRenderPipelineHash;

	float m_FOV = 75.f;
	glm::uvec2 m_LastResolution = glm::uvec2(1, 1);
	glm::vec2 m_NearFarDistance = glm::vec2(0.005f, 1000.f);
	AkViewport m_Viewport = { 0.f, 0.f, 1.f, 1.f, 0.f, 1.f };
	AkProjectionType m_ProjectionType = AkProjectionType::PERSPECTIVE;

	glm::mat4 m_View = glm::mat4(1.f);
	glm::mat4 m_Projection = glm::mat4(1.f);
	glm::mat4 m_ViewProjection = glm::mat4(1.f);
	glm::mat4 m_InverseViewProjection = glm::mat4(1.f);
};