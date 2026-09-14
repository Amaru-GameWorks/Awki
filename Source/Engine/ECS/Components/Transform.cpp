#include "Transform.h"
#include "Core/Assert.h"

#include <glm/gtx/transform.hpp>

void AkTransform::SetPosition(const glm::vec3& newPosition)
{
	m_Position = newPosition;
	m_State = DIRTY;
}

void AkTransform::SetRotation(const glm::quat& newRotation)
{
	m_Rotation = newRotation;
	m_State = DIRTY;
}

void AkTransform::SetScale(const glm::vec3& newScale)
{
	m_Scale = newScale;
	m_State = DIRTY;
}

void AkTransform::SetTransform(const glm::mat4& transform)
{
	m_Transform = transform;
	DecomposeTransform();
	m_State = RECENTLY_UPDATED;
}

const glm::mat4& AkTransform::GetTransform()
{
	AkAssert(!(m_State & DIRTY), "Transform is dirty");
	return m_Transform;
}

void AkTransform::Translate(const glm::vec3& translation, const Space space)
{
	switch (space)
	{
		case Space::LOCAL:
			m_Position += m_Rotation * translation;
			break;
		case Space::WORLD:
			m_Position += translation;
			break;
	}

	m_State = DIRTY;
}

void AkTransform::Rotate(const glm::quat& rotation)
{
	m_Rotation = m_Rotation * rotation;
	m_State = DIRTY;
}

void AkTransform::Rotate(const glm::vec3& rotation)
{
	Rotate(glm::quat(glm::radians(rotation)));
}

void AkTransform::Rotate(const float& angle, const glm::vec3& axis)
{
	Rotate(glm::angleAxis(glm::radians(angle), axis));
}

void AkTransform::LookAt(const glm::vec3& at, const glm::vec3& up)
{
	if (m_Position == at)
		return;

	const glm::vec3 direction = glm::normalize(at - m_Position);
	m_Rotation = glm::quat_cast(glm::lookAt(m_Position, m_Position + direction, up));
	m_State = DIRTY;
}

void AkTransform::Scale(const glm::vec3& scaler)
{
	m_Scale *= scaler;
	m_State = DIRTY;
}

void AkTransform::Scale(const float scaler)
{
	m_Scale *= scaler;
	m_State = DIRTY;
}

glm::vec3 AkTransform::Up()
{
	return glm::normalize(m_Rotation * glm::vec3(0.f, 1.f, 0.f));
}

glm::vec3 AkTransform::Right()
{
	return glm::normalize(m_Rotation * glm::vec3(1.f, 0.f, 0.f));
}

glm::vec3 AkTransform::Forward()
{
	return glm::normalize(m_Rotation * glm::vec3(0.f, 0.f, 1.f));
}

void AkTransform::Update()
{
	if (m_State & DIRTY)
	{
		m_Transform = glm::translate(m_Position);
		m_Transform *= glm::toMat4(m_Rotation);
		m_Transform *= glm::scale(m_Scale);
		m_State = State::RECENTLY_UPDATED;
	}
	else if (m_State & RECENTLY_UPDATED)
		m_State = STALE;
}

void AkTransform::DecomposeTransform()
{
	m_Position = glm::vec3(m_Transform[3][0], m_Transform[3][1], m_Transform[3][2]);

	m_Scale.x = glm::sqrt(glm::pow(m_Transform[0][0], 2.f) + glm::pow(m_Transform[0][1], 2.f) + glm::pow(m_Transform[0][2], 2.f));
	m_Scale.y = glm::sqrt(glm::pow(m_Transform[1][0], 2.f) + glm::pow(m_Transform[1][1], 2.f) + glm::pow(m_Transform[1][2], 2.f));
	m_Scale.z = glm::sqrt(glm::pow(m_Transform[2][0], 2.f) + glm::pow(m_Transform[2][1], 2.f) + glm::pow(m_Transform[2][2], 2.f));

	glm::mat3 rotationMatrix(m_Transform);

	if (m_Scale.x != 0.f)
		rotationMatrix[0] /= m_Scale.x;
	
	if (m_Scale.y != 0.f)
		rotationMatrix[1] /= m_Scale.y;
	
	if (m_Scale.z != 0.f)
		rotationMatrix[2] /= m_Scale.z;

	m_Rotation = glm::quat_cast(rotationMatrix);
}
