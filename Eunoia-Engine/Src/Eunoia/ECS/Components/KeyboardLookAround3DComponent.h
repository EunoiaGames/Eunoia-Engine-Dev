#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct KeyboardLookAround3DComponent : public ECSComponent
	{
		KeyboardLookAround3DComponent(Key up = EU_KEY_UP, Key down = EU_KEY_DOWN, Key left = EU_KEY_LEFT, Key right = EU_KEY_RIGHT, r32 sensitivity = 1.0f) :
			up(up),
			down(down),
			left(left),
			right(right),
			sensitivity(sensitivity)
		{}

		EU_PROPERTY() Key up;
		EU_PROPERTY() Key down;
		EU_PROPERTY() Key left;
		EU_PROPERTY() Key right;
		EU_PROPERTY() r32 sensitivity;
	};

}