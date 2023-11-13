#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Camera2DComponent : public ECSComponent
	{
		Camera2DComponent(r32 orthoScale = 1.0f) :
			orthoScale(orthoScale)
		{}

		EU_PROPERTY()
		r32 orthoScale;
	};

}