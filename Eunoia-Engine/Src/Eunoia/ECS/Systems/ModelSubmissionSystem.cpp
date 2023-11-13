#include "ModelSubmissionSystem.h"
#include "../Components/ModelComponent.h"
#include "../Components/Transform3DComponet.h"
#include "../Components/MaterialComponent.h"
#include "../Components/ModelAnimationComponent.h"
#include "../../Rendering/Renderer3D.h"
#include "../../Rendering/Asset/AssetManager.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	ModelSubmissionSystem::ModelSubmissionSystem()
	{
		AddComponentType<ModelComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void ModelSubmissionSystem::ProcessEntityOnRender(EntityID entity)
	{
		Renderer3D* renderer = Engine::GetRenderer()->GetRenderer3D();

		ModelComponent* modelComponent = m_ECS->GetComponent<ModelComponent>(entity);
		m4 transform = m_ECS->GetComponent<Transform3DComponent>(entity)->worldTransform.CreateTransformMatrix();
		MaterialComponent* materialComponent = m_ECS->GetComponent<MaterialComponent>(entity);
		ModelAnimationComponent* animationComponent = m_ECS->GetComponent<ModelAnimationComponent>(entity);

		const Model& model = AssetManager::GetModel(modelComponent->model);

		if (modelComponent->wireframe)
		{
			renderer->SubmitWireframeModel(model, transform);
			return;
		}

		if (materialComponent)
		{
			Model modelCustomMaterial = model;
			for (u32 i = 0; i < modelCustomMaterial.materials.Size(); i++)
				modelCustomMaterial.materials[i] = materialComponent->material;
			for (u32 i = 0; i < modelCustomMaterial.modifiers.Size(); i++)
				modelCustomMaterial.modifiers[i] = materialComponent->modifier;

			if (animationComponent)
			{
				renderer->SubmitModel(modelCustomMaterial, transform, &animationComponent->boneTransforms[0], animationComponent->boneTransforms.Size(), true, entity);
			}
			else
			{
				renderer->SubmitModel(modelCustomMaterial, transform, 0, 0, false, entity);
			}
		}
		else
		{
			if (animationComponent)
			{
				renderer->SubmitModel(model, transform, &animationComponent->boneTransforms[0], animationComponent->boneTransforms.Size(), true, entity);
			}
			else
			{
				renderer->SubmitModel(model, transform, 0, 0, false, entity);
			}
		}
	}

}
