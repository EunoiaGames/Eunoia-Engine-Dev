#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct ModelAnimationComponent : public ECSComponent
	{
		ModelAnimationComponent(String name, r32 speed) :
			name(name),
			speed(speed),
			currentTime(0.0f)
		{}

		ModelAnimationComponent()
		{}

		EU_PROPERTY() String name;
		EU_PROPERTY() r32 speed;
		EU_PROPERTY() r32 currentTime;
		List<m4> boneTransforms;
	};

}