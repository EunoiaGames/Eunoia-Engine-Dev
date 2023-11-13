#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct MouseLookAround3DComponent : public ECSComponent
	{
		MouseLookAround3DComponent(r32 sensitivity = 1.0f, Key toggleKey = EU_KEY_ESC) :
			sensitivity(sensitivity),
			toggleKey(toggleKey)
		{}

		EU_PROPERTY() r32 sensitivity;
		EU_PROPERTY() Key toggleKey;
	};

}