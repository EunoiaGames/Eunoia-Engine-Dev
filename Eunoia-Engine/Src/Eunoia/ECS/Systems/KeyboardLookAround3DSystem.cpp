#include "KeyboardLookAround3DSystem.h"
#include "../Components/Transform3DComponet.h"
#include "../Components/KeyboardLookAround3DComponent.h"
#include "../../Core/Input.h"

namespace Eunoia {

	KeyboardLookAround3DSystem::KeyboardLookAround3DSystem()
	{
		AddComponentType<KeyboardLookAround3DComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void KeyboardLookAround3DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;
		KeyboardLookAround3DComponent* lookAround = m_ECS->GetComponent<KeyboardLookAround3DComponent>(entity);

		if (EUInput::IsKeyDown(lookAround->up))
			transform->Rotate(transform->rot.GetRight(), -lookAround->sensitivity * dt);
		if(EUInput::IsKeyDown(lookAround->down))
			transform->Rotate(transform->rot.GetRight(), lookAround->sensitivity * dt);
		if(EUInput::IsKeyDown(lookAround->right))
			transform->Rotate(v3(0.0f, 1.0f, 0.0f), lookAround->sensitivity * dt);
		if (EUInput::IsKeyDown(lookAround->left))
			transform->Rotate(v3(0.0f, 1.0f, 0.0f), -lookAround->sensitivity * dt);
	}

}
