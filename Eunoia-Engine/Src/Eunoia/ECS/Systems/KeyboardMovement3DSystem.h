#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API KeyboardMovement3DSystem : public ECSSystem
	{
	public:
		KeyboardMovement3DSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	};

}