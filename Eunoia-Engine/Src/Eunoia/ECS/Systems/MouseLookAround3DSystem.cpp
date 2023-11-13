#include "MouseLookAround3DSystem.h"
#include "../Components/MouseLookAround3DComponent.h"
#include "../Components/Transform3DComponet.h"
#include "../../Core/Engine.h"
#include "../../Core/Input.h"

namespace Eunoia {

	MouseLookAround3DSystem::MouseLookAround3DSystem()
	{
		AddComponentType<MouseLookAround3DComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void MouseLookAround3DSystem::Init() 
	{
		toggleKey = EU_KEY_ESC;
	}

	void MouseLookAround3DSystem::PreUpdate(r32 dt)
	{
		v2 centerPos = v2(Engine::GetDisplay()->GetWidth() / 2, Engine::GetDisplay()->GetHeigth() / 2);
		centerPos.x = EU_FLOOR(centerPos.x);
		centerPos.y = EU_FLOOR(centerPos.y);

		if (EUInput::IsKeyPressed(toggleKey))
		{
			Display* display = Engine::GetDisplay();
			display->SetCursorVisible(!display->IsCursorVisible());
			display->SetMousePos(centerPos);
		}
	}

	void MouseLookAround3DSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		const MouseLookAround3DComponent* lookAround = m_ECS->GetComponent<MouseLookAround3DComponent>(entity);
		Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;
		
		Display* display = Engine::GetDisplay();
		v2 centerPos = v2(display->GetWidth() / 2, display->GetHeigth() / 2);
		centerPos.x = EU_FLOOR(centerPos.x);
		centerPos.y = EU_FLOOR(centerPos.y);

		if(display->IsCursorVisible())
			return;

		v2 mousePos = display->GetMousePos();
		v2 deltaPos = mousePos - centerPos;

		b32 rotX = deltaPos.y != 0;
		b32 rotY = deltaPos.x != 0;

		r32 sensitivity = lookAround->sensitivity * dt;

		if (rotX)
			transform->Rotate(transform->rot.GetRight(), deltaPos.y * sensitivity);
		if (rotY)
			transform->Rotate(v3(0.0f, 1.0f, 0.0f), deltaPos.x * sensitivity);
		if (rotX || rotY)
			display->SetMousePos(centerPos);
	}


}