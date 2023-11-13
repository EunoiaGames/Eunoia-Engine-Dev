#include "SpriteSubmissionSystem.h"
#include "../Components/SpriteComponent.h"
#include "../Components/Transform2DComponent.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	SpriteSubmissionSystem::SpriteSubmissionSystem()
	{
		AddComponentType<SpriteComponent>();
		AddComponentType<Transform2DComponent>();
	}

	void SpriteSubmissionSystem::ProcessEntityOnRender(EntityID entity)
	{
		const Transform2D& transform = m_ECS->GetComponent<Transform2DComponent>(entity)->worldTransform;
		const SpriteComponent* spriteComponent = m_ECS->GetComponent<SpriteComponent>(entity);

		Sprite sprite;
		sprite.pos = v3(transform.pos + spriteComponent->offset, 0.0f);
		sprite.occluder = false;
		sprite.color = spriteComponent->color;
		sprite.rot = transform.rot;
		sprite.size = spriteComponent->size;
		sprite.spriteSheet = spriteComponent->spriteSheet;
		sprite.texturePos = spriteComponent->texturePos;

		Engine::GetRenderer()->GetRenderer2D()->SubmitSprite(sprite);
	}

	SpriteGroupSubmissionSystem::SpriteGroupSubmissionSystem()
	{
		AddComponentType<SpriteGroupComponent>();
	}

	void SpriteGroupSubmissionSystem::ProcessEntityOnRender(EntityID entity)
	{
		const SpriteGroupComponent* group = m_ECS->GetComponent<SpriteGroupComponent>(entity);
		const Transform2DComponent* transformComponent = m_ECS->GetComponent<Transform2DComponent>(entity);

		m3 transform = transformComponent ? transformComponent->worldTransform.CreateTransformMatrix() : m3::CreateIdentity();

		Renderer2D* renderer = Engine::GetRenderer()->GetRenderer2D();
		renderer->PushTransformStack(transform);

		for (u32 i = 0; i < group->sprites.Size(); i++)
			renderer->SubmitSprite(group->sprites[i]);

		renderer->PopTransformStack();
	}

}
