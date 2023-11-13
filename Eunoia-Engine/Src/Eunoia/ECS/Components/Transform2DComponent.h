#pragma once

#include "../ECS.h"
#include "../../Math/Math.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Transform2DComponent : public ECSComponent
	{
		Transform2DComponent(const Transform2D& localTransform) :
			localTransform(localTransform),
			worldTransform(localTransform)
		{}

		Transform2DComponent()
		{}

		EU_PROPERTY()
		Transform2D localTransform;
		Transform2D worldTransform;
	};

}
