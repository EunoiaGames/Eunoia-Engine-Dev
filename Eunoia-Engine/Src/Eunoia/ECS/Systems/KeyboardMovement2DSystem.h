#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API KeyboardMovement2DSystem : public ECSSystem
	{
	public:
		KeyboardMovement2DSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	};

}