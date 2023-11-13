#pragma once

#include "../ECS.h"
#include "../../Math/Math.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Transform3DComponent : public ECSComponent
	{
		Transform3DComponent(const Transform3D& localTransform = Transform3D()) :
			localTransform(localTransform),
			worldTransform(localTransform)
		{}

		EU_PROPERTY()
		Transform3D localTransform;
		Transform3D worldTransform;
	};

}