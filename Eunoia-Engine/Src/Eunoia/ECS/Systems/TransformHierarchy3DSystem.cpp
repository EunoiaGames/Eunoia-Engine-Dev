#include "TransformHierarchy3DSystem.h"
#include "../Components/Transform3DComponet.h"

namespace Eunoia
{
	TransformHierarchy3DSystem::TransformHierarchy3DSystem()
	{
		AddComponentType<Transform3DComponent>();
	}

	void TransformHierarchy3DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		EntityID parentEntity = m_ECS->GetParentEntity(entity);
		Transform3DComponent* transform = m_ECS->GetComponent<Transform3DComponent>(entity);
		if (parentEntity == EU_ECS_INVALID_ENTITY_ID)
		{
			transform->worldTransform = transform->localTransform;
			return;
		}
		Transform3DComponent* parentTransform = m_ECS->GetComponent<Transform3DComponent>(parentEntity);
		if (!parentTransform)
		{
			transform->worldTransform = transform->localTransform;
			return;
		}
		transform->worldTransform = parentTransform->worldTransform * transform->localTransform;
	}
}
