#include "Camera.h"
#include "ECS/Components/Transform.h"

void AkCamera::SetRenderOrder(uint8_t renderOrder)
{
	m_RenderOrder = renderOrder;
}

uint8_t AkCamera::GetRenderOrder() const
{
	return m_RenderOrder;
}

void AkCamera::SetProjectionType(const AkProjectionType& type)
{
	m_ProjectionType = type;
	m_Dirty = true;
}

AkProjectionType AkCamera::GetProjectionType() const
{
	return m_ProjectionType;
}

void AkCamera::SetFieldOfView(const float& fieldOfView)
{
	m_FOV = glm::clamp(fieldOfView, 1.f, 179.f);
	m_Dirty = true;
}

void AkCamera::SetNearFarPlaneDistance(const glm::vec2& nearFarDistance)
{
	m_NearFarDistance.x = glm::clamp(nearFarDistance.x, 0.005f, nearFarDistance.y);
	m_NearFarDistance.y = glm::clamp(nearFarDistance.y, nearFarDistance.x, std::numeric_limits<float>::max());
	m_Dirty = true;
}

void AkCamera::SetViewport(const AkViewport& viewport)
{
	m_Viewport = viewport;
	m_Dirty = true;
}

void AkCamera::SetRenderTarget(AkRenderTarget* renderTarget)
{
	m_RenderTarget = renderTarget;
	m_Dirty = true;
}

void AkCamera::Update(AkTransform& transform, const glm::uvec2& resolution)
{
	const bool resized = m_LastResolution != resolution;
	const bool needsViewUpdate = transform.HasUpdated();
	const bool needsProjectionChange = m_Dirty || resized;

	m_LastResolution = resolution;
	m_Dirty = false;

	if (!needsViewUpdate && !needsProjectionChange)
		return;

	if (needsProjectionChange)
	{
		float width = static_cast<float>(resolution.x);
		float height = static_cast<float>(resolution.y);

		if (m_ProjectionType == AkProjectionType::PERSPECTIVE)
		{
			m_Projection = glm::perspectiveFov(
				glm::radians(m_FOV),
				static_cast<float>(m_Viewport.width * width),
				static_cast<float>(m_Viewport.height * height),
				m_NearFarDistance.y,
				m_NearFarDistance.x
			);
		}
		else
		{
			width *= m_Viewport.width / 2;
			height *= m_Viewport.height / 2;
			m_Projection = glm::ortho(-width, width, -height, height, m_NearFarDistance.y, m_NearFarDistance.x);
		}
	}

	if (needsViewUpdate)
		m_View = glm::inverse(transform.GetTransform());

	m_ViewProjection = m_Projection * m_View;
	m_InverseViewProjection = glm::inverse(m_ViewProjection);
}

void AkCamera::SetRenderPipelineHash(size_t renderPipelineHash)
{
	m_RenderPipelineHash = renderPipelineHash;
}
