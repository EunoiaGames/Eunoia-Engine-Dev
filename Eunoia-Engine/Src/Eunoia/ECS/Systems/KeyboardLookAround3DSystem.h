#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API KeyboardLookAround3DSystem : public ECSSystem
	{
	public:
		KeyboardLookAround3DSystem();

		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	};

}