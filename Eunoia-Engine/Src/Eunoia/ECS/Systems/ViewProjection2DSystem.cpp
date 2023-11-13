#include "ViewProjection2DSystem.h"

#include "../Components/Transform2DComponent.h"
#include "../Components/Camera2DComponent.h"

#include "../../Core/Engine.h"

namespace Eunoia {

	ViewProjection2DSystem::ViewProjection2DSystem()
	{
		AddComponentType<Camera2DComponent>();
		AddComponentType<Transform2DComponent>();
	}

	void ViewProjection2DSystem::ProcessEntityOnRender(EntityID entity)
	{
		Camera2DComponent* camera = m_ECS->GetComponent<Camera2DComponent>(entity);
		Transform2DComponent* transform = m_ECS->GetComponent<Transform2DComponent>(entity);

		v2 pos = transform->localTransform.pos;
		r32 rot = transform->localTransform.rot;

		u32 w, h;
		Engine::GetRenderer()->GetOutputSize(&w, &h);
		r32 width = w;
		r32 height = h;

		r32 left = (-width / 2.0f) * camera->orthoScale;
		r32 right = (width / 2.0f) * camera->orthoScale;
		r32 bottom = (-height / 2.0f) * camera->orthoScale;
		r32 top = (height / 2.0f) * camera->orthoScale;

		Engine::GetRenderer()->GetRenderer2D()->SetCamera(pos, rot);
		Engine::GetRenderer()->GetRenderer2D()->SetOrthographic(left, right, bottom, top);
	}

}
