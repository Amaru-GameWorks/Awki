#pragma once
#include "ECS/ComponentTypeInfo.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

REGISTER_COMPONENT(AkTransform);
class AkTransform
{
	enum class Space
	{
		LOCAL,
		WORLD
	};

public:
	const glm::vec3& GetPosition() const { return m_Position; }
	void SetPosition(const glm::vec3& newPosition);
	
	const glm::quat& GetRotation() const { return m_Rotation; }
	void SetRotation(const glm::quat& newRotation);
	
	const glm::vec3& GetScale() const { return m_Scale; }
	void SetScale(const glm::vec3& newScale);
	
	void SetTransform(const glm::mat4& transform);
	const glm::mat4& GetTransform();

	void Translate(const glm::vec3& translation, const Space space = Space::LOCAL);
	void Rotate(const glm::quat& rotation);
	void Rotate(const glm::vec3& rotation);
	void Rotate(const float& angle, const glm::vec3& axis);
	void LookAt(const glm::vec3& at, const glm::vec3& up = glm::vec3(0.f, 1.f, 0.f));
	void Scale(const glm::vec3& scaler);
	void Scale(const float scaler);

	//Direction - function
	glm::vec3 Up();
	glm::vec3 Right();
	glm::vec3 Forward();

	void Update();
	bool HasUpdated() const { return m_State & RECENTLY_UPDATED; }

private:
	enum State
	{
		STALE				= 1 << 0,
		DIRTY				= 1 << 1,
		RECENTLY_UPDATED	= 1 << 2
	};

	State m_State = DIRTY;
	glm::vec3 m_Scale = glm::vec3(1.f);
	glm::quat m_Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 m_Position = glm::vec3(0.f);
	glm::mat4 m_Transform = glm::mat4(1.f);

	void DecomposeTransform();
};