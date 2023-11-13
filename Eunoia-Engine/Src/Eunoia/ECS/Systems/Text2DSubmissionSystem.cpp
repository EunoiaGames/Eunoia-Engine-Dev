#include "Text2DSubmissionSystem.h"
#include "../Components/Text2DComponent.h"
#include "../Components/Transform2DComponent.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	Text2DSubmissionSystem::Text2DSubmissionSystem()
	{
		AddComponentType<Text2DComponent>();
		AddComponentType<Transform2DComponent>();
	}

	void Text2DSubmissionSystem::ProcessEntityOnRender(EntityID entity)
	{
		const Transform2D& transform = m_ECS->GetComponent<Transform2DComponent>(entity)->worldTransform;
		const Text2DComponent* text = m_ECS->GetComponent<Text2DComponent>(entity);

		Engine::GetRenderer()->GetRenderer2D()->SubmitText(text->text, v3(transform.pos, 0.0f), text->color, transform.scale.x);
	}

}
