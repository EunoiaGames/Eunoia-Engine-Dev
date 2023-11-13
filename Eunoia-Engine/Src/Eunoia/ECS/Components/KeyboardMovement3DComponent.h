#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct KeyboardMovement3DComponent : public ECSComponent
	{
		KeyboardMovement3DComponent(Key forward = EU_KEY_W, Key back = EU_KEY_S, Key left = EU_KEY_A, Key right = EU_KEY_D,
			Key sprint = EU_KEY_LEFT_SHIFT, r32 speed = 1.0f, r32 speedSprintMultiplier = 3.0f) :
			forward(forward),
			back(back),
			left(left),
			right(right),
			sprint(sprint),
			speed(speed),
			speedSprintMultiplier(speedSprintMultiplier)
		{}

		EU_PROPERTY() Key forward;
		EU_PROPERTY() Key back;
		EU_PROPERTY() Key left;
		EU_PROPERTY() Key right;
		EU_PROPERTY() Key sprint;

		EU_PROPERTY() r32 speed;
		EU_PROPERTY() r32 speedSprintMultiplier;
	};

}