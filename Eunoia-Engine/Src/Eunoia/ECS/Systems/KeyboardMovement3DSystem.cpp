#include "KeyboardMovement3DSystem.h"
#include "../Components/KeyboardMovement3DComponent.h"
#include "../Components/Transform3DComponet.h"
#include "../../Core/Input.h"

namespace Eunoia {

	KeyboardMovement3DSystem::KeyboardMovement3DSystem()
	{
		AddComponentType<KeyboardMovement3DComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void KeyboardMovement3DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		const KeyboardMovement3DComponent* movement = m_ECS->GetComponent<KeyboardMovement3DComponent>(entity);
		Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;

		r32 speed = movement->speed * dt;

		if (EUInput::IsKeyDown(movement->sprint))
		{
			speed *= movement->speedSprintMultiplier;
		}

		if (EUInput::IsKeyDown(movement->forward))
		{
			transform->Translate(transform->rot.GetForward(), speed);
		}
		if (EUInput::IsKeyDown(movement->back))
		{
			transform->Translate(transform->rot.GetBack(), speed);
		}
		if (EUInput::IsKeyDown(movement->right))
		{
			transform->Translate(transform->rot.GetRight(), speed);
		}
		if (EUInput::IsKeyDown(movement->left))
		{
			transform->Translate(transform->rot.GetLeft(), speed);
		}
	}

}
