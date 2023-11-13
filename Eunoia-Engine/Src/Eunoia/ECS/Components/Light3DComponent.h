#pragma once

#include "../ECS.h"
#include "../../Rendering/Light3D.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Light3DComponent : public ECSComponent
	{
		Light3DComponent(Light3DType type, const v3& color, r32 intensity, const Attenuation& attenuation = Attenuation(0.0f, 0.0f, 1.0f)) :
			type(type),
			color(color),
			intensity(intensity),
			attenuation(attenuation)
		{}

		Light3DComponent() {}

		EU_PROPERTY() Light3DType type;
		EU_PROPERTY() r32 intensity;
		EU_PROPERTY() v3 color;
		EU_PROPERTY() Attenuation attenuation;
	};

}