#include "KeyboardMovement2DSystem.h"

#include "../Components/Transform2DComponent.h"
#include "../Components/KeyboardMovement2DComponent.h"
#include "../../Core/Input.h"

namespace Eunoia {

	KeyboardMovement2DSystem::KeyboardMovement2DSystem()
	{
		AddComponentType<KeyboardMovement2DComponent>();
		AddComponentType<Transform2DComponent>();
	}

	void KeyboardMovement2DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		KeyboardMovement2DComponent* movement = m_ECS->GetComponent<KeyboardMovement2DComponent>(entity);
		Transform2DComponent* transform = m_ECS->GetComponent<Transform2DComponent>(entity);

		if (EUInput::IsKeyDown(movement->up))
		{
			transform->localTransform.pos.y += movement->speed * dt;
		}
		if (EUInput::IsKeyDown(movement->down))
		{
			transform->localTransform.pos.y -= movement->speed * dt;
		}
		if (EUInput::IsKeyDown(movement->left))
		{
			transform->localTransform.pos.x -= movement->speed * dt;
		}
		if (EUInput::IsKeyDown(movement->right))
		{
			transform->localTransform.pos.x += movement->speed * dt;
		}
	}

}
