#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct KeyboardMovement2DComponent : public ECSComponent
	{
		KeyboardMovement2DComponent(Key up = EU_KEY_W, Key down = EU_KEY_S, Key left = EU_KEY_A, Key right = EU_KEY_D, r32 speed = 1.0f) :
			up(up),
			down(down),
			left(left),
			right(right),
			speed(speed)
		{}

		EU_PROPERTY() Key up;
		EU_PROPERTY() Key down;
		EU_PROPERTY() Key left;
		EU_PROPERTY() Key right;
		EU_PROPERTY() r32 speed;
	};

}