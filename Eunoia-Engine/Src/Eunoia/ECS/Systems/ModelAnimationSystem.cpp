#include "ModelAnimationSystem.h"
#include "../Components/ModelComponent.h"
#include "../../Rendering/Asset/AssetManager.h"
#include "../../Core/Engine.h"

namespace Eunoia {

	ModelAnimationSystem::ModelAnimationSystem()
	{
		AddComponentType<ModelAnimationComponent>();
		AddComponentType<ModelComponent>();
	}

	void ModelAnimationSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		ModelAnimationComponent* animation = m_ECS->GetComponent<ModelAnimationComponent>(entity);
		ModelComponent* modelComponent = m_ECS->GetComponent<ModelComponent>(entity);

		const Model& model = AssetManager::GetModel(modelComponent->model);

		u32 animationIndex = 0;
		for (u32 i = 0; i < model.animations.Size(); i++)
		{
			if (model.animations[i].name == animation->name)
			{
				animationIndex = i;
				break;
			}
		}

		r32 tps = model.animations[animationIndex].tps;
		tps == 0 ? 25.0f : tps;

		r32 timeInTicks = Engine::GetTime() * tps * animation->speed;
		r32 animationTime = fmodf(timeInTicks, model.animations[animationIndex].durration);

		CalculateBoneTransform(model, model.animations[animationIndex], animation, model.rootBone, m4::CreateIdentity(), animationTime);
	}

	u32 ModelAnimationSystem::GetPosKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime)
	{
		for (u32 i = 0; i < boneAnimation.positionKeyFrames.Size() - 1; i++)
			if (animationTime < boneAnimation.positionKeyFrames[i + 1].timeStamp)
				return i;

		return boneAnimation.positionKeyFrames.Size();
	}

	u32 ModelAnimationSystem::GetScaleKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime)
	{
		for (u32 i = 0; i < boneAnimation.scaleKeyFrames.Size() - 1; i++)
			if (animationTime < boneAnimation.scaleKeyFrames[i + 1].timeStamp)
				return i;

		return boneAnimation.scaleKeyFrames.Size();
	}

	u32 ModelAnimationSystem::GetRotKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime)
	{
		for (u32 i = 0; i < boneAnimation.rotationKeyFrames.Size() - 1; i++)
			if (animationTime < boneAnimation.rotationKeyFrames[i + 1].timeStamp)
				return i;

		return boneAnimation.rotationKeyFrames.Size();
	}

	void ModelAnimationSystem::CalculateBoneTransform(const Model& model, const ModelAnimation& animationData, ModelAnimationComponent* animation, u32 boneIndex, const m4& parentTransform, r32 animationTime)
	{
		const ModelBone& bone = model.bones[boneIndex];

		const ModelAnimationChannel* boneAnimaion = 0;
		for (u32 i = 0; i < animationData.channels.Size(); i++)
		{
			if (animationData.channels[i].boneIndex == boneIndex)
			{
				boneAnimaion = &animationData.channels[i];
				break;
			}
		}

		m4 transform = bone.transform;

		if (boneAnimaion)
		{
			u32 positionKeyFrameIndex = GetPosKeyFrameIndex(*boneAnimaion, animationTime);
			u32 scaleKeyFrameIndex = GetPosKeyFrameIndex(*boneAnimaion, animationTime);
			u32 rotationKeyFrameIndex = GetRotKeyFrameIndex(*boneAnimaion, animationTime);

			const ModelAnimationVecKeyFrame& posCurrent = boneAnimaion->positionKeyFrames[positionKeyFrameIndex];
			const ModelAnimationVecKeyFrame& posNext = boneAnimaion->positionKeyFrames[positionKeyFrameIndex + 1];
			r32 deltaTime = posNext.timeStamp - posCurrent.timeStamp;
			r32 lerpFactor = (animationTime - posCurrent.timeStamp) / deltaTime;
			v3 lerpedPos = posCurrent.value.Lerp(posNext.value, lerpFactor);

			const ModelAnimationVecKeyFrame& scaleCurrent = boneAnimaion->scaleKeyFrames[scaleKeyFrameIndex];
			const ModelAnimationVecKeyFrame& scaleNext = boneAnimaion->scaleKeyFrames[scaleKeyFrameIndex + 1];
			deltaTime = scaleNext.timeStamp - scaleCurrent.timeStamp;
			lerpFactor = (animationTime - scaleCurrent.timeStamp) / deltaTime;
			v3 lerpedScale = scaleCurrent.value.Lerp(scaleNext.value, lerpFactor);

			const ModelAnimationQuatKeyFrame& rotCurrent = boneAnimaion->rotationKeyFrames[rotationKeyFrameIndex];
			const ModelAnimationQuatKeyFrame& rotNext = boneAnimaion->rotationKeyFrames[rotationKeyFrameIndex + 1];
			deltaTime = rotNext.timeStamp - rotCurrent.timeStamp;
			r32 slerpFactor = (animationTime - rotCurrent.timeStamp) / deltaTime;
			quat slerpedRot = rotCurrent.value.Slerp(rotNext.value, slerpFactor);

			transform = m4::CreateTransformation(lerpedPos, lerpedScale, slerpedRot);
		}

		m4 globalTransform = parentTransform * transform;
		if (boneAnimaion)
		{
			if (animation->boneTransforms.Empty())
				animation->boneTransforms.SetCapacityAndElementCount(model.bones.Size());
			animation->boneTransforms[boneIndex] = model.globalInverseTransform * globalTransform * bone.boneSpaceTransform;
		}

		for (u32 i = 0; i < bone.children.Size(); i++)
			CalculateBoneTransform(model, animationData, animation, bone.children[i], globalTransform, animationTime);
	}


}