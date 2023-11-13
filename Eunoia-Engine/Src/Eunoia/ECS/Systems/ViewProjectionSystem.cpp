#include "ViewProjectionSystem.h"

#include "../Components/CameraComponent.h"
#include "../Components/Transform3DComponet.h"
#include "../../Core/Engine.h"
#include "../../Rendering/Renderer3D.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	Eunoia::ViewProjectionSystem::ViewProjectionSystem()
	{
		AddComponentType<CameraComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void ViewProjectionSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		const Transform3D& transform = m_ECS->GetComponent<Transform3DComponent>(entity)->worldTransform;
		const CameraComponent* camera = m_ECS->GetComponent<CameraComponent>(entity);

		m4 viewMatrix = m4::CreateView(transform.pos, transform.rot);
		u32 width, height;
		Engine::GetRenderer()->GetOutputSize(&width, &height);
		m4 projectionMatrix = m4::CreatePerspective(width, height, camera->fov, 0.01f, 1000.0f);

		Engine::GetRenderer()->GetRenderer3D()->SetCamera(viewMatrix, projectionMatrix, transform.pos, transform.rot);
	}

}
