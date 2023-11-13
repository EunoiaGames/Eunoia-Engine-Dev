#include "LightSubmissionSystem.h"
#include "../Components/Light3DComponent.h"
#include "../Components/Transform3DComponet.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	LightSubmissionSystem::LightSubmissionSystem()
	{
		AddComponentType<Light3DComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void LightSubmissionSystem::ProcessEntityOnRender(EntityID entity)
	{
		const Transform3D& transform = m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;
		const Light3DComponent* lightComponent = m_ECS->GetComponent<Light3DComponent>(entity);

		Light3D light;
		light.type = lightComponent->type;
		light.colorAndIntensity = v4(lightComponent->color, lightComponent->intensity);
		light.direction = transform.rot.GetForward();
		light.attenuation = lightComponent->attenuation;
		light.pos = transform.pos;

		if (light.type == LIGHT3D_DIRECTIONAL)
		{
			r32 orthoValue = 50.0f;
			r32 nearPlane = -30.0f;
			r32 farPlane = 30.0f;
			//TODO: Should't create a matrix for every light every frame
			light.shadowInfo.projection = m4::CreateOrthographic(-orthoValue, orthoValue, -orthoValue, orthoValue, nearPlane, farPlane);
			light.shadowInfo.directionalRot = transform.rot;
			light.shadowInfo.castShadow = true;
		}

		Engine::GetRenderer()->GetRenderer3D()->SubmitLight(light);
	}

}
