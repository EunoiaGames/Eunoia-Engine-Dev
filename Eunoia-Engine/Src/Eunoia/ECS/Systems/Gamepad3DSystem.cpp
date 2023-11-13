#include "Gamepad3DSystem.h"
#include "../Components/Transform3DComponet.h"
#include "../Components/Gamepad3DComponent.h"
#include "../../Core/Input.h"

namespace Eunoia {

	Gamepad3DSystem::Gamepad3DSystem()
	{
		AddComponentType<Transform3DComponent>();
		AddComponentType<Gamepad3DComponent>();
	}

	void Gamepad3DSystem::Init()
	{
		toggleButton = EU_GAMEPAD_XBOX360_BUTTON_BACK;
		m_Toggled = false;
	}

	void Gamepad3DSystem::PreUpdate(r32 dt)
	{
		u32 gIndex = 0;
		for (u32 i = 0; i < EU_NUM_GAMEPADS; i++)
		{
			if (EUInput::IsGamepadActive((Gamepad)i))
			{
				if (EUInput::IsGamepadButtonPressed((Gamepad)i, toggleButton))
				{
					m_Toggled = !m_Toggled;
					break;
				}
			}
		}
	}

	void Gamepad3DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;
		const Gamepad3DComponent* gamepad = m_ECS->GetComponent<Gamepad3DComponent>(entity);

		if (!m_Toggled)
			return;

		Gamepad gIndex = gamepad->gamepad;
		r32 speed = gamepad->speed * dt;
		if (EUInput::IsGamepadButtonDown(gIndex, gamepad->sprintButton))
			speed *= gamepad->sprintSpeedMultiplier;

		v2 leftThumbstick = EUInput::GetGamepadThumbstick(gIndex, EU_GAMEPAD_XBOX360_THUMBSTICK_LEFT);
		v2 rightThumbstick = EUInput::GetGamepadThumbstick(gIndex, EU_GAMEPAD_XBOX360_THUMBSTICK_RIGHT);

		if (leftThumbstick.y != 0.0f)
			transform->Translate(transform->rot.GetForward(), leftThumbstick.y * speed);
		if (leftThumbstick.x != 0.0f)
			transform->Translate(transform->rot.GetRight(), leftThumbstick.x * speed);

		if (rightThumbstick.y != 0)
			transform->Rotate(transform->rot.GetRight(), -rightThumbstick.y * gamepad->sensitivity * dt * (gamepad->invertY ? -1.0f : 1.0f));
		if (rightThumbstick.x != 0)
			transform->Rotate(v3(0.0f, 1.0f, 0.0f), rightThumbstick.x * gamepad->sensitivity * dt);
	}

}
