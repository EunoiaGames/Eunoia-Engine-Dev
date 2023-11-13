#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct CameraComponent : public ECSComponent
	{
		CameraComponent(r32 fov = 70.0f) :
			fov(fov)
		{}

		EU_PROPERTY()
		r32 fov;
	};

}