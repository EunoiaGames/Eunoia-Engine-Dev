#include "TransformHierarchy2DSystem.h"
#include "../Components/Transform2DComponent.h"

namespace Eunoia {

	TransformHierarchy2DSystem::TransformHierarchy2DSystem()
	{
		AddComponentType<Transform2DComponent>();
	}

	void TransformHierarchy2DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		EntityID parentEntity = m_ECS->GetParentEntity(entity);
		Transform2DComponent* transform = m_ECS->GetComponent<Transform2DComponent>(entity);
		if (parentEntity == EU_ECS_INVALID_ENTITY_ID)
		{
			transform->worldTransform = transform->localTransform;
			return;
		}
		Transform2DComponent* parentTransform = m_ECS->GetComponent<Transform2DComponent>(parentEntity);
		if (!parentTransform)
		{
			transform->worldTransform = transform->localTransform;
			return;
		}
		transform->worldTransform = parentTransform->worldTransform * transform->localTransform;
	}

}
