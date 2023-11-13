#pragma once

#include "../ECS.h"
#include "../Components/ModelAnimationComponent.h"
#include "../../Rendering/Asset/Model.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API ModelAnimationSystem : public ECSSystem
	{
	public:
		ModelAnimationSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt);
	private:
		r32 m_Time;
	private:
		static u32 GetPosKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime);
		static u32 GetScaleKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime);
		static u32 GetRotKeyFrameIndex(const ModelAnimationChannel& boneAnimation, r32 animationTime);
		static void CalculateBoneTransform(const Model& model, const ModelAnimation& animationData, ModelAnimationComponent* animation, u32 bone, const m4& parentTransform, r32 animationTime);
	};

}